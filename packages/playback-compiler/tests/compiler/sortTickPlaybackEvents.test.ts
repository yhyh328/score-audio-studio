import { describe, expect, it } from "vitest";
import type { TickPlaybackEvent } from "../../src/model/playbackEvent";
import { sortTickPlaybackEvents } from "../../src/compiler/sortTickPlaybackEvents";

function createEvent(
  tick: number,
  type: TickPlaybackEvent["type"],
  midiNote: number,
): TickPlaybackEvent {
  return {
    type,
    tick,
    partId: `part-${crypto.randomUUID()}`,
    noteId: `event-${crypto.randomUUID()}`,
    midiNote,
    velocity: type === "noteOff" ? 0 : 1,
  };
}

describe("sortTickPlaybackEvents", () => {

  it("sorts events with different ticks by ascending tick", () => {
    const input: TickPlaybackEvent[] = [
      createEvent(2000, "noteOn", 60),
      createEvent(0, "noteOn", 60),
      createEvent(1000, "noteOn", 60),
    ];
    const result = sortTickPlaybackEvents(input);
    expect(result.map((event) => event.tick)).toEqual([0, 1000, 2000]);
  });

  it("sorts noteOff before noteOn when ticks are equal", () => {
    const input: TickPlaybackEvent[] = [
      createEvent(1000, "noteOn", 43),
      createEvent(1000, "noteOff", 79),
    ];
    const result = sortTickPlaybackEvents(input);
    expect(result.map((event) => event.type)).toEqual([
      "noteOff",
      "noteOn",
    ]);
  });

  it("sorts equal-tick events of the same type by ascending MIDI note", () => {
    const input: TickPlaybackEvent[] = [
      createEvent(1000, "noteOn", 79),
      createEvent(1000, "noteOn", 43),
      createEvent(1000, "noteOn", 60),
    ];
    const result = sortTickPlaybackEvents(input);
    expect(result.map((event) => event.midiNote)).toEqual([43, 60, 79]);
  });

  it("returns an empty array for empty input", () => {
    expect(sortTickPlaybackEvents([])).toEqual([]);
  });
  
});
