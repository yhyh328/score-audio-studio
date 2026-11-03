#!/usr/bin/env bash
set -euo pipefail

cmake \
  -S apps/portaudio-player \
  -B build/portaudio-player \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build/portaudio-player

#ctest \
#  --test-dir build/portaudio-player \
#  --output-on-failure
