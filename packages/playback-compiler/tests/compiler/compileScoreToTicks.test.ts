/**
 * 1. phase 1에 같은 마디면 어느 파트더라도 같은 타임 시그너쳐를 써야 한다는 제약.
 * 2. 타임 시그너쳐의 분자는 30으로 제한하기
 */

import { describe, expect, it } from "vitest";
import { 
    compileScoreToTicks 
} from "../../src/compiler/compileScoreToTicks";
import type { ScoreDocument } from "@score-audio-studio/score-domain";
import { DEFAULT_PPQ } from "@score-audio-studio/score-domain";

const uuid = () => crypto.randomUUID();

function createScore(): ScoreDocument {
  return {
    schemaVersion: 1,
    ppq: DEFAULT_PPQ,
    tempoMap: [
        { tick: 0, bpm: 100 },
        { tick: 1, bpm: 120 },
        { tick: 100, bpm: 110 },
        { tick: 480, bpm: 200 },
        { tick: 5000, bpm: 50 },
        { tick: 9999, bpm: 20 },
        { tick: Number.MAX_SAFE_INTEGER - 2, bpm: 300 }
    ],
    parts: [
      {
        id: `part-${uuid()}`,
        name: "Piano",
        measures: [
          {
            id: `measure-${uuid()}`,
            number: 1,
            timeSignature: { numerator: 4, denominator: 4 },
            events: [
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 0,
                durationTicks: DEFAULT_PPQ,
                pitches: [
                  { step: "C", alter: 0, octave: 4 },
                  { step: "E", alter: 0, octave: 4 },
                  { step: "G", alter: 0, octave: 4 },
                ],
                intensity: { dynamic: "mf" },
              },
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: DEFAULT_PPQ,
                durationTicks: DEFAULT_PPQ,
                pitches: [{ step: "F", alter: 1, octave: 4 }],
                intensity: { dynamic: "f", velocity: 96 },
              },
              {
                type: "rest",
                id: `event-${uuid()}`,
                offsetTicks: DEFAULT_PPQ * 2,
                durationTicks: DEFAULT_PPQ,
              },
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: DEFAULT_PPQ * 3,
                durationTicks: DEFAULT_PPQ,
                pitches: [{ step: "B", alter: -1, octave: 4 }],
                intensity: { dynamic: "p" },
              },
            ],
          },
          {
            id: `measure-${uuid()}`,
            number: 2,
            timeSignature: { numerator: 3, denominator: 4 },
            events: [
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 0,
                durationTicks: DEFAULT_PPQ + DEFAULT_PPQ / 2,
                pitches: [{ step: "A", alter: 0, octave: 3 }],
                intensity: { dynamic: "pp" },
              },
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: DEFAULT_PPQ + DEFAULT_PPQ / 2,
                durationTicks: DEFAULT_PPQ + DEFAULT_PPQ / 2,
                pitches: [
                  { step: "D", alter: 0, octave: 4 },
                  { step: "F", alter: 0, octave: 4 },
                  { step: "A", alter: 0, octave: 4 },
                ],
                intensity: { dynamic: "ff" },
              },
            ],
          },
          {
            id: `measure-${uuid()}`,
            number: 3,
            timeSignature: { numerator: 2, denominator: 4 },
            events: [
              {
                type: "rest",
                id: `event-${uuid()}`,
                offsetTicks: 0,
                durationTicks: DEFAULT_PPQ,
              },
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: DEFAULT_PPQ,
                durationTicks: DEFAULT_PPQ,
                pitches: [{ step: "C", alter: 0, octave: 5 }],
                intensity: { dynamic: "fff" },
              },
            ],
          },
        ],
      },
      {
        id: `part-${uuid()}`,
        name: "Violin",
        measures: [
          {
            id: `measure-${uuid()}`,
            number: 1,
            timeSignature: { numerator: 4, denominator: 4 },
            events: [
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 0,
                durationTicks: DEFAULT_PPQ * 4,
                pitches: [{ step: "G", alter: 0, octave: 4 }],
                intensity: { dynamic: "mp" },
              },
            ],
          },
          {
            id: `measure-${uuid()}`,
            number: 2,
            timeSignature: { numerator: 6, denominator: 8 },
            events: [
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 0,
                durationTicks: DEFAULT_PPQ,
                pitches: [{ step: "A", alter: 0, octave: 4 }],
                intensity: { dynamic: "p" },
              },
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: DEFAULT_PPQ,
                durationTicks: DEFAULT_PPQ,
                pitches: [{ step: "B", alter: 0, octave: 4 }],
                intensity: { dynamic: "mf" },
              },
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: DEFAULT_PPQ * 2,
                durationTicks: DEFAULT_PPQ,
                pitches: [{ step: "C", alter: 1, octave: 5 }],
                intensity: { dynamic: "f" },
              },
            ],
          },
        ],
      },
      {
        id: `part-${uuid()}`,
        name: "Guitar",
        measures: [
          {
            id: `measure-${uuid()}`,
            number: 1,
            timeSignature: { numerator: 1, denominator: 1 },
            events: [
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 0,
                durationTicks: 1,
                pitches: [{ step: "C", alter: 0, octave: 3 }],
                intensity: { dynamic: "mf" },
              },
            ],
          },
          {
            id: `measure-${uuid()}`,
            number: 2,
            timeSignature: { numerator: 5, denominator: 2 },
            events: [
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 0,
                durationTicks: 1,
                pitches: [{ step: "D", alter: 0, octave: 3 }],
                intensity: { dynamic: "mf" },
              },
            ],
          },
          {
            id: `measure-${uuid()}`,
            number: 3,
            timeSignature: { numerator: 100, denominator: 8 },
            events: [
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 0,
                durationTicks: 1,
                pitches: [{ step: "E", alter: 0, octave: 3 }],
                intensity: { dynamic: "mf" },
              },
            ],
          },
          {
            id: `measure-${uuid()}`,
            number: 4,
            timeSignature: { numerator: 1000, denominator: 16 },
            events: [
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 0,
                durationTicks: 1,
                pitches: [{ step: "F", alter: 0, octave: 3 }],
                intensity: { dynamic: "mf" },
              },
            ],
          },
        ],
      },
      {
        id: `part-${uuid()}`,
        name: "Bass",
        measures: [
          {
            id: `measure-${uuid()}`,
            number: 1,
            timeSignature: {
              numerator: Number.MAX_SAFE_INTEGER,
              denominator: 16,
            },
            events: [
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 0,
                durationTicks: 1,
                pitches: [{ step: "C", alter: 0, octave: 4 }],
                intensity: { dynamic: "mf" },
              },
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 1,
                durationTicks: 1,
                pitches: [{ step: "D", alter: 0, octave: 4 }],
                intensity: { dynamic: "mf" },
              },
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 100,
                durationTicks: 1,
                pitches: [{ step: "E", alter: 0, octave: 4 }],
                intensity: { dynamic: "mf" },
              },
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 480,
                durationTicks: 1,
                pitches: [{ step: "F", alter: 0, octave: 4 }],
                intensity: { dynamic: "mf" },
              },
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 5000,
                durationTicks: 1,
                pitches: [{ step: "G", alter: 0, octave: 4 }],
                intensity: { dynamic: "mf" },
              },
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: 9999,
                durationTicks: 1,
                pitches: [{ step: "A", alter: 0, octave: 4 }],
                intensity: { dynamic: "mf" },
              },
              {
                type: "note",
                id: `event-${uuid()}`,
                offsetTicks: Number.MAX_SAFE_INTEGER - 2,
                durationTicks: 1,
                pitches: [{ step: "B", alter: 0, octave: 4 }],
                intensity: { dynamic: "mf" },
              },
            ],
          },
        ],
      },
    ],
  };
}

describe("compileScoreToTicks", () => {
    const score = 
    it.each()
})
