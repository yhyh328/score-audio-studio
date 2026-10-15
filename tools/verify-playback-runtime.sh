#!/usr/bin/env bash
set -euo pipefail

cmake \
  -S packages/playback-runtime \
  -B build/playback-runtime \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug

cmake \
  --build build/playback-runtime \
  --clean-first

ctest \
  --test-dir build/playback-runtime \
  --output-on-failure
