import { describe, expect, it } from "vitest";
import { DEFAULT_PPQ } from "../../src/model/score";
import { isValidPPQ } from "../../src/validation/isValidPPQ";

describe("isValidPPQ", () => {
  it.each([1, DEFAULT_PPQ, 481, 0x7fff])("accepts PPQ %s", (ppq) => {
    expect(isValidPPQ(ppq)).toBe(true);
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
    expect(isValidPPQ(ppq)).toBe(false);
  });
});
