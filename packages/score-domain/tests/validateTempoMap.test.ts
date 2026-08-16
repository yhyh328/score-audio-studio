import { describe, expect, it } from "vitest";
import type { TempoEvent } from "../src/model/tempo";
import { DEFAULT_PPQ } from "../src/validation/validatePPQ";
import { validateTempoEvent } from "../src/validation/validateTempoEvent";
import { validateTempoMap } from "../src/validation/validateTempoMap";

const PATH = "tempoMap";

describe("validateTempoEvent", () => {
  it.each([20, 120, 300])("accepts integer BPM %s", (bpm) => {
    expect(validateTempoEvent({ tick: 0, bpm }, `${PATH}[0]`)).toEqual([]);
  });

  it.each([19, 20.5, 301, Number.NaN, Number.POSITIVE_INFINITY])(
    "rejects BPM %s",
    (bpm) => {
      expect(validateTempoEvent({ tick: 0, bpm }, `${PATH}[0]`)).toContainEqual(
        expect.objectContaining({
          code: "INVALID_BPM",
          path: `${PATH}[0].bpm`,
        }),
      );
    },
  );

  it.each([-1, 0.5, Number.MAX_SAFE_INTEGER + 1])(
    "rejects tempo tick %s",
    (tick) => {
      expect(validateTempoEvent({ tick, bpm: 120 }, `${PATH}[0]`)).toContainEqual(
        expect.objectContaining({
          code: "INVALID_TEMPO_TICK",
          path: `${PATH}[0].tick`,
        }),
      );
    },
  );
});

describe("validateTempoMap", () => {
  it("accepts a non-empty, strictly ascending tempo map starting at zero", () => {
    expect(validateTempoMap([
      { tick: 0, bpm: 120 },
      { tick: DEFAULT_PPQ, bpm: 100 },
      { tick: 960, bpm: 140 },
    ], PATH)).toEqual([]);
  });

  it("rejects an empty tempo map", () => {
    expect(validateTempoMap([], PATH)).toContainEqual(
      expect.objectContaining({ code: "EMPTY_TEMPO_MAP", path: PATH }),
    );
  });

  it("requires the first tempo event to start at tick zero", () => {
    expect(validateTempoMap([{ tick: 1, bpm: 120 }], PATH)).toContainEqual(
      expect.objectContaining({
        code: "TEMPO_MAP_MUST_START_AT_ZERO",
        path: `${PATH}[0].tick`,
      }),
    );
  });

  it("rejects a tempo map that is not sorted", () => {
    expect(validateTempoMap([
      { tick: 0, bpm: 120 },
      { tick: 960, bpm: 100 },
      { tick: DEFAULT_PPQ, bpm: 140 },
    ], PATH)).toContainEqual(
      expect.objectContaining({
        code: "UNSORTED_TEMPO_MAP",
        path: `${PATH}[2].tick`,
      }),
    );
  });

  it("detects duplicate ticks even when they are not adjacent", () => {
    const tempoMap: TempoEvent[] = [
      { tick: 0, bpm: 120 },
      { tick: DEFAULT_PPQ, bpm: 100 },
      { tick: 960, bpm: 140 },
      { tick: DEFAULT_PPQ, bpm: 80 },
    ];

    expect(validateTempoMap(tempoMap, PATH)).toContainEqual(
      expect.objectContaining({
        code: "DUPLICATED_TEMPO_TICK",
        path: `${PATH}[3].tick`,
      }),
    );
  });
});
