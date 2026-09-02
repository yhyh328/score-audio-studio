import { describe, expect, it } from "vitest";
import type { ScoreDocument } from "@score-audio-studio/score-domain";
import { compileScoreToTicks } from "../../src/compiler/compileScoreToTicks";
import type { TickPlaybackEvent } from "../../src/model/playbackEvent";

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

function eventSummaries(
  events: readonly TickPlaybackEvent[],
  parts: ScoreDocument["parts"],
) {
  const partNameById = new Map(
    parts.map(({ id, name }) => [id, name]),
  );

  return events.map((event) => ({
    type: event.type,
    tick: event.tick,
    partName: partNameById.get(event.partId),
    midiNote: event.midiNote,
    velocity: event.velocity,
  }));
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
      score.parts[0]?.measures.push({
        id: `measure-${crypto.randomUUID()}`,
        number: 2,
        timeSignature: { numerator: 4, denominator: 4 },
        events: [
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 500,
            durationTicks: 1000,
            pitches: [{ step: "D", alter: 0, octave: 4 }],
            intensity: { dynamic: "mf" },
          },
        ],
      });
      const result = compileScoreToTicks(score);
      if (!result.success) {
        throw new Error(JSON.stringify(result.validation, null, 2))
      }
      expect(eventSummaries(result.events, score.parts)).toEqual([
        { type: "noteOn", tick: 0, partName: "Piano", midiNote: 60, velocity: 72 },
        { type: "noteOff", tick: 1000, partName: "Piano", midiNote: 60, velocity: 0 },
        { type: "noteOn", tick: 4500, partName: "Piano", midiNote: 62, velocity: 72 },
        { type: "noteOff", tick: 5500, partName: "Piano", midiNote: 62, velocity: 0 },
      ]);
      expect(result.scoreEndTick).toBe(8000);
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
      expect(eventSummaries(result.events, score.parts)).toEqual([
        { type: "noteOn", tick: 0, partName: "Cello", midiNote: 48, velocity: 104 },
        { type: "noteOn", tick: 0, partName: "Viola", midiNote: 60, velocity: 44 },
        { type: "noteOn", tick: 0, partName: "Piano", midiNote: 60, velocity: 72 },
        { type: "noteOn", tick: 0, partName: "Piano", midiNote: 64, velocity: 72 },
        { type: "noteOn", tick: 0, partName: "Piano", midiNote: 67, velocity: 72 },
        { type: "noteOn", tick: 0, partName: "1st Violin", midiNote: 76, velocity: 88 },
        { type: "noteOff", tick: 1000, partName: "Cello", midiNote: 48, velocity: 0 },
        { type: "noteOff", tick: 1000, partName: "1st Violin", midiNote: 76, velocity: 0 },
        { type: "noteOn", tick: 1000, partName: "2nd Violin", midiNote: 71, velocity: 58 },
        { type: "noteOff", tick: 2000, partName: "Viola", midiNote: 60, velocity: 0 },
        { type: "noteOff", tick: 2000, partName: "Piano", midiNote: 60, velocity: 0 },
        { type: "noteOff", tick: 2000, partName: "Piano", midiNote: 64, velocity: 0 },
        { type: "noteOff", tick: 2000, partName: "Piano", midiNote: 67, velocity: 0 },
        { type: "noteOff", tick: 2000, partName: "2nd Violin", midiNote: 71, velocity: 0 },
        { type: "noteOn", tick: 2000, partName: "Cello", midiNote: 43, velocity: 58 },
        { type: "noteOn", tick: 2000, partName: "Viola", midiNote: 62, velocity: 72 },
        { type: "noteOn", tick: 2000, partName: "Viola", midiNote: 69, velocity: 72 },
        { type: "noteOn", tick: 2000, partName: "2nd Violin", midiNote: 74, velocity: 88 },
        { type: "noteOn", tick: 2000, partName: "1st Violin", midiNote: 79, velocity: 72 },
        { type: "noteOn", tick: 3000, partName: "Piano", midiNote: 62, velocity: 88 },
        { type: "noteOn", tick: 3000, partName: "Piano", midiNote: 66, velocity: 88 },
        { type: "noteOff", tick: 4000, partName: "Cello", midiNote: 43, velocity: 0 },
        { type: "noteOff", tick: 4000, partName: "Viola", midiNote: 62, velocity: 0 },
        { type: "noteOff", tick: 4000, partName: "Piano", midiNote: 62, velocity: 0 },
        { type: "noteOff", tick: 4000, partName: "Piano", midiNote: 66, velocity: 0 },
        { type: "noteOff", tick: 4000, partName: "Viola", midiNote: 69, velocity: 0 },
        { type: "noteOff", tick: 4000, partName: "2nd Violin", midiNote: 74, velocity: 0 },
        { type: "noteOff", tick: 4000, partName: "1st Violin", midiNote: 79, velocity: 0 },
        { type: "noteOn", tick: 4000, partName: "Cello", midiNote: 50, velocity: 58 },
        { type: "noteOn", tick: 4000, partName: "Viola", midiNote: 60, velocity: 44 },
        { type: "noteOn", tick: 4000, partName: "Piano", midiNote: 62, velocity: 72 },
        { type: "noteOn", tick: 4000, partName: "Piano", midiNote: 66, velocity: 72 },
        { type: "noteOn", tick: 4000, partName: "Piano", midiNote: 69, velocity: 72 },
        { type: "noteOn", tick: 4000, partName: "2nd Violin", midiNote: 76, velocity: 58 },
        { type: "noteOn", tick: 4000, partName: "1st Violin", midiNote: 81, velocity: 88 },
        { type: "noteOff", tick: 5000, partName: "Viola", midiNote: 60, velocity: 0 },
        { type: "noteOff", tick: 5000, partName: "1st Violin", midiNote: 81, velocity: 0 },
        { type: "noteOn", tick: 5000, partName: "1st Violin", midiNote: 83, velocity: 72 },
        { type: "noteOff", tick: 6000, partName: "Piano", midiNote: 62, velocity: 0 },
        { type: "noteOff", tick: 6000, partName: "Piano", midiNote: 66, velocity: 0 },
        { type: "noteOff", tick: 6000, partName: "Piano", midiNote: 69, velocity: 0 },
        { type: "noteOff", tick: 6000, partName: "2nd Violin", midiNote: 76, velocity: 0 },
        { type: "noteOff", tick: 6000, partName: "1st Violin", midiNote: 83, velocity: 0 },
        { type: "noteOn", tick: 6000, partName: "Viola", midiNote: 62, velocity: 72 },
        { type: "noteOn", tick: 6000, partName: "2nd Violin", midiNote: 78, velocity: 88 },
        { type: "noteOn", tick: 6000, partName: "1st Violin", midiNote: 81, velocity: 72 },
        { type: "noteOff", tick: 7000, partName: "Cello", midiNote: 50, velocity: 0 },
        { type: "noteOff", tick: 7000, partName: "Viola", midiNote: 62, velocity: 0 },
        { type: "noteOff", tick: 7000, partName: "2nd Violin", midiNote: 78, velocity: 0 },
        { type: "noteOff", tick: 7000, partName: "1st Violin", midiNote: 81, velocity: 0 },
      ]);
      expect(result.scoreEndTick).toBe(7000);
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
      }, {
        id: `measure-${crypto.randomUUID()}`,
        number: 2,
        timeSignature: { numerator: 3, denominator: 4 },
        events: [
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 0,
            durationTicks: 1000,
            pitches: [
              { step: "A", alter: 0, octave: 5 },
            ],
            intensity: { dynamic: "f" },
          },
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 1000,
            durationTicks: 1000,
            pitches: [
              { step: "B", alter: 0, octave: 5 },
            ],
            intensity: { dynamic: "mf" },
          },
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 2000,
            durationTicks: 1000,
            pitches: [
              { step: "A", alter: 0, octave: 5 },
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
      }, {
        id: `measure-${crypto.randomUUID()}`,
        number: 2,
        timeSignature: { numerator: 3, denominator: 4 },
        events: [
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 0,
            durationTicks: 2000,
            pitches: [
              { step: "E", alter: 0, octave: 5 },
            ],
            intensity: { dynamic: "mp" },
          },
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 2000,
            durationTicks: 1000,
            pitches: [
              { step: "F", alter: 1, octave: 5 },
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
      }, {
        id: `measure-${crypto.randomUUID()}`,
        number: 2,
        timeSignature: { numerator: 3, denominator: 4 },
        events: [
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 0,
            durationTicks: 1000,
            pitches: [
              { step: "C", alter: 0, octave: 4 },
            ],
            intensity: { dynamic: "p" },
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
            durationTicks: 1000,
            pitches: [
              { step: "D", alter: 0, octave: 4 },
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
      }, {
        id: `measure-${crypto.randomUUID()}`,
        number: 2,
        timeSignature: { numerator: 3, denominator: 4 },
        events: [
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 0,
            durationTicks: 3000,
            pitches: [
              { step: "D", alter: 0, octave: 3 },
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
      }, {
        id: `measure-${crypto.randomUUID()}`,
        number: 2,
        timeSignature: { numerator: 3, denominator: 4 },
        events: [
          {
            type: "note",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 0,
            durationTicks: 2000,
            pitches: [
              { step: "D", alter: 0, octave: 4 },
              { step: "F", alter: 1, octave: 4 },
              { step: "A", alter: 0, octave: 4 },
            ],
            intensity: { dynamic: "mf" },
          },
          {
            type: "rest",
            id: `event-${crypto.randomUUID()}`,
            offsetTicks: 2000,
            durationTicks: 1000,
          },
        ],
      }],
    },
  ];
}
