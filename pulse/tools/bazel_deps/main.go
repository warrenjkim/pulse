package main

import (
	"bufio"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"
)

var includeRe = regexp.MustCompile(`^\s*#include\s*["<]([^">]+)[">]`)

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintln(os.Stderr, "usage: bazel_deps <bazel-target-or-pattern>")
		os.Exit(1)
	}
	pattern := os.Args[1]

	root, err := workspaceRoot()
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	if err := os.Chdir(root); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	targets, err := bazelQuery(fmt.Sprintf("kind('cc_library|cc_test|cc_binary', %s)", pattern))
	if err != nil {
		fmt.Fprintf(os.Stderr, "error expanding targets: %v\n", err)
		os.Exit(1)
	}

	for _, target := range targets {
		analyzeTarget(target)
	}
}

func analyzeTarget(target string) {
	if isUmbrellaTarget(target) {
		return
	}

	deps, err := bazelQuery(fmt.Sprintf("deps(%s, 1) - %s", target, target))
	if err != nil {
		fmt.Fprintf(os.Stderr, "error querying deps for %s: %v\n", target, err)
		return
	}

	srcs, err := bazelQuery(fmt.Sprintf("labels(srcs, %s) union labels(hdrs, %s)", target, target))
	if err != nil {
		fmt.Fprintf(os.Stderr, "error querying srcs/hdrs for %s: %v\n", target, err)
		return
	}

	includes, err := extractIncludes(srcs)
	if err != nil {
		fmt.Fprintf(os.Stderr, "error extracting includes for %s: %v\n", target, err)
		return
	}

	fmt.Printf("%s\n", target)
	hasUnused := false
	for _, dep := range deps {
		if isToolchainDep(dep) || isFileDep(dep) {
			continue
		}
		hdrs, err := bazelQuery(fmt.Sprintf("labels(hdrs, %s)", dep))
		if err != nil {
			continue
		}
		if !isDepUsed(hdrs, includes) {
			fmt.Printf("  unused: %s\n", dep)
			hasUnused = true
		}
	}
	if !hasUnused {
		fmt.Println("  no unused deps")
	}
	fmt.Println()
}

func workspaceRoot() (string, error) {
	if dir := os.Getenv("BUILD_WORKSPACE_DIRECTORY"); dir != "" {
		return dir, nil
	}
	return "", fmt.Errorf("BUILD_WORKSPACE_DIRECTORY not set — run via 'bazel run //pulse/tools:bazel_deps'")
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
		line = strings.TrimSpace(line)
		if line != "" {
			lines = append(lines, line)
		}
	}
	return lines, nil
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
	}
	return includes, nil
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

func isToolchainDep(dep string) bool {
	return strings.HasPrefix(dep, "@bazel_tools//") ||
		strings.HasPrefix(dep, "@@platforms//") ||
		strings.HasPrefix(dep, "@googletest//")

}

func isFileDep(dep string) bool {
	ext := filepath.Ext(dep)
	return ext == ".h" || ext == ".cc" || ext == ".cpp"
}

func labelToPath(label string) string {
	label = strings.TrimPrefix(label, "//")
	label = strings.ReplaceAll(label, ":", "/")
	return label
}

func isUmbrellaTarget(target string) bool {
	// //pulse/http:http -> package = "pulse/http", name = "http"
	parts := strings.SplitN(strings.TrimPrefix(target, "//"), ":", 2)
	if len(parts) != 2 {
		return false
	}
	pkg := parts[0]
	name := parts[1]
	// last segment of package matches target name
	segments := strings.Split(pkg, "/")
	return segments[len(segments)-1] == name
}
