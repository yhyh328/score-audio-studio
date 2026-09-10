# Score Audio Studio

Score Audio Studio is a music-software portfolio project built around an internal score model. The score model is the source of truth; formats such as MIDI and MusicXML will be handled by adapters instead of defining the domain model.

## Current status

The Phase 2 — Playback Compiler implementation baseline is complete on the `phase-2-playback-compiler` branch.

The current Phase 2 code passes the verification baseline and builds on the completed Phase 1 score domain. It accepts a ScoreDocument, delegates validation to score-domain,
and transforms valid score data into deterministic playback events and tempo-aware timing values.

The current scope includes:

- `TickPlaybackEvent` compilation
- measure-local tick to absolute score-tick conversion
- note and chord expansion into note-on and note-off events
- multi-measure and multi-part event merging
- deterministic playback-event ordering
- tempo-segment construction
- tick to absolute-seconds conversion
- tick to absolute sample-position conversion
- input validation delegated to `score-domain` at the public compiler boundary
- a public package entry point
- unit and boundary tests for the implemented Phase 2 behavior
- reproducible Markdown test-evidence generation

Synthesizers, samplers, DSP effects, transport, score-editor UI, and AudioWorklet integration belong to later phases.

## Design documentation

The linked Notion documents are currently maintained primarily in Korean. English versions will be provided later for international reviewers.

- [Architecture overview — Korean](https://app.notion.com/p/3a14b2f5a3b0818eb209f90e79bc229e)
- [Phase design index — Korean](https://app.notion.com/p/3c04b2f5a3b0801189c5df2121310ab0)
- [Phase 2 detailed design: Playback Compiler — Korean](https://app.notion.com/p/Phase-2-Playback-Compiler-3c04b2f5a3b081598bbfd1b0a94abad5)
- [Repository verification evidence after Phase 2 — 295 tests](docs/test-evidence/test-evidence-20260910-222343.md)

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

## Playback-compiler design

The Phase 2 compilation and timing flow is:

```text
ScoreDocument
└── compileScoreToTicks()
    ├── validateScoreDocument()
    │   └── invalid → validation issues
    └── valid
        ├── TickPlaybackEvent[]
        └── scoreEndTick

TempoEvent[]
└── buildTempoSegments()
    └── TempoSegment[]

TickPlaybackEvent + TempoSegment[]
└── tickToSamplePosition()
    └── sample position
```

The public `compileScoreToTicks()` entry point accepts a `ScoreDocument` and delegates validation to `score-domain` before compilation. Invalid input is returned as validation issues rather than being compiled.

After validation succeeds, the compiler relies on the Score Domain invariants, including valid PPQ, tempo events, measures, time signatures, score events, pitches, velocities, and entity IDs. Corresponding measures across parts must use the same time signature; explicit polymeter support remains outside the current scope.

The current Phase 2 compilation and timing policy is:

- measure-local event offsets are converted into absolute score ticks
- each pitch in a note or chord produces a note-on and note-off event
- rests do not produce playback events, while measure structure still advances score time
- events from all parts are merged into one playback timeline
- events are ordered by tick, with note-off before note-on and MIDI note number as the next ordering key
- `scoreEndTick` is the end tick of the longest part
- tempo events are converted into accumulated constant-tempo segments
- tick positions are converted to absolute seconds through the applicable tempo segment
- sample positions are rounded to the nearest integer and must remain within the non-negative safe-integer range
- invalid sample rates or unrepresentable sample positions are rejected

### Public API policy

Package-root exports expose the concrete Phase 2 API currently needed by consumers:

- `compileScoreToTicks()`
- `buildTempoSegments()`
- `tickToSamplePosition()`
- `TickPlaybackEvent`
- `TempoSegment`

Internal timing helpers remain internal until a concrete external or cross-package use case requires them.

## Current and planned repository structure

```text
score-audio-studio/
├── apps/                       # Future application packages
├── packages/
│   ├── score-domain/           # Phase 1 domain model and validation
│   └── playback-compiler/
│       ├── src/
│       │   ├── compiler/
│       │   ├── model/
│       │   ├── timing/
│       │   └── utils/
│       └── tests/
│           ├── compiler/
│           └── timing/
├── tools/                      # Test-evidence utilities
├── package.json
├── tsconfig.base.json
├── tsconfig.json
└── vitest.config.ts
```

## Current boundary

The playback compiler schedules musical data but does not generate audio. Synthesizers, samplers, General MIDI program assignment, DSP effects, AudioWorklet integration, transport state, score-editor UI, and MIDI or MusicXML adapters remain outside Phase 2.

The TypeScript compiler APIs operate on already decoded domain values. A future adapter or decoding layer must validate the raw shape of untrusted JSON, MIDI, or MusicXML before treating it as a `ScoreDocument`.
