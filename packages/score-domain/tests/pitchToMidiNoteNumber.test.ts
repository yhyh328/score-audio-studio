import { describe, expect, it } from "vitest";
import type { Pitch } from "../src/model/pitch";
import { pitchToMidiNoteNumber } from "../src/conversion/pitchToMidiNoteNumber";

describe("pitchToMidiNoteNumber", () => {
  it.each([
    [{ step: "C", alter: 0, octave: -1 }, 0],
    [{ step: "C", alter: 0, octave: 4 }, 60],
    [{ step: "F", alter: 1, octave: 4 }, 66],
    [{ step: "G", alter: 0, octave: 9 }, 127],
  ] as const)("converts %o to MIDI note number %i", (pitch, expected) => {
    expect(pitchToMidiNoteNumber(pitch)).toBe(expected);
  });

  it.each([
    { step: "C", alter: -1, octave: -1 },
    { step: "G", alter: 1, octave: 9 },
    { step: "C", alter: 0, octave: 4.5 },
    { step: "H", alter: 0, octave: 4 },
    { step: "C", alter: 3, octave: 4 },
  ] as unknown as Pitch[])("rejects the invalid pitch %o", (pitch) => {
    expect(() => pitchToMidiNoteNumber(pitch)).toThrow(RangeError);
  });
});
