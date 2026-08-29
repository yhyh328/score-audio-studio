import { describe, expect, it } from "vitest";
import type { EntityId } from "../../src/model/entityId";
import type { Measure } from "../../src/model/measure";
import type { TimeSignature } from "../../src/model/timeSignature";
import { validateMeasures } from "../../src/validation/validateMeasures";
import { DEFAULT_PPQ } from "../../src/model/score";

const PPQ = DEFAULT_PPQ;
const PATH = "measures";

function createMeasure(overrides: Partial<Measure> = {}): Measure {
  return {
    id: `measure-${crypto.randomUUID()}`,
    number: 1,
    timeSignature: { numerator: 4, denominator: 4 },
    events: [],
    ...overrides,
  };
}

function validate(measures: readonly Measure[]) {
  return validateMeasures(measures, PPQ, PATH, new Set<EntityId>());
}

describe("validateMeasures", () => {
  it("accepts unique measures sorted by ascending number", () => {
    const result = validate([
      createMeasure(),
      createMeasure({ 
        id: `measure-${crypto.randomUUID()}`, 
        number: 2 
      }),
    ]);

    expect(result).toEqual([]);
  });

  it("rejects an empty measure array", () => {
    expect(validate([])).toContainEqual(
      expect.objectContaining({
        code: "EMPTY_MEASURES",
        path: PATH,
      }),
    );
  });

  it("rejects a duplicate entity ID", () => {
    const duplicatedId = `measure-${crypto.randomUUID()}`;
    const result = validate([
      createMeasure({ id: duplicatedId }),
      createMeasure({ id: duplicatedId, number: 2 }),
    ]);

    expect(result).toContainEqual(
      expect.objectContaining({
        code: "DUPLICATE_ENTITY_ID",
        message: `Entity ID must be unique: ${duplicatedId}`,
        path: `${PATH}[1].id`,
      }),
    );
  });

  it.each([
    0,
    -1,
    0.5,
    Number.NaN,
    Number.POSITIVE_INFINITY,
    Number.MAX_SAFE_INTEGER + 1,
  ])("rejects the invalid measure number %s", (number) => {
    const result = validate([createMeasure({ number })]);

    expect(result).toContainEqual(
      expect.objectContaining({
        code: "INVALID_MEASURE_NUMBER",
        path: `${PATH}[0].number`,
      }),
    );
  });

  it("rejects a duplicate measure number", () => {
    const result = validate([
      createMeasure(),
      createMeasure({ 
        id: `measure-${crypto.randomUUID()}` 
      }),
    ]);

    expect(result).toContainEqual(
      expect.objectContaining({
        code: "DUPLICATE_MEASURE_NUMBER",
        message: "Measure number must be unique: 1",
        path: `${PATH}[1].number`,
      }),
    );
  });

  it("rejects measures that are not sorted by ascending number", () => {
    const result = validate([
      createMeasure({ number: 2 }),
      createMeasure({ 
        id: `measure-${crypto.randomUUID()}`, 
        number: 1 
      }),
    ]);

    expect(result).toContainEqual(
      expect.objectContaining({
        code: "UNSORTED_MEASURES",
        path: `${PATH}[1].number`,
      }),
    );
  });

  it("reports event issues even when the time signature is invalid", () => {
    const invalidTimeSignature = {
      numerator: 4,
      denominator: 3,
    } as unknown as TimeSignature;
    const result = validate([
      createMeasure({
        timeSignature: invalidTimeSignature,
        events: [{
          type: "rest",
          id: `event-${crypto.randomUUID()}`,
          offsetTicks: -1,
          durationTicks: 0,
        }],
      }),
    ]);

    expect(result.map(({ code }) => code)).toEqual(expect.arrayContaining([
      "INVALID_TIME_SIGNATURE",
      "INVALID_EVENT_OFFSET",
      "INVALID_EVENT_DURATION",
    ]));
    expect(result.some(({ code }) => code === "EVENT_EXCEEDS_MEASURE")).toBe(false);
  });
});
