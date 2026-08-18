import { describe, expect, it } from "vitest";
import type { ProjectDocument } from "../src/model/project";
import { DEFAULT_PPQ } from "../src/validation/isValidPPQ";
import { validateProjectDocument } from "../src/validation/validateProjectDocument";

function createProject(): ProjectDocument {
  return {
    schemaVersion: 1,
    title: "Validation fixture",
    score: {
      schemaVersion: 1,
      ppq: DEFAULT_PPQ,
      tempoMap: [{ tick: 0, bpm: 120 }],
      parts: [{
        id: `part-${crypto.randomUUID()}`,
        name: "Piano",
        measures: [{
          id: `measure-${crypto.randomUUID()}`,
          number: 1,
          timeSignature: { numerator: 4, denominator: 4 },
          events: [],
        }],
      }],
    },
  };
}

describe("validateProjectDocument", () => {
  it("accepts a JSON-round-tripped project document", () => {
    const project = JSON.parse(JSON.stringify(createProject())) as ProjectDocument;
    expect(validateProjectDocument(project)).toEqual({ valid: true, issues: [] });
  });

  it("stops after a fatal unsupported project schema version", () => {
    const project = createProject();
    project.schemaVersion = 2 as 1;
    project.score.ppq = 0;

    expect(validateProjectDocument(project)).toEqual({
      valid: false,
      issues: [expect.objectContaining({
        severity: "fatal",
        code: "INVALID_SCHEMA_VERSION",
        path: "schemaVersion",
      })],
    });
  });

  it("prefixes nested score issue paths", () => {
    const project = createProject();
    project.score.ppq = 0;

    expect(validateProjectDocument(project).issues).toContainEqual(
      expect.objectContaining({
        code: "INVALID_PPQ",
        path: "score.ppq",
      }),
    );
  });
});
