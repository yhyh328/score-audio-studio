# Score Audio Studio

Score Audio Studio is a music-software portfolio project built around an internal score model. The score model is the source of truth; formats such as MIDI and MusicXML are treated as adapters instead of defining the domain model.

## Current status

Phase 1 — Score Domain is complete. Phase 2 — Playback Compiler is now in progress on the `phase-2-playback-compiler` branch.

Phase 1 delivered:

- `ProjectDocument` and `ScoreDocument`
- parts, measures, time signatures, note events, and rest events
- pitch, dynamic, intensity, and tempo models
- PPQ-based integer timing
- pitch-to-MIDI-note-number conversion
- document, tempo-map, entity-ID, measure, event, pitch, and intensity validation
- unit and boundary tests for the public Phase 1 behavior
- reproducible Markdown test-evidence generation

The updated Phase 1 result is recorded in [the Phase 1 test evidence](docs/test-evidence/test-evidence-20260830-025415.md): 257 tests passed from a clean working tree.

## Phase 2 — Playback Compiler

Status: work in progress. Tick compilation and tempo-aware timing primitives are under active development.

The goal of Phase 2 is to transform a validated `ScoreDocument` into a deterministic, playback-oriented representation without coupling the score domain to a synthesizer, DSP engine, or the Web Audio API.

Planned responsibilities:

- consume a validated `ScoreDocument`
- derive absolute score positions from measures and measure-local event offsets
- derive playback context such as part identity without storing playback-only fields in the score domain
- expand note and chord information into an ordered playback timeline
- merge playback events from multiple parts
- produce deterministic ordering for events that share the same score position
- interpret the global tempo map when converting ticks into playback time
- convert score ticks into seconds and, later, sample positions
- report validation or compilation failures without introducing module-global mutable state

The exact public output types will be introduced together with their tests instead of being fixed in the README before implementation.

Phase 2 completion requires:

- a dedicated `playback-compiler` package and public entry point
- tick compilation and tempo-conversion tests, including boundary cases
- note, chord, rest, multi-measure, and multi-part compilation tests
- deterministic ordering tests
- TypeScript, unit-test, and build verification
- updated architecture documentation and reproducible Phase 2 evidence

The following remain outside Phase 2:

- synthesizers, samplers, and SoundFont playback
- General MIDI instrument assignment and program changes
- DSP effects and effect chains
- AudioWorklet integration
- transport and score-editor user interfaces
- MIDI and MusicXML import or export adapters
- raw untrusted-data decoding

## Requirements

- Node.js 24.18.0 (pinned in `.nvmrc`)
- npm 11.16.0 (pinned by the root `packageManager` field)

## Setup and verification

Install the exact dependency versions:

```bash
npm ci
```

Run the complete local verification baseline:

```bash
npm run typecheck
npm test
npm run build
```

Generate a reproducible Markdown test report when release or review evidence is needed:

```bash
npm run test:evidence
```

Generated reports are written under `docs/test-evidence/`. Remove them with `npm run clean:evidence` when they are no longer needed.

## Input contract

The playback compiler consumes a validated `ScoreDocument` from the `score-domain` package.

The relevant input hierarchy is:

```text
ScoreDocument
├── TempoEvent[]
└── Part[]
    └── Measure[]
        └── ScoreEvent[]
            ├── NoteEvent
            │   ├── Pitch[]
            │   └── Intensity
            └── RestEvent
```

The compiler relies on Score Domain validation rather than re-implementing those rules.

In particular, it assumes that:

- PPQ is valid
- the tempo map begins at tick `0` and is validly ordered
- measure numbers and measure ordering are valid within each part
- time-signature numerators are between `1` and `30`
- corresponding measures across parts use the same time signature
- event offsets and durations are valid
- note pitches are valid
- velocity overrides are valid
- entity IDs satisfy the Score Domain validation contract

The compiler derives playback-specific information such as absolute ticks, part context, event ordering, and timing data without adding those fields back into the Score Domain model.

## Expected package structure

```text
packages/playback-compiler/
├── src/
│   ├── model/
│   │   └── playbackEvent.ts
│   ├── compiler/
│   │   ├── compileScoreToTicks.ts
│   │   └── sortTickPlaybackEvents.ts
│   ├── timing/
│   │   ├── buildTempoSegments.ts
│   │   ├── convertTickToAbsoluteSeconds.ts
│   │   ├── tickToSeconds.ts
│   │   └── tickToSamplePosition.ts
│   └── index.ts
├── tests/
├── package.json
└── tsconfig.json
```

This structure represents the current Phase 2 design and may be refined as implementation and tests are introduced. Files should be split only when a concrete responsibility or test boundary justifies the separation.

## Phase 2 boundary

The playback compiler transforms validated score-domain data into deterministic playback-oriented data.

It is responsible for:

- converting measure-local event offsets into global score ticks
- expanding note and chord data into playback events
- ignoring rests as sound-producing events while preserving their timing effect through measure structure
- merging events from multiple parts
- deterministic event ordering
- resolving tempo-aware playback time
- converting ticks into seconds and, later, sample positions

It is not responsible for:

- synthesizers or samplers
- SoundFont or General MIDI instrument playback
- DSP effects
- AudioWorklet integration
- transport state
- score-editor UI
- MIDI or MusicXML import/export
- raw JSON, MIDI, or MusicXML shape decoding

A future adapter or decoding layer must validate raw untrusted data before treating it as a `ScoreDocument`.

The playback compiler schedules musical data but does not generate audio.
