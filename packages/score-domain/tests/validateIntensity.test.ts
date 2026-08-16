import { describe, expect, it } from "vitest";
import type { Dynamic } from "../src/model/intensity";
import { validateIntensity } from "../src/validation/validateIntensity";
import type { ValidationIssue } from "../src/validation/validationTypes";

const dynamics: Dynamic[] = [
  "ppp",
  "pp",
  "p",
  "mp",
  "mf",
  "f",
  "ff",
  "fff",
];

const path = "intensity";

const invalidVelocityMessage =
  "Velocity override must be an integer between 1 and 127; " +
  "MIDI Note On velocity 0 is interpreted as Note Off.";

describe("validateIntensity", () => {
  /**
   * Unsupported dynamic strings such as "pppp", "ffff", "sfz", and "fp"
   * are excluded by the Dynamic type at compile time.
   * Runtime validation of untrusted input, such as JSON, belongs to the
   * decoding layer and is outside the scope of this validator.
   */
  it.each(dynamics)(
    "accepts %s with no velocity override",
    (dynamic) => {
      const result: ValidationIssue[] = validateIntensity(
        { dynamic }, path
      );
      expect(result).toEqual([]);
    },
  );

  const validCases = dynamics.flatMap(
    (dynamic) =>
      [1, 64, 127].map(
        (velocity) =>
          [dynamic, velocity] as const,
      ),
  );
  it.each(validCases)(
    "accepts note dynamic %s with velocity %s",
    (dynamic, velocity) => {
      const result: ValidationIssue[] = validateIntensity(
        { dynamic, velocity }, path
      );
      expect(result).toEqual([]);
    },
  );

  const invalidCases = dynamics.flatMap(
    (dynamic) =>
      [ 0, -1, 0.5, 128, Number.NaN, Number.POSITIVE_INFINITY ].map(
        (velocity) => [
          dynamic, velocity
        ] as const,
      )
  );
  it.each(invalidCases)(
    "rejects note dynamic %s with velocity %s",
    (dynamic, velocity) => {
      const result: ValidationIssue[] = validateIntensity(
        { dynamic, velocity }, path
      );
      expect(result).toContainEqual(
        expect.objectContaining({
          code: "INVALID_VELOCITY",
          message: invalidVelocityMessage,
          path: `${path}.velocity`,
        })
      );
    },
  );
});
