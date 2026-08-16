import { describe, expect, it } from "vitest";
import {
  isNonNegativeInteger,
  isPositiveInteger,
} from "../src/validation/numberPredicates";

describe("number predicates", () => {
  describe("isNonNegativeInteger", () => {
    it.each([0, 1, 42, Number.MAX_SAFE_INTEGER])(
      "returns true for the safe non-negative integer %s",
      (value) => {
        expect(isNonNegativeInteger(value)).toBe(true);
      },
    );

    it.each([
      -1,
      -42,
      0.5,
      Number.NaN,
      Number.POSITIVE_INFINITY,
      Number.MAX_SAFE_INTEGER + 1,
    ])("returns false for the invalid value %s", (value) => {
      expect(isNonNegativeInteger(value)).toBe(false);
    });
  });

  describe("isPositiveInteger", () => {
    it.each([1, 2, 42, Number.MAX_SAFE_INTEGER])(
      "returns true for the safe positive integer %s",
      (value) => {
        expect(isPositiveInteger(value)).toBe(true);
      },
    );

    it.each([
      0,
      -1,
      -42,
      0.5,
      Number.NaN,
      Number.POSITIVE_INFINITY,
      Number.MAX_SAFE_INTEGER + 1,
    ])("returns false for the invalid value %s", (value) => {
      expect(isPositiveInteger(value)).toBe(false);
    });
  });
});
