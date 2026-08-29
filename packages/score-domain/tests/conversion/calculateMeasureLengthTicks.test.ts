import { describe, expect, it } from "vitest";
import { calculateMeasureLengthTicks } from "../../src/conversion/calculateMeasureLengthTicks";
import { DEFAULT_PPQ } from "../../src/model/score";

describe("calculateMeasureLengthTicks", () => {
  it.each([
    [{ numerator: 4, denominator: 4 } as const, DEFAULT_PPQ, 1920],
    [{ numerator: 3, denominator: 4 } as const, DEFAULT_PPQ, 1440],
    [{ numerator: 6, denominator: 8 } as const, DEFAULT_PPQ, 1440],
  ])("calculates %o at PPQ %i as %i ticks", (timeSignature, ppq, expected) => {
    expect(calculateMeasureLengthTicks(timeSignature, ppq)).toBe(expected);
  });
});
