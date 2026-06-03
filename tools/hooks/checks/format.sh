step "clang-format"

if [[ ${#cc_files[@]} -eq 0 ]]; then
  ok "clang-format"
  return
fi

format_issues=()
for file in "${cc_files[@]}"; do
  if ! clang-format --dry-run --Werror "$file" 2>/dev/null; then
    format_issues+=("$file")
  fi
done

if [[ ${#format_issues[@]} -gt 0 ]]; then
  fail "clang-format"formatting issues in:
  for f in "${format_issues[@]}"; do echo "    $f"; done
  echo "  run: clang-format -i ${format_issues[*]}"
  exit 1
fi

ok "clang-format"
