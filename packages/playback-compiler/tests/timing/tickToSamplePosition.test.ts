import { describe, expect, it } from "vitest";
import type { TickPlaybackEvent } from "../../src/model/playbackEvent";
import { buildTempoSegments } from "../../src/timing/buildTempoSegments";
import { tickToSamplePosition } from "../../src/timing/tickToSamplePosition";

function createNoteEvent(tick: number): TickPlaybackEvent {
  return {
    type: "noteOn",
    tick,
    partId: `part-${crypto.randomUUID()}`,
    noteId: `event-${crypto.randomUUID()}`,
    midiNote: 0,
    velocity: 1,
  };
}

describe("tickToSamplePosition", () => {
  it("converts tick to sample position", () => {
    const ppq = 480;
    const segments = buildTempoSegments(ppq, [
      { tick: 0, bpm: 120 },
    ]);

    expect(
      tickToSamplePosition(
        createNoteEvent(480),
        48000,
        ppq,
        segments,
        480,
      ),
    ).toBe(24000);
  });

  it.each([
    0.5,
    0,
    -1,
    -0.5,
    Number.NaN,
    Number.POSITIVE_INFINITY,
    Number.MAX_SAFE_INTEGER + 1,
  ])("throws for invalid sampleRate: %s", (sampleRate) => {
    const ppq = 1;
    const segments = buildTempoSegments(ppq, [
      { tick: 0, bpm: 120 },
    ]);

    expect(() =>
      tickToSamplePosition(
        createNoteEvent(0),
        sampleRate,
        ppq,
        segments,
        1,
      ),
    ).toThrow("Sample rate must be a positive safe integer");
  });

  it("accepts the largest safe sample position", () => {
    const ppq = 1;
    const segments = buildTempoSegments(ppq, [
      { tick: 0, bpm: 120 },
    ]);

    expect(
      tickToSamplePosition(
        createNoteEvent(2), // 1 second
        Number.MAX_SAFE_INTEGER,
        ppq,
        segments,
        3,
      ),
    ).toBe(Number.MAX_SAFE_INTEGER);
  });

  it("throws when sample position exceeds the safe integer range", () => {
    const ppq = 1;
    const segments = buildTempoSegments(ppq, [
      { tick: 0, bpm: 120 },
    ]);

    expect(() =>
      tickToSamplePosition(
        createNoteEvent(3), // 1.5 seconds
        Number.MAX_SAFE_INTEGER,
        ppq,
        segments,
        3,
      ),
    ).toThrow("Sample position exceeds safe positive integer range");
  });
});
