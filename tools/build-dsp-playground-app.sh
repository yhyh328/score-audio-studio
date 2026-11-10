#!/usr/bin/env bash
set -euo pipefail

cmake \
  -S apps/dsp-playground \
  -B build/dsp-playground \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build/dsp-playground
