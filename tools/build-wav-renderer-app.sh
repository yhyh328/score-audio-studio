#!/usr/bin/env bash
set -euo pipefail

cmake \
  -S apps/wav-renderer \
  -B build/wav-renderer \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=off

cmake --build build/wav-renderer
