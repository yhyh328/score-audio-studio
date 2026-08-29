import { describe, expect, it } from "vitest";
import type { Pitch } from "../../src/model/pitch";
import { validatePitches } from "../../src/validation/validatePitches";

const PATH = "parts[0].measures[0].events[0].pitches";

describe("validatePitches", () => {
  it("accepts pitches at both ends of the MIDI note-number range", () => {
    expect(validatePitches([
      { step: "C", alter: 0, octave: -1 },
      { step: "G", alter: 0, octave: 9 },
    ], PATH)).toEqual([]);
  });

  it("rejects a note with no pitch", () => {
    expect(validatePitches([], PATH)).toContainEqual(
      expect.objectContaining({
        code: "MISSING_PITCH",
        path: PATH,
      }),
    );
  });

  it("reports the exact index of each invalid pitch", () => {
    const invalidPitch = {
      step: "G",
      alter: 1,
      octave: 9,
    } as Pitch;

    expect(validatePitches([
      { step: "C", alter: 0, octave: 4 },
      invalidPitch,
    ], PATH)).toContainEqual(
      expect.objectContaining({
        code: "INVALID_PITCH",
        path: `${PATH}[1]`,
      }),
    );
  });
});
