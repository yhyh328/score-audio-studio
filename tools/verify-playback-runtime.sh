#!/usr/bin/env bash
set -euo pipefail

NODE_INCLUDE_DIR="$(
    dirname "$(dirname "$(command -v node)")"
)/include/node"

cmake \
  -S packages/playback-runtime \
  -B build/playback-runtime \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_PLAYBACK_RUNTIME_NODE_ADDON=ON \
  -DNODE_INCLUDE_DIR="$NODE_INCLUDE_DIR"

cmake \
  --build build/playback-runtime \
  --clean-first

ctest \
  --test-dir build/playback-runtime \
  --output-on-failure

node \
  packages/playback-runtime/tests/ShareBufferSmoke.js \
  build/playback-runtime/playback_runtime_shared_buffer_addon.node
