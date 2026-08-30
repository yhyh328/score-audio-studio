import { describe, expect, it } from "vitest";
import type { ScoreDocument } from "@score-audio-studio/score-domain";
import { compileScoreToTicks } from "../../src/compiler/compileScoreToTicks";

function createScore(): ScoreDocument {
  return {
    schemaVersion: 1,
    ppq: 1000,
    tempoMap: [
      { tick: 0, bpm: 120 },
      { tick: 2000, bpm: 90 },
    ],
    parts: [{
      id: `part-${crypto.randomUUID()}`,
      name: "Piano",
      measures: [{
        id: `measure-${crypto.randomUUID()}`,
        number: 1,
        timeSignature: { numerator: 4, denominator: 4 },
        events: [],
      }],
    }],
  };
}

describe("compileScoreToTicks", () => {

  it("preserves the measure duration when the score contains no events", 
    () => {
      const score = createScore();
      const result = compileScoreToTicks(score);
      expect(result).toEqual({
        success: true,
        events: [],
        scoreEndTick: 4000 // numerator * (ppq * 4 / denominator)
      });
    }
  );

  it("returns validation failure for an invalid score", () => {
      const score = createScore();
      score.tempoMap.push({ tick: -1, bpm: -1 }) // push an invalid tempmo map
      const result = compileScoreToTicks(score);
      expect(result).toEqual({
        success: false,
        validation: expect.arrayContaining([
          expect.objectContaining({
            target: "tempo",
            severity: "error",
            code: "INVALID_TEMPO_TICK",
            path: "tempoMap[2].tick"
          })
        ])
      });
  });

  it("compiles a valid score with a single note event " + 
     "into playback events and calculates its end tick", () => {
      const score = createScore();
      const measure = score.parts[0]?.measures[0];
      if (measure === undefined) {
        throw new Error("Test fixture measure is missing.");
      }
      measure.events.push({
        type: "note",
        id: `event-${crypto.randomUUID()}`,
        offsetTicks: 0,
        durationTicks: 1000,
        pitches: [{ step: "C", alter: 0, octave: 4 }],
        intensity: { dynamic: "mf" },
      });
      const result = compileScoreToTicks(score);
      if (!result.success) {
        throw new Error(JSON.stringify(result.validation, null, 2))
      }
      expect(result.events).toHaveLength(2);
      expect(result.scoreEndTick).toBe(4000);
    }
  );

  it("compiles a valid multi-part score with multiple events " + 
     "into playback events and calculates its end tick", () => {
      const score = createScore();
      const measure = score.parts[0]?.measures[0];
      if (measure === undefined) {
        throw new Error("Test fixture measure is missing.");
      }
      score["parts"] = createParts();
      const result = compileScoreToTicks(score);
      if (!result.success) {
        throw new Error(JSON.stringify(result.validation, null, 2))
      }
      /**
       * Each pitch generates one noteOn event and noteOff event.
       * Therefore, the number of playback events is twice the total number of pitches.
       * This fixture contains 14 pitches, so it produces 28 playback events.
       */
      expect(result.events).toHaveLength(28);
      expect(result.scoreEndTick).toBe(4000);
    }
  );

});

function createParts(): ScoreDocument["parts"] {
  return [
    {
      id: `part-${crypto.randomUUID()}`,
      name: "1st Violin",
      measures: [{
        id: `measure-${crypto.randomUUID()}`,
        number: 1,
        timeSignature: { numerator: 4, denominator: 4 },
        events: [
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 0,
            durationTicks: 1000,
            pitches: [
              { step: "E", alter: 0, octave: 5 },
            ],
            intensity: { dynamic: "f" },
          },
          {
            type: "rest",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 1000,
            durationTicks: 1000,
          },
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 2000,
            durationTicks: 2000,
            pitches: [
              { step: "G", alter: 0, octave: 5 },
            ],
            intensity: { dynamic: "mf" },
          },
        ],
      }],
    },

    {
      id: `part-${crypto.randomUUID()}`,
      name: "2nd Violin",
      measures: [{
        id: `measure-${crypto.randomUUID()}`,
        number: 1,
        timeSignature: { numerator: 4, denominator: 4 },
        events: [
          {
            type: "rest",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 0,
            durationTicks: 1000,
          },
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 1000,
            durationTicks: 1000,
            pitches: [
              { step: "B", alter: 0, octave: 4 },
            ],
            intensity: { dynamic: "mp" },
          },
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 2000,
            durationTicks: 2000,
            pitches: [
              { step: "D", alter: 0, octave: 5 },
            ],
            intensity: { dynamic: "f" },
          },
        ],
      }],
    },

    {
      id: `part-${crypto.randomUUID()}`,
      name: "Viola",
      measures: [{
        id: `measure-${crypto.randomUUID()}`,
        number: 1,
        timeSignature: { numerator: 4, denominator: 4 },
        events: [
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 0,
            durationTicks: 2000,
            pitches: [
              { step: "C", alter: 0, octave: 4 },
            ],
            intensity: { dynamic: "p" },
          },
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 2000,
            durationTicks: 2000,
            pitches: [
              { step: "D", alter: 0, octave: 4 },
              { step: "A", alter: 0, octave: 4 },
            ],
            intensity: { dynamic: "mf" },
          },
        ],
      }],
    },

    {
      id: `part-${crypto.randomUUID()}`,
      name: "Cello",
      measures: [{
        id: `measure-${crypto.randomUUID()}`,
        number: 1,
        timeSignature: { numerator: 4, denominator: 4 },
        events: [
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 0,
            durationTicks: 1000,
            pitches: [
              { step: "C", alter: 0, octave: 3 },
            ],
            intensity: { dynamic: "ff" },
          },
          {
            type: "rest",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 1000,
            durationTicks: 1000,
          },
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 2000,
            durationTicks: 2000,
            pitches: [
              { step: "G", alter: 0, octave: 2 },
            ],
            intensity: { dynamic: "mp" },
          },
        ],
      }],
    },

    {
      id: `part-${crypto.randomUUID()}`,
      name: "Piano",
      measures: [{
        id: `measure-${crypto.randomUUID()}`,
        number: 1,
        timeSignature: { numerator: 4, denominator: 4 },
        events: [
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 0,
            durationTicks: 2000,
            pitches: [
              { step: "C", alter: 0, octave: 4 },
              { step: "E", alter: 0, octave: 4 },
              { step: "G", alter: 0, octave: 4 },
            ],
            intensity: { dynamic: "mf" },
          },
          {
            type: "rest",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 2000,
            durationTicks: 1000,
          },
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 3000,
            durationTicks: 1000,
            pitches: [
              { step: "D", alter: 0, octave: 4 },
              { step: "F", alter: 1, octave: 4 },
            ],
            intensity: { dynamic: "f" },
          },
        ],
      }],
    },
  ];
}