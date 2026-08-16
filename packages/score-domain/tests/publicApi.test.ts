import { describe, expect, it } from "vitest";
import {
  calculateMeasureLengthTicks,
  pitchToMidiNoteNumber,
  validateProjectDocument,
  validateScoreDocument,
} from "../src/index";

describe("score-domain public API", () => {
  it("exports the Phase 1 conversion and document-validation entry points", () => {
    expect(calculateMeasureLengthTicks).toBeTypeOf("function");
    expect(pitchToMidiNoteNumber).toBeTypeOf("function");
    expect(validateProjectDocument).toBeTypeOf("function");
    expect(validateScoreDocument).toBeTypeOf("function");
  });
});