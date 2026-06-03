step "cc_deps $BAZEL_TARGET"

if ! bazel_capture bazel run "${BAZEL_FLAGS[@]}" //tools:cc_deps -- "$BAZEL_TARGET"; then
  fail "cc_deps"
  exit 1
fi

ok "cc_deps $BAZEL_TARGET"
