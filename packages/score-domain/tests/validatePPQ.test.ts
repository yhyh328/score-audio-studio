import { describe, expect, it } from "vitest";
import { DEFAULT_PPQ, validatePPQ } from "../src/validation/validatePPQ";

describe("validatePPQ", () => {
  it.each([1, DEFAULT_PPQ, 481, 0x7fff])("accepts PPQ %s", (ppq) => {
    expect(validatePPQ(ppq)).toBe(true);
  });

  it.each([
    0,
    -1,
    0.5,
    0x8000,
    Number.NaN,
    Number.POSITIVE_INFINITY,
    Number.MAX_SAFE_INTEGER + 1,
  ])("rejects PPQ %s", (ppq) => {
    expect(validatePPQ(ppq)).toBe(false);
  });
});
