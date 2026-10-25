# Score Audio Studio

Score Audio Studio is a music-software portfolio project built around an internal score model. The score model is the source of truth; formats such as MIDI and MusicXML will be handled by adapters instead of defining the domain model.

## Current status

Phase 3 — DSP Core is currently in progress on the `phase-3-dsp-core` branch.

Phase 3 builds on:

- the completed Phase 1 Score Domain
- the completed Phase 2 Playback Compiler
- the completed Phase 2.5 Playback Runtime

Its goal is to provide a platform-independent C++20 DSP core for sample-accurate note processing, synthesis, polyphony, and basic audio effects.

Current Phase 3 progress:

- [x] native C++20 `dsp-core` library and CMake/CTest baseline
- [x] `PlaybackEvent` contract and `AudioEngine` processing skeleton
- [x] oscillator implementation
- [x] ADSR envelope implementation
- [x] oscillator and ADSR native tests
- [x] oscillator and ADSR visualization evidence
- [x] `Voice`
- [x] `VoiceManager` and polyphony
- [x] Phase 2.5 absolute-sample-position playback boundary
- [ ] sample-accurate event dispatch inside `AudioEngine`
- [ ] deterministic voice stealing
- [ ] master gain
- [ ] low-pass filtering
- [ ] soft clipping
- [ ] delay
- [x] strict compiler warnings for native library and tests
- [x] ASan/UBSan verification in Debug native tests
- [ ] offline WAV verification through a separate native host
- [ ] PortAudio realtime playback through a separate native host

The DSP Core remains independent from `ScoreDocument`, TypeScript package internals, browser APIs, file I/O, and audio-device APIs.

Phase 2.5 provides the runtime boundary between the TypeScript playback compiler and the C++ DSP layer. WebAssembly, AudioWorklet, JUCE/VST3 integration, Unreal integration, and score-editor UI belong to later phases.

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

Alternative band-limited oscillator techniques can be evaluated later if audio-quality or performance requirements justify them.

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

ADSR behavior is verified through native C++ tests. Test output can also be exported as CSV and rendered as envelope graphs for visual verification.

## Playback Runtime boundary

Phase 2.5 introduces the TypeScript-to-C++ playback boundary between the Playback Compiler and DSP Core.

The current playback pipeline is:

```text
Score Domain
    ↓
Playback Compiler
    ↓
TickPlaybackEvent[]
    ↓
SamplePlaybackEvent[]
    ↓
Binary playback buffer
    ↓
Playback Runtime
    ↓
current-block PlaybackEvent[]
    ↓
DSP Core
```

Phase 2 calculates absolute sample positions from score timing.

```text
tick
↓
tempo-aware timing conversion
↓
absolute samplePosition
```

`samplePosition` is expressed as an unsigned 32-bit frame index relative to the beginning of the score timeline.

The current playback buffer uses a versioned binary format with a 24-byte header and 16-byte event records.

```text
Playback event record

offset  size  field
0       1     type
1       1     MIDI note
2       2     partId
4       4     noteId
8       4     samplePosition
12      4     velocity

total: 16 bytes
```

Phase 2.5 validates and owns the decoded playback events, then selects only the events belonging to the current audio block.

The current block is described by:

```text
blockStartFrame
frameCount
```

Both are represented as unsigned 32-bit frame values.

An event belongs to the current block when:

```text
samplePosition >= blockStartFrame

and

samplePosition - blockStartFrame < frameCount
```

The subtraction-based range check avoids unsigned overflow from computing:

```text
blockStartFrame + frameCount
```

directly.

Velocity is normalized before DSP consumption so that the DSP-side range is:

```text
0.0f <= velocity <= 1.0f
```

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

Native development:

- Windows 11 + WSL2 Ubuntu
- a C++20-capable GCC or Clang compiler
- CMake
- Ninja
- CTest

A specific compiler major version is not currently pinned. The actual toolchain baseline can be fixed later when reproducibility requirements are confirmed.

## Setup and verification

Install the JavaScript dependencies:

```bash
npm ci
```

Run the TypeScript verification baseline:

```bash
npm run typecheck
npm test
npm run build
```

Install the native toolchain on WSL2 Ubuntu:

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

The DSP Core build and test loop uses:

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

The same DSP verification sequence is available through:

```bash
./tools/verify-dsp-core.sh
```

## DSP-core design

The current responsibility boundary is:

```text
Score Domain
    ↓
Playback Compiler
    ├── TickPlaybackEvent[]
    └── absolute samplePosition calculation
    ↓
Playback Runtime
    ├── binary validation / decoding
    ├── owned playback-event storage
    └── current-block event selection
    ↓
PlaybackEvent[] with absolute samplePosition
    ↓
DSP Core
    ├── AudioEngine
    │    └── samplePosition - blockStartFrame
    │         → block-relative sampleOffset
    │
    ├── Voice Manager
    │    └── Voice[]
    │         ├── Oscillator
    │         └── ADSR
    │
    └── Effect Chain
         ├── Master Gain
         ├── Low-pass Filter
         ├── Soft Clip
         └── Delay
    ↓
stereo float audio buffers
```

The DSP Core does not interpret:

- score ticks
- tempo maps
- `ScoreDocument`
- TypeScript compiler internals

It does, however, receive absolute playback `samplePosition` values for events belonging to the current block.

`AudioEngine::process()` also receives the current `blockStartFrame`.

The block-relative position used internally for rendering is calculated as:

```cpp
const std::uint32_t sampleOffset =
    event.samplePosition - blockStartFrame;
```

Phase 2.5 guarantees that only events belonging to the current block are provided to the DSP Core.

Therefore:

```text
samplePosition >= blockStartFrame

samplePosition - blockStartFrame < frameCount
```

must hold for every event passed to one `process()` call.

The current DSP-side event contract is:

```cpp
struct PlaybackEvent {
    PlaybackEventType type;
    std::uint8_t midiNoteNumber;
    std::uint16_t partId;
    std::uint32_t noteId;
    std::uint32_t samplePosition;
    float velocity;
};
```

The current processing API conceptually operates on:

```cpp
process(
    left,
    right,
    frameCount,
    blockStartFrame,
    events
);
```

where:

```text
frameCount
```

is the number of audio frames requested for the current processing block, while:

```text
blockStartFrame
```

is the absolute frame position of the first frame in that block.

The real-time processing path avoids:

- heap allocation
- blocking operations
- console logging
- file I/O
- mutex waits
- exceptions

Binary decoding, process creation, and other transport work are performed outside the real-time audio processing path.

### Sample-accurate event processing

A host may request a block such as:

```text
frameCount = 512
```

while events may occur at different positions within that block.

For example:

```text
blockStartFrame = 24000

event A samplePosition = 24100
event B samplePosition = 24300
```

The DSP Core derives:

```text
event A sampleOffset = 100
event B sampleOffset = 300
```

and can internally render:

```text
[0, 100)
apply event A

[100, 300)
apply event B

[300, 512)
```

The external audio block remains 512 frames. The internal segmentation exists only to apply playback events at sample-accurate positions.

### Native verification

Audio-device and file-I/O concerns remain outside `dsp-core`.

A separate native verification host will later be used for:

```text
DSP Core
├── offline WAV rendering
└── PortAudio realtime playback
```

Unit and integration tests are added as implementation units are completed rather than being deferred until the end of Phase 3.

Oscillator and ADSR milestones also use lightweight visualization tools as supplementary evidence. These visualizations do not replace native correctness tests; they make waveform and envelope behavior easier to inspect.

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
│   ├── playback-runtime/       # Phase 2.5 TS-to-C++ playback runtime
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

Generated CMake and Ninja build outputs remain outside the source tree under `build/`.

## Next Phase 3 milestones

The current implementation sequence is:

```text
Oscillator                     completed
    ↓
ADSR                           completed
    ↓
Voice                          completed
    ↓
VoiceManager / polyphony       completed
    ↓
Phase 2.5 playback runtime     completed
    ↓
sample-accurate event dispatch
    ↓
Gain / LPF / Soft Clip / Delay
    ↓
integrated native verification
```

## Current boundary

Phase 3 focuses on the reusable native DSP layer.

The following remain outside the `dsp-core` responsibility:

- `ScoreDocument` interpretation
- tick and tempo-map interpretation
- tick-to-absolute-sample conversion
- TypeScript-to-C++ binary decoding
- current-block event selection
- Web Audio API and AudioWorklet integration
- WebAssembly bindings
- JUCE and VST3 integration
- Unreal Engine integration
- MIDI and MusicXML parsing
- direct audio-device control
- WAV or other audio-file I/O

Phase 3 does receive the current block's absolute playback positions and derives block-relative sample offsets internally for sample-accurate processing.

PortAudio realtime playback and offline WAV rendering may be used for Phase 3 verification, but they remain separate from the DSP Core library.