# Score Audio Studio

Score Audio Studio is a music-software portfolio project built around an internal score model. The score model is the source of truth; formats such as MIDI and MusicXML will be handled by adapters instead of defining the domain model.

## Current status

The Phase 1 — Score Domain implementation baseline is complete on the `phase-1-score-domain` branch.

The current Phase 1 code passes the verification baseline and reflects the current validation and public-API decisions. It provides the domain foundation for the Phase 2 playback compiler.

The current scope includes:

- `ProjectDocument` and `ScoreDocument`
- parts, measures, time signatures, note events, and rest events
- pitch, dynamic, intensity, and tempo models
- PPQ-based integer timing
- pitch-to-MIDI-note-number conversion
- document, tempo-map, entity-ID, measure, event, pitch, and intensity validation
- unit and boundary tests for the implemented Phase 1 behavior
- reproducible Markdown test-evidence generation

The Phase 2 playback compiler is outside this branch. Synthesizers, effects, transport, and AudioWorklet integration belong to later phases.

## Design documentation

The linked Notion documents are currently maintained primarily in Korean. English versions will be provided later for international reviewers.

- [Architecture overview — Korean](https://app.notion.com/p/3a14b2f5a3b0818eb209f90e79bc229e)
- [Phase design index — Korean](https://app.notion.com/p/3c04b2f5a3b0801189c5df2121310ab0)
- [Phase 1 detailed design: Score Domain — Korean](https://app.notion.com/p/3c04b2f5a3b0819fb288c444e32d0edb)
- [Phase 1 test evidence](docs/test-evidence/test-evidence-20260825-200609.md)

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
                │   ├── Pitch[]
                │   └── Intensity
                └── RestEvent
```

The current Phase 1 validation policy is:

* schema version `1` for the current project and score formats
* PPQ from `1` through `32767`
* a non-empty tempo map beginning at tick `0`, with unique ascending ticks
* integer BPM values from `20` through `300`
* globally unique entity IDs within one score-validation run
* positive integer measure numbers that are unique within each part and stored in ascending order
* MIDI note numbers from `0` (`C-1`) through `127` (`G9`)
* note velocity overrides from `1` through `127`

Validation is stateless between calls. An unsupported schema version is fatal; other independent checks continue where their required inputs remain valid.

`pitchToMidiNoteNumber()` remains a pure domain conversion. Invalid pitch data is handled by validation rather than by throwing from the conversion function.

### Public API policy

Package-root exports are intentionally kept minimal.

Internal domain types and utilities are exposed through the package root only when there is a concrete external or cross-package use case for them. Additional public exports can be added later as those dependencies appear.

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