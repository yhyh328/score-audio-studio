# Score Audio Studio

Score Audio Studio is a music-software portfolio project built around an internal score model. The score model is the source of truth; formats such as MIDI and MusicXML will be handled by adapters instead of defining the domain model.

## Current status

The Phase 3 — DSP Core development phase is starting on the `phase-3-dsp-core` branch.

Phase 3 builds on the completed Phase 1 Score Domain and Phase 2 Playback Compiler baselines. Its goal is to introduce a platform-independent C++20 DSP core for sample-accurate note processing, synthesis, polyphony, and basic audio effects.

The planned Phase 3 scope includes:

- a native C++20 `dsp-core` library
- block-based stereo float processing
- block-relative `NoteMessage` input
- sample-accurate NoteOn and NoteOff handling
- sine oscillator synthesis
- ADSR envelope processing
- polyphonic voice management
- deterministic voice stealing
- master gain
- low-pass filtering
- soft clipping
- delay
- native C++ unit and integration tests
- real-time safety constraints
- sanitizer verification
- offline WAV verification through a separate native host
- PortAudio realtime playback through a separate native host

The DSP Core itself will remain independent from `ScoreDocument`, TypeScript package internals, browser APIs, file I/O, and audio-device APIs.

Runtime scheduling, TypeScript-to-C++ adaptation, WebAssembly, AudioWorklet, JUCE/VST3 integration, and score-editor UI belong to later phases.

## Design documentation

The linked Notion documents are currently maintained primarily in Korean. English versions will be provided later for international reviewers.

- [Architecture overview — Korean](https://app.notion.com/p/3a14b2f5a3b0818eb209f90e79bc229e)
- [Phase design index — Korean](https://app.notion.com/p/3c04b2f5a3b0801189c5df2121310ab0)
- [Phase 3 basic design: DSP Core — Korean](https://app.notion.com/p/3c74b2f5a3b0818f8b59d388f2b81619)
- [Phase 3 detailed design: DSP Core — Korean](https://app.notion.com/p/3c74b2f5a3b081d08523f155fdb48aef)
- [Phase 3 environment setup — Korean](https://app.notion.com/p/3c74b2f5a3b0815bb095f024ff8f08dc)

## Requirements

Existing TypeScript baseline:

- Node.js 24.18.0 (pinned in `.nvmrc`)
- npm 11.16.0 (pinned by the root `packageManager` field)

Phase 3 native development:

- Windows 11 + WSL2 Ubuntu
- a C++20-capable GCC or Clang compiler
- CMake
- Ninja
- CTest

A specific compiler major version is not pinned at the start of Phase 3. The actual toolchain baseline can be fixed later when reproducibility requirements are confirmed.

## Setup and verification

Install the existing JavaScript dependencies:

```bash
npm ci
```

Run the existing Phase 1 and Phase 2 verification baseline:

```bash
npm run typecheck
npm test
npm run build
```

Install the native Phase 3 toolchain on WSL2 Ubuntu:

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  ninja-build
```

Verify the native tools:

```bash
g++ --version
cmake --version
ninja --version
```

Once the `dsp-core` skeleton is added, the native build and test loop will use:

```bash
cmake \
  -S packages/dsp-core \
  -B build/dsp-core \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build/dsp-core

ctest \
  --test-dir build/dsp-core \
  --output-on-failure
```

## DSP-core design

The Phase 3 responsibility boundary is:

```text
Score Domain
    ↓
Playback Compiler
    ↓ TickPlaybackEvent[] + timing functions
Runtime / DSP Adapter                  ← later phase
    ↓ block-relative NoteMessage[]
DSP Core
    ├── Voice Manager
    │   └── Voice[]
    │       ├── Oscillator
    │       └── ADSR
    └── Effect Chain
        ├── Master Gain
        ├── Low-pass Filter
        ├── Soft Clip
        └── Delay
    ↓
stereo float audio buffers
```

The DSP Core does not interpret score ticks, tempo maps, absolute sample positions, or score entity IDs.

A later Runtime/DSP Adapter will be responsible for:

- converting playback timing into block-relative `sampleOffset` values
- normalizing velocity for DSP input
- mapping note identity to an opaque numeric `voiceKey`
- providing already ordered `NoteMessage` values to the DSP Core

During Phase 3, native tests can construct `NoteMessage` values directly without requiring that adapter.

The initial processing contract is:

```text
0 <= sampleOffset < frameCount
frameCount <= maximumBlockSize
```

`maximumBlockSize` represents the maximum number of frames accepted by one `process()` call, not the number of blocks.

The real-time processing path will avoid heap allocation, blocking operations, console logging, file I/O, mutex waits, and exceptions.

### Native verification

Audio-device and file-I/O concerns remain outside `dsp-core`.

A separate native verification host will later be used for:

```text
DSP Core
├── offline WAV rendering
└── PortAudio realtime playback
```

Unit and integration tests are added as implementation units are completed rather than being deferred until the end of Phase 3.

Temporary WAV renders may be used during development for debugging oscillator, envelope, and effect behavior. Only representative results need to be retained as final Phase 3 evidence.

The initial CMake/CTest smoke test is an environment check and does not need to remain as final verification evidence.

## Current and planned repository structure

```text
score-audio-studio/
├── apps/                       # Future application packages
├── packages/
│   ├── score-domain/           # Phase 1 domain model and validation
│   ├── playback-compiler/      # Phase 2 playback compilation and timing
│   └── dsp-core/               # Phase 3 C++20 DSP Core
│       ├── CMakeLists.txt
│       ├── include/
│       ├── src/
│       └── tests/
├── docs/
│   └── test-evidence/
├── tools/                      # Development and evidence utilities
├── package.json
├── tsconfig.base.json
├── tsconfig.json
└── vitest.config.ts
```

The `dsp-core` structure will be added incrementally as Phase 3 implementation progresses. Generated CMake and Ninja build outputs remain outside the source tree under `build/`.

## Current boundary

Phase 3 focuses on the reusable native DSP layer.

The following remain outside the `dsp-core` responsibility:

- `ScoreDocument` interpretation
- tick, tempo, seconds, and absolute sample-position calculation
- TypeScript-to-C++ runtime scheduling
- Web Audio API and AudioWorklet integration
- WebAssembly bindings
- JUCE and VST3 integration
- MIDI and MusicXML parsing
- transport look-ahead scheduling
- direct audio-device control
- WAV or other audio-file I/O

PortAudio realtime playback and offline WAV rendering may be used for Phase 3 verification, but they remain separate from the DSP Core library.
