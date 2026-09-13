#!/usr/bin/env bash
set -euo pipefail

cmake \
  -S packages/dsp-core \
  -B build/dsp-core \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build/dsp-core

ctest \
  --test-dir build/dsp-core \
  --output-on-failure
