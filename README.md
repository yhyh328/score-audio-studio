# Score Audio Studio

Score Audio Studio is a music-software portfolio project built around an internal score model. The score model is the source of truth; formats such as MIDI and MusicXML will be handled by adapters instead of defining the domain model.

## Current status

Phase 3 — DSP Core is currently in progress on the `phase-3-dsp-core` branch.

Phase 3 builds on the completed Phase 1 Score Domain and Phase 2 Playback Compiler baselines. Its goal is to introduce a platform-independent C++20 DSP core for sample-accurate note processing, synthesis, polyphony, and basic audio effects.

Current Phase 3 progress:

- [x] native C++20 `dsp-core` library and CMake/CTest baseline
- [x] `PlaybackEvent` contract and `AudioEngine` silence-processing skeleton
- [x] oscillator implementation
- [x] ADSR envelope implementation
- [x] oscillator and ADSR native tests
- [x] oscillator and ADSR visualization evidence
- [ ] `Voice`
- [ ] `VoiceManager` and polyphony
- [ ] sample-offset event dispatch
- [ ] deterministic voice stealing
- [ ] master gain
- [ ] low-pass filtering
- [ ] soft clipping
- [ ] delay
- [x] ASan/UBSan verification in Debug native tests
- [ ] offline WAV verification through a separate native host
- [ ] PortAudio realtime playback through a separate native host

The DSP Core itself will remain independent from `ScoreDocument`, TypeScript package internals, browser APIs, file I/O, and audio-device APIs.

Runtime scheduling, TypeScript-to-C++ adaptation, WebAssembly, AudioWorklet, JUCE/VST3 integration, and score-editor UI belong to later phases.

## Implemented DSP components

### Oscillator

The current oscillator supports:

- Sine
- Triangle
- Square
- Sawtooth
- additive harmonic synthesis
- configurable harmonic count up to the internal limit
- Nyquist filtering of harmonics by default
- explicit aliasing opt-in for experiments
- normalized phase wrapping
- MIDI note number to frequency conversion

The current implementation uses harmonic synthesis rather than PolyBLEP.
Alternative band-limited oscillator techniques can be evaluated later if
audio-quality or performance requirements justify them.

### ADSR envelope

The ADSR implementation currently supports:

- `Idle → Attack → Decay → Sustain → Release → Idle`
- Linear and Exponential curves
- stage duration quantization from seconds to sample counts
- sample-count-based stage completion
- exact target-gain clamping at stage completion
- release from the current envelope level
- zero-sample Attack, Decay, and Release transitions
- consecutive zero-sample stage transitions without consuming audio samples

ADSR behavior is verified through native C++ tests. Test output can also be
exported as CSV and rendered as envelope graphs for visual verification.

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

The native DSP Core build and test loop uses:

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

The same verification sequence is available through:

```bash
./tools/verify-dsp-core.sh
```

## DSP-core design

The Phase 3 responsibility boundary is:

```text
Score Domain
    ↓
Playback Compiler
    ↓ TickPlaybackEvent[] + timing functions
Runtime / DSP Adapter                  ← later phase
    ↓ block-relative PlaybackEvent[]
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
- providing already ordered `PlaybackEvent` values to the DSP Core

During Phase 3, native tests can construct `PlaybackEvent` values directly without requiring that adapter.

The current DSP-side event value is intentionally small and platform-independent:

```cpp
struct PlaybackEvent {
    PlaybackEventType type;
    std::uint32_t sampleOffset;
    std::uint32_t voiceKey;
    std::uint8_t midiNoteNumber;
    float velocity;
};
```

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

Oscillator and ADSR milestones also use lightweight visualization tools as
supplementary evidence. These visualizations do not replace native correctness
tests; they make waveform and envelope behavior easier to inspect.

Current evidence includes:

- oscillator visualization
- ADSR CSV generation
- Linear and Exponential ADSR envelope plots

Temporary WAV renders may be used during development for debugging oscillator, envelope, and effect behavior. Only representative results need to be retained as final Phase 3 evidence.

The initial CMake/CTest smoke test is an environment check and does not need to remain as final verification evidence.

## Repository structure

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
│   ├── Adsr-Envelope/
│   ├── Oscillator/
│   └── test-evidence/
├── tools/                      # Verification and visualization utilities
├── package.json
├── tsconfig.base.json
├── tsconfig.json
└── vitest.config.ts
```

The `dsp-core` structure continues to grow incrementally as Phase 3 implementation progresses. Generated CMake and Ninja build outputs remain outside the source tree under `build/`.

## Next Phase 3 milestones

The immediate implementation sequence is:

```text
Oscillator       completed
    ↓
ADSR             completed
    ↓
Voice            next
    ↓
VoiceManager / polyphony
    ↓
sample-offset event dispatch
    ↓
Gain / LPF / Soft Clip / Delay
    ↓
integrated native verification
```

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
