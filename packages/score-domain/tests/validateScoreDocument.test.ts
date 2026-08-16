import { describe, expect, it } from "vitest";
import type { ScoreDocument } from "../src/model/score";
import { DEFAULT_PPQ } from "../src/validation/validatePPQ";
import { validateScoreDocument } from "../src/validation/validateScoreDocument";

function createScore(overrides: Partial<ScoreDocument> = {}): ScoreDocument {
  return {
    schemaVersion: 1,
    ppq: DEFAULT_PPQ,
    tempoMap: [{ tick: 0, bpm: 120 }],
    parts: [{
      id: crypto.randomUUID(),
      name: "Piano",
      measures: [{
        id: crypto.randomUUID(),
        number: 1,
        timeSignature: { numerator: 4, denominator: 4 },
        events: [{
          type: "note",
          id: crypto.randomUUID(),
          offsetTicks: 0,
          durationTicks: DEFAULT_PPQ,
          pitches: [{ step: "C", alter: 0, octave: 4 }],
          intensity: { dynamic: "mf" },
        }],
      }],
    }],
    ...overrides,
  };
}

describe("validateScoreDocument", () => {
  it("accepts a valid score document", () => {
    expect(validateScoreDocument(createScore())).toEqual({
      valid: true,
      issues: [],
    });
  });

  it("stops after a fatal unsupported score schema version", () => {
    const score = createScore({
      schemaVersion: 2,
      ppq: 0,
      tempoMap: [],
      parts: [],
    } as unknown as Partial<ScoreDocument>);

    expect(validateScoreDocument(score)).toEqual({
      valid: false,
      issues: [expect.objectContaining({
        severity: "fatal",
        code: "INVALID_SCHEMA_VERSION",
        path: "schemaVersion",
      })],
    });
  });

  it("continues PPQ-independent validation when PPQ is invalid", () => {
    const score = createScore({ ppq: 0 });
    const event = score.parts[0]?.measures[0]?.events[0];
    if (event === undefined) throw new Error("Test fixture event is missing.");
    event.offsetTicks = -1;
    event.durationTicks = 5000;

    const result = validateScoreDocument(score);

    expect(result.issues.map(({ code }) => code)).toEqual(expect.arrayContaining([
      "INVALID_PPQ",
      "INVALID_EVENT_OFFSET",
    ]));
    expect(result.issues.some(({ code }) => code === "EVENT_EXCEEDS_MEASURE")).toBe(false);
  });

  it("reports empty tempo and part collections independently", () => {
    const result = validateScoreDocument(createScore({ tempoMap: [], parts: [] }));

    expect(result.issues).toEqual(expect.arrayContaining([
      expect.objectContaining({ code: "EMPTY_TEMPO_MAP", path: "tempoMap" }),
      expect.objectContaining({ code: "EMPTY_PARTS", path: "parts" }),
    ]));
  });

  it("detects duplicate entity IDs across parts, measures, and events", () => {
    const score = createScore();
    const part = score.parts[0];
    if (part === undefined) throw new Error("Test fixture part is missing.");
    part.measures[0] = {
      id: part.id,
      number: 1,
      timeSignature: { numerator: 4, denominator: 4 },
      events: [{
        type: "rest",
        id: part.id,
        offsetTicks: 0,
        durationTicks: DEFAULT_PPQ,
      }],
    };

    const result = validateScoreDocument(score);

    expect(result.issues.filter(({ code }) => code === "DUPLICATED_ENTITY_ID"))
      .toHaveLength(2);
    expect(result.issues.map(({ path }) => path)).toEqual(expect.arrayContaining([
      "parts[0].measures[0].id",
      "parts[0].measures[0].events[0].id",
    ]));
  });

  it("allows part names and measure numbers to repeat in different parts", () => {
    const score = createScore();
    const firstPart = score.parts[0];
    if (firstPart === undefined) throw new Error("Test fixture part is missing.");
    score.parts.push({
      id: crypto.randomUUID(),
      name: firstPart.name,
      measures: [{
        id: crypto.randomUUID(),
        number: 1,
        timeSignature: { numerator: 4, denominator: 4 },
        events: [],
      }],
    });

    expect(validateScoreDocument(score)).toEqual({ valid: true, issues: [] });
  });
});
