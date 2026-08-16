# Score Audio Studio

Score Audio Studio is a music-software portfolio project built around an internal score model. The score model is the source of truth; formats such as MIDI and MusicXML will be handled by adapters instead of defining the domain model.

## Current status

Phase 1 — Score Domain is implemented on the `phase-1-score-domain` branch.

The current scope includes:

- `ProjectDocument` and `ScoreDocument`
- parts, measures, time signatures, note events, and rest events
- pitch, dynamic, intensity, and tempo models
- PPQ-based integer timing
- pitch-to-MIDI-note-number conversion
- document, tempo-map, entity-ID, measure, event, pitch, and intensity validation
- unit and boundary tests for the public Phase 1 behavior
- reproducible Markdown test-evidence generation

Phase 2 components such as the playback compiler, synthesizer, effects, transport, and AudioWorklet integration have not been implemented.

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

## Score-domain design

The Phase 1 ownership hierarchy is:

```text
ProjectDocument
└── ScoreDocument
    ├── TempoEvent[]
    └── Part[]
        └── Measure[]
            └── ScoreEvent[]
                ├── NoteEvent
                │   └── Pitch[]
                └── RestEvent
```

Important validation rules include:

- schema version `1` for the current project and score formats
- PPQ from `1` through `32767`
- a non-empty tempo map beginning at tick `0`, with unique ascending ticks
- integer BPM from `20` through `300`
- globally unique entity IDs within one score-validation run
- positive, unique, ascending measure numbers within each part
- MIDI note numbers from `0` (`C-1`) through `127` (`G9`)
- note velocity overrides from `1` through `127`

Validation is stateless between calls. An unsupported schema version is fatal; other independent checks continue where their required inputs remain valid.

## Repository structure

```text
score-audio-studio/
├── apps/                       # Future application packages
├── packages/
│   └── score-domain/
│       ├── src/
│       │   ├── conversion/
│       │   ├── model/
│       │   └── validation/
│       └── tests/
├── tools/                      # Test-evidence utilities
├── package.json
├── tsconfig.base.json
├── tsconfig.json
└── vitest.config.ts
```

## Current boundary

The TypeScript validators operate on already decoded domain values. A future adapter or decoding layer must validate the raw shape of untrusted JSON, MIDI, or MusicXML before treating it as a domain document.
