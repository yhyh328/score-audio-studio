# Score Audio Studio

Score Audio Studio is a music-software portfolio project for representing musical scores as structured domain data, compiling them into playback events, and eventually connecting them to synthesis, DSP, and interactive score editing.

The internal score model is the source of truth. Formats such as MIDI and MusicXML are treated as adapters rather than as the core domain model.

## Architecture

```text
Score Editor / External Formats
            ↓
     Internal Score Model
            ↓
      Playback Compiler
            ↓
     Synth / DSP Pipeline
            ↓
        Audio Output
```

The project is developed incrementally so that score semantics, playback timing, audio processing, and UI concerns remain separated.

## Project status

| Phase | Scope | Status |
| --- | --- | --- |
| Phase 1 | Score Domain | Complete |
| Phase 2 | Playback Compiler | Starting |
| Later phases | Synthesis, DSP, effects, UI, audio integration | Planned |

### Phase 1 — Score Domain

The Phase 1 baseline provides:

- `ProjectDocument` and `ScoreDocument`
- parts, measures, time signatures, note events, and rest events
- pitch, dynamic, intensity, and tempo models
- PPQ-based integer timing
- pitch-to-MIDI-note-number conversion
- score and domain validation
- unit and boundary tests
- reproducible test-evidence generation

The final Phase 1 verification passed all 246 test cases from a clean working tree.

### Phase 2 — Playback Compiler

Phase 2 builds on the Score Domain without moving playback responsibilities into the domain package.

Its responsibility is to compile structured score data into deterministic playback-oriented events and timing information.

Development takes place on:

`phase-2-playback-compiler`

## Design principles

- The internal score model is independent of MIDI and MusicXML.
- Score Domain owns musical meaning and validation.
- Playback compilation is a separate responsibility.
- DSP and audio rendering remain independent from score validation.
- Package-root APIs are kept minimal and expanded only for concrete consumers.
- Persisted schema versions represent data compatibility, not development phase numbers.

## Requirements

- Node.js 24.18.0
- npm 11.16.0

The exact Node.js version is pinned in `.nvmrc`, and npm is pinned through the root `packageManager` field.

## Setup

Install dependencies:

```bash
npm ci
```

Run the verification baseline:

```bash
npm run typecheck
npm test
npm run build
```

Generate reproducible test evidence when required:

```bash
npm run test:evidence
```

## Repository structure

```text
score-audio-studio/
├── apps/                       # Application packages
├── packages/
│   └── score-domain/           # Phase 1 domain model and validation
├── docs/
│   └── test-evidence/          # Reproducible verification evidence
├── tools/                      # Development and evidence utilities
├── package.json
├── tsconfig.base.json
├── tsconfig.json
└── vitest.config.ts
```

The Playback Compiler package will be developed as the Phase 2 implementation progresses.

## Verification

Phase 1 final verification:

- Evidence ID: `TEST-20260825-205047`
- Result: PASS
- Test files: 13 / 13
- Test cases: 246 / 246
- Working tree: clean
- Environment: Node.js 24.18.0 / npm 11.16.0
- Time zone: Asia/Tokyo

[View Phase 1 test evidence](docs/test-evidence/test-evidence-20260825-205047.md)

## Design documentation

The linked design documents are currently maintained primarily in Korean.

- [Architecture overview — Korean](https://app.notion.com/p/3a14b2f5a3b0818eb209f90e79bc229e)
- [Phase design index — Korean](https://app.notion.com/p/3c04b2f5a3b0801189c5df2121310ab0)
- [Phase 1 detailed design — Korean](https://app.notion.com/p/3c04b2f5a3b0819fb288c444e32d0edb)

## Branch strategy

- `main` — stable integrated project baseline
- `phase-1-score-domain` — completed Phase 1 baseline
- `phase-2-playback-compiler` — Phase 2 development

Completed phases are integrated into `main`, and each new phase starts from the latest stable `main`.