package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"
)

var includeRe = regexp.MustCompile(`^\s*#include\s*["<]([^">]+)[">]`)

func cdToWorkspace() error {
	root := os.Getenv("BUILD_WORKSPACE_DIRECTORY")
	if root == "" {
		return fmt.Errorf("BUILD_WORKSPACE_DIRECTORY not set — run via 'bazel run //tools:cc_deps'")
	}

	return os.Chdir(root)
}

func isUmbrellaTarget(target string) bool {
	parts := strings.SplitN(strings.TrimPrefix(target, "//"), ":", 2)
	if len(parts) != 2 {
		return false
	}

	segments := strings.Split(parts[0], "/")
	return segments[len(segments)-1] == parts[1]
}

func isTestFixtureDep(dep string) bool {
	return strings.HasPrefix(dep, "@googletest//")
}

func isToolchainDep(dep string) bool {
	return strings.HasPrefix(dep, "@bazel_tools//") ||
		strings.HasPrefix(dep, "@@platforms//")
}

func isFileDep(dep string) bool {
	ext := filepath.Ext(dep)
	return ext == ".h" ||
		ext == ".cc" ||
		ext == ".cpp" ||
		ext == ".sql"
}

func shouldIgnoreDep(dep string) bool {
	return isTestFixtureDep(dep) || isToolchainDep(dep) || isFileDep(dep)
}

func isDepUsed(hdrs []string, includes map[string]bool) bool {
	for _, hdr := range hdrs {
		path := labelToPath(hdr)
		base := filepath.Base(path)
		if includes[path] || includes[base] {
			return true
		}
	}

	return false
}

func labelToPath(label string) string {
	// handle external repo labels like @pulse//pulse/core:result.h
	if idx := strings.Index(label, "//"); idx != -1 {
		label = label[idx+2:]
	}

	return strings.ReplaceAll(label, ":", "/")
}

func extractIncludes(srcs []string) (map[string]bool, error) {
	includes := make(map[string]bool)
	for _, src := range srcs {
		path := labelToPath(src)
		f, err := os.Open(path)
		if err != nil {
			return nil, err
		}

		defer f.Close()
		scanner := bufio.NewScanner(f)
		for scanner.Scan() {
			if m := includeRe.FindStringSubmatch(scanner.Text()); m != nil {
				includes[m[1]] = true
			}
		}

		if err := scanner.Err(); err != nil {
			return nil, fmt.Errorf("error scanning %s: %w", path, err)
		}
	}

	return includes, nil
}

func bazelQuery(query string) ([]string, error) {
	cmd := exec.Command("bazel", "query", query)

	var stderr strings.Builder
	cmd.Stderr = &stderr
	out, err := cmd.Output()
	if err != nil {
		return nil, fmt.Errorf("bazel query %q failed: %w\nstderr: %s", query, err, stderr.String())
	}

	var lines []string
	for _, line := range strings.Split(string(out), "\n") {
		if line = strings.TrimSpace(line); line != "" {
			lines = append(lines, line)
		}
	}

	return lines, nil
}

func buildozerRemove(dep string, target string) error {
	cmd := exec.Command("buildozer", fmt.Sprintf("remove deps %s", dep), target)

	var stderr strings.Builder
	cmd.Stderr = &stderr
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("buildozer failed: %w\nstderr: %s", err, stderr.String())
	}

	return nil
}

func analyzeTarget(target string) bool {
	if isUmbrellaTarget(target) {
		return false
	}

	var errs []string

	deps, err := bazelQuery(fmt.Sprintf("deps(%s, 1) - %s", target, target))
	if err != nil {
		fmt.Printf("%s\n  error: %v\n\n", target, err)
		return false
	}

	srcs, err := bazelQuery(fmt.Sprintf("labels(srcs, %s) union labels(hdrs, %s)", target, target))
	if err != nil {
		fmt.Printf("%s\n  error: %v\n\n", target, err)
		return false
	}

	includes, err := extractIncludes(srcs)
	if err != nil {
		fmt.Printf("%s\n  error: %v\n\n", target, err)
		return false
	}

	var unused []string
	for _, dep := range deps {
		if shouldIgnoreDep(dep) {
			continue
		}

		hdrs, err := bazelQuery(fmt.Sprintf("labels(hdrs, %s)", dep))
		if err != nil {
			errs = append(errs, fmt.Sprintf("  error querying hdrs for %s: %v", dep, err))
			continue
		}

		if !isDepUsed(hdrs, includes) {
			unused = append(unused, dep)
		}
	}

	if len(unused) == 0 && len(errs) == 0 {
		fmt.Printf("%s -- OK\n", target)
		return false
	}

	fmt.Printf("%s\n", target)
	for _, dep := range unused {
		fmt.Printf("  unused: %s", dep)
		if !*fix {
			fmt.Println()
			continue
		}

		if err := buildozerRemove(dep, target); err != nil {
			fmt.Printf("--  error fixing %s: %v\n", dep, err)
			continue
		}

		fmt.Printf(" -- REMOVED\n")
	}

	for _, e := range errs {
		fmt.Println(e)
	}

	fmt.Println()

	return len(unused) == 0
}

var fix = flag.Bool("fix", false, "automatically remove unused deps using buildozer")

func main() {
	flag.Parse()
	if flag.NArg() < 1 {
		fmt.Fprintln(os.Stderr, "usage: bazel_deps [--fix] <bazel-target-or-pattern>")
		os.Exit(1)
	}

	if err := cdToWorkspace(); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	targets, err := bazelQuery(fmt.Sprintf("kind('cc_library|cc_test|cc_binary', %s)", flag.Arg(0) /*pattern*/))
	if err != nil {
		fmt.Fprintf(os.Stderr, "error expanding targets: %v\n", err)
		os.Exit(1)
	}

	unused := false
	for _, target := range targets {
		unused = unused || analyzeTarget(target)
	}

	if unused {
		os.Exit(1)
	}
}
