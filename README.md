# @score-audio-studio/playback-runtime

Runtime adapter layer between the Phase 2 Playback Compiler and the Phase 3 DSP Core.

## Introduction

Playback Runtime was introduced while integrating Phase 2 with the Phase 3 DSP Core.

The original design assumed that Playback Compiler output could be consumed directly by the DSP Core. During the implementation of `AudioEngine`, `VoiceManager`, and sample-accurate event dispatch, however, a separate runtime layer became necessary to handle responsibilities such as:

- tick-to-frame conversion
- block-relative event scheduling
- part-to-instrument routing
- runtime event generation

These responsibilities are too runtime-oriented for the Playback Compiler and too dependent on compiler-level information for the DSP Core.

For this reason, the layer was extracted as **Phase 2.5 — Playback Runtime**.

## Phase

**Phase 2.5 — Playback Runtime**

```text
Score Domain
    ↓
Playback Compiler
    ↓
Playback Runtime
    ↓
DSP Core / AudioEngine
```

The Playback Compiler represents musical time using ticks.

The DSP Core processes audio using frames and block-relative event offsets.

This package bridges those two representations.

To enable `dsp-core` to consume playback events directly from memory
without recalculating playback timing, the Phase 2 `playback-compiler`
package provides `encodePlaybackEvents`.

## Required Phase 2 extension

Although Playback Runtime belongs to Phase 2.5 and is implemented in C++,
establishing the TypeScript-to-C++ boundary required a minimal extension
to the Phase 2 playback compiler.

Phase 2 now converts score events into absolute sample-position events,
normalizes domain identifiers into fixed-width numeric fields, and encodes
the result using the 16-byte little-endian binary layout consumed by the
native runtime.

This extension is limited to producing the handoff format. Binary loading,
event ownership, block scheduling, and delivery to the DSP engine remain
the responsibility of Phase 2.5.

## Design documentation

The linked Notion documents are currently maintained primarily in Korean. English versions will be provided later for international reviewers.

- [Phase 2.5 playback runtime design](https://app.notion.com/p/Phase-2-5-Playback-Runtime-3fa4b2f5a3b08193b3e4e1336f4ecfc9)
- [Phase 2(Playback Compiler) verification results — 299 tests](docs/test-evidence/test-evidence-20261023-214043.md)

## Responsibilities

The Playback Runtime is responsible for converting compiler output into data that can be consumed by the realtime audio engine.

Planned responsibilities include:

- converting tick positions to absolute frame positions
- converting absolute frame positions to block-relative event offsets
- mapping score/compiler part identifiers to runtime instrument indices
- selecting events that belong to the current audio block
- preserving deterministic event ordering
- producing runtime playback events for the DSP Core

## Time Representation

Phase 2 uses musical time:

```text
tick
```

Tick positions depend on:

- PPQ
- tempo map

Phase 3 uses audio time:

```text
frame
```

Frame positions additionally depend on:

- sample rate

Example:

```text
PPQ: 480
Tempo: 120 BPM
Sample rate: 48,000 Hz

480 ticks
→ 0.5 seconds
→ 24,000 frames
```

## Block-relative Event Offset

A runtime playback event contains an offset relative to the beginning of the current audio block.

```text
Audio block: 512 frames

0 ---------------- 100 ---------------- 300 ---------------- 511
                  NoteOn              NoteOff
```

In this example:

```ini
NoteOn.sampleOffset  = 100
NoteOff.sampleOffset = 300
```

Although the field is currently named `sampleOffset`, its unit is a **block-relative frame offset**.

One frame contains one sample per channel.

## Event Ordering

When multiple events occur at the same position, the runtime preserves the Phase 2 tie-break policy:

```text
noteOff
before
noteOn
```

This allows an existing note to be released before another note begins at the same frame.

## Instrument Routing

Phase 2 identifies musical parts using logical identifiers such as `partId`.

The DSP Core should not depend directly on those score-domain identifiers.

The runtime layer therefore converts them into runtime routing information.

```text
partId
    ↓
instrumentIndex
```

Example:

```text
"violin-1" → 0
"cello"    → 1
```

The resulting `instrumentIndex` can be used by `AudioEngine` to route an event to the appropriate instrument.

## Non-responsibilities

This package does not perform DSP or audio rendering.

It does not implement:

- oscillators
- envelopes
- voice rendering
- voice allocation
- effects
- audio-device I/O
- JUCE, PortAudio, or Unreal-specific processing

Those responsibilities belong to the DSP Core or platform integration layers.

## Current Status

Work in progress.

Initial goals:

- define the Phase 2 → runtime input boundary
- implement tick-to-frame conversion
- implement block-relative event conversion
- implement part-to-instrument routing
- preserve deterministic event ordering
- define the minimal runtime playback event contract
- add unit tests

## Playback-runtime design
The minimal Phase 2.5 compilation and timing flow is:
```text
packages/playback-runtime/
├── CMakeLists.txt
├── include/
│   └── score_audio_studio/
│       └── playback_runtime/
│           └── PlaybackRuntime.hpp
├── src/
│   └── PlaybackRuntime.cpp
└── tests/
    ├── CMakeLists.txt
    └── PlaybackRuntimeTest.cpp
```
