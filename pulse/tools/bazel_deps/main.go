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
		fmt.Fprintln(os.Stderr, "usage: unused-deps <bazel-target>")
		os.Exit(1)
	}
	target := os.Args[1]

	root, err := workspaceRoot()
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	if err := os.Chdir(root); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	deps, err := bazelQuery(fmt.Sprintf("deps(%s, 1) - %s", target, target))
	if err != nil {
		fmt.Fprintf(os.Stderr, "error querying deps: %v\n", err)
		os.Exit(1)
	}

	srcs, err := bazelQuery(fmt.Sprintf("labels(srcs, %s)", target))
	if err != nil {
		fmt.Fprintf(os.Stderr, "error querying srcs: %v\n", err)
		os.Exit(1)
	}

	includes, err := extractIncludes(srcs)
	if err != nil {
		fmt.Fprintf(os.Stderr, "error extracting includes: %v\n", err)
		os.Exit(1)
	}

	fmt.Printf("%s\n", target)
	for _, dep := range deps {
		hdrs, err := bazelQuery(fmt.Sprintf("labels(hdrs, %s)", dep))
		if err != nil {
			continue
		}
		if !isDepUsed(hdrs, includes) {
			fmt.Printf("  unused: %s\n", dep)
		}
	}
}

func workspaceRoot() (string, error) {
	if dir := os.Getenv("BUILD_WORKSPACE_DIRECTORY"); dir != "" {
		return dir, nil
	}
	return "", fmt.Errorf("BUILD_WORKSPACE_DIRECTORY not set — run via 'bazel run //tools:bazel_deps'")
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

func labelToPath(label string) string {
	label = strings.TrimPrefix(label, "//")
	label = strings.ReplaceAll(label, ":", "/")
	return label
}
