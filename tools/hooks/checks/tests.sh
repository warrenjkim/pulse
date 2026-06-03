step "test $BAZEL_TARGET"

if ! bazel_capture bazel test "${BAZEL_FLAGS[@]}" --test_output=errors --cache_test_results=no "$BAZEL_TARGET"; then
  fail "test $BAZEL_TARGET"
  exit 1
fi

ok "test $BAZEL_TARGET"
