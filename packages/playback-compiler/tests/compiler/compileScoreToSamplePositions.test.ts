import { describe, expect, it } from "vitest";
import {
    DEFAULT_PPQ 
} from "@score-audio-studio/score-domain";
import type {
    ScoreDocument
} from "@score-audio-studio/score-domain";
import type {
    SamplePlaybackEvent
} from "../../src/model/playbackEvent";
import {
    compileScoreToSamplePositions
} from "../../src/compiler/compileScoreToSamplePositions";

function createScore(
    overrides: Partial<ScoreDocument> = {}
): ScoreDocument {
  return {
    schemaVersion: 1,
    ppq: DEFAULT_PPQ,
    tempoMap: [{ tick: 0, bpm: 120 }],
    parts: [{
      id: `part-${crypto.randomUUID()}`,
      name: "Piano",
      measures: [{
        id: `measure-${crypto.randomUUID()}`,
        number: 1,
        timeSignature: { numerator: 4, denominator: 4 },
        events: [{
          type: "note",
          id: `event-${crypto.randomUUID()}`,
          offsetTicks: 0,
          durationTicks: 480,
          pitches: [{ step: "C", alter: 0, octave: 4 }],
          intensity: { dynamic: "mf" },
        }],
      }],
    }],
    ...overrides,
  };
}

function eventSummaries(
  events: readonly SamplePlaybackEvent[],
) {
  return events.map((event) => ({
    type: event.type,
    samplePosition: event.samplePosition,
    midiNote: event.midiNote,
    velocity: event.velocity,
  }));
}

describe("compileTicksToSamplePositions", () => {
    const SAMPLE_RATE: number = 48_000;
    it("does not compile sample playback events" +
       "if the score invalid.", () => {
        const score = createScore({
            schemaVersion: 2,
            ppq: 0,
            tempoMap: [],
            parts: [],
        } as unknown as Partial<ScoreDocument>);
        expect(
            () => compileScoreToSamplePositions(
                score,
                SAMPLE_RATE
            ),
        ).toThrow("Score compilation failed");
    });
    it("compiles sample playback events", () => {
        const score = createScore();
        const result =
            compileScoreToSamplePositions(
                score,
                SAMPLE_RATE
            );
        expect(eventSummaries(result)).toEqual([{
            type: "noteOn",
            midiNote: 60,
            velocity: 72,
            samplePosition: 0
        }, {
            type: "noteOff",
            midiNote: 60,
            velocity: 0,
            samplePosition: 24000
        }])
    });
});
