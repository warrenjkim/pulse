#!/usr/bin/env bash

set -euo pipefail

"$1" -x c++ -std=c++23 -fsyntax-only -fplugin="$2" -Xclang -verify "$3"
