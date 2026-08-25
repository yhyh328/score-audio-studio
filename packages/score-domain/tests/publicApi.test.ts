import { describe, expect, expectTypeOf, it } from "vitest";

import {
  calculateMeasureLengthTicks,
  pitchToMidiNoteNumber,
  validateProjectDocument,
  validateScoreDocument,
} from "../src/index";

import type {
  EntityId,
  ProjectDocument,
  ScoreDocument,
  ValidationIssue,
  ValidationResult,
} from "../src/index";

describe("score-domain public API", () => {
  it("exports public runtime functions", () => {
    expect(calculateMeasureLengthTicks).toBeTypeOf("function");
    expect(pitchToMidiNoteNumber).toBeTypeOf("function");
    expect(validateProjectDocument).toBeTypeOf("function");
    expect(validateScoreDocument).toBeTypeOf("function");
  });

  it("exports public domain types", () => {
    expectTypeOf<EntityId>().toEqualTypeOf<string>();
    expectTypeOf<ProjectDocument>().toBeObject();
    expectTypeOf<ScoreDocument>().toBeObject();
  });

  it("exports public validation types", () => {
    expectTypeOf<ValidationIssue>().toBeObject();
    expectTypeOf<ValidationResult>().toBeObject();
  });
});