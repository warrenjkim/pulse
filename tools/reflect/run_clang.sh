#!/usr/bin/env bash
#
# Runs `$@`, pointing upstream clang at the macOS SDK. Apple's clang finds the
# SDK on its own; upstream clang reads `SDKROOT`.

set -euo pipefail

if [[ "$(uname)" == "Darwin" && -z "${SDKROOT:-}" ]]; then
  SDKROOT="$(xcrun --show-sdk-path)"
  export SDKROOT
fi

exec "$@"
