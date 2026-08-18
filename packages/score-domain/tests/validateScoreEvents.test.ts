import { describe, expect, it } from "vitest";
import type { EntityId } from "../src/model/entityId";
import type { ScoreEvent } from "../src/model/scoreEvent";
import { DEFAULT_PPQ } from "../src/validation/isValidPPQ";
import { validateScoreEvents } from "../src/validation/validateScoreEvents";

const PATH = "parts[0].measures[0].events";
const MEASURE_LENGTH_TICKS = DEFAULT_PPQ * 4;

function validate(
  events: readonly ScoreEvent[],
  measureLengthTicks: number | undefined = MEASURE_LENGTH_TICKS,
  usedEntityIds = new Set<EntityId>(),
) {
  return validateScoreEvents(
    events, 
    measureLengthTicks, 
    PATH, 
    usedEntityIds
  );
}

describe("validateScoreEvents", () => {
  it("accepts an empty event array and a valid rest", () => {
    expect(validate([])).toEqual([]);
    expect(validate([{
      type: "rest",
      id: `event-${crypto.randomUUID()}`,
      offsetTicks: 0,
      durationTicks: MEASURE_LENGTH_TICKS,
    }])).toEqual([]);
  });

  it("rejects duplicate entity IDs", () => {
    const duplicatedId = `event-${crypto.randomUUID()}`;
    const usedEntityIds = new Set<EntityId>([duplicatedId]);
    const result = validate([{
      type: "rest",
      id: duplicatedId,
      offsetTicks: 0,
      durationTicks: DEFAULT_PPQ,
    }], MEASURE_LENGTH_TICKS, usedEntityIds);

    expect(result).toContainEqual(expect.objectContaining({
      code: "DUPLICATED_ENTITY_ID",
      path: `${PATH}[0].id`,
    }));
  });

  it.each([
    ["offsetTicks", -1, "INVALID_EVENT_OFFSET"],
    ["offsetTicks", 0.5, "INVALID_EVENT_OFFSET"],
    ["durationTicks", 0, "INVALID_EVENT_DURATION"],
    ["durationTicks", Number.MAX_SAFE_INTEGER + 1, "INVALID_EVENT_DURATION"],
  ] as const)("rejects invalid %s value %s", (field, value, code) => {
    const event: ScoreEvent = {
      type: "rest",
      id: `event-${crypto.randomUUID()}`,
      offsetTicks: 0,
      durationTicks: DEFAULT_PPQ,
      [field]: value,
    };

    expect(validate([event])).toContainEqual(expect.objectContaining({
      code,
      path: `${PATH}[0].${field}`,
    }));
  });

  it("rejects an event that extends beyond its measure", () => {
    const result = validate([{
      type: "rest",
      id: `event-${crypto.randomUUID()}`,
      offsetTicks: 1800,
      durationTicks: 121,
    }]);

    expect(result).toContainEqual(expect.objectContaining({
      code: "EVENT_EXCEEDS_MEASURE",
      path: `${PATH}[0].durationTicks`,
    }));
  });

  it("skips only the measure-boundary check when the length is unavailable", () => {
    const result = validate([{
      type: "rest",
      id: `event-${crypto.randomUUID()}`,
      offsetTicks: -1,
      durationTicks: 5000,
    }], undefined);

    expect(result).toContainEqual(expect.objectContaining({
      code: "INVALID_EVENT_OFFSET",
    }));
    expect(result.some(({ code }) => code === "EVENT_EXCEEDS_MEASURE")).toBe(false);
  });

  it("delegates note pitch and velocity validation", () => {
    const result = validate([{
      type: "note",
      id: `event-${crypto.randomUUID()}`,
      offsetTicks: 0,
      durationTicks: DEFAULT_PPQ,
      pitches: [],
      intensity: { dynamic: "mf", velocity: 0 },
    }]);

    expect(result).toContainEqual(expect.objectContaining({
      code: "MISSING_PITCH",
      path: `${PATH}[0].pitches`,
    }));

    expect(result).toContainEqual(expect.objectContaining({
      code: "INVALID_VELOCITY",
      path: `${PATH}[0].intensity.velocity`,
    }));

  });
});
