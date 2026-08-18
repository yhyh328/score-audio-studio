import type { ScoreDocument } from "../model/score";
import { validateParts } from "./validateParts";
import { isValidPPQ } from "./isValidPPQ";
import { isValidSchemaVersion } from "./validateSchemaVersion";
import { validateTempoMap } from "./validateTempoMap";
import type { ValidationIssue, ValidationResult } from "./validationTypes";

export function validateScoreDocument(
  score: ScoreDocument,
  path = "",
): ValidationResult {
  
  const issues: ValidationIssue[] = [];

  const schemaVersion = score.schemaVersion;
  if (!isValidSchemaVersion("score", schemaVersion)) {
    issues.push({
      target: "schemaVersion",
      severity: "fatal",
      code: "INVALID_SCHEMA_VERSION",
      message: `Invalid schema version: ${schemaVersion}`,
      path: childPath(path, "schemaVersion"),
    });
    return { valid: false, issues };
  }

  const ppq = score.ppq;
  const hasValidPPQ = isValidPPQ(ppq);
  if (!hasValidPPQ) {
    issues.push({
      target: "ppq",
      severity: "error",
      code: "INVALID_PPQ",
      message: `Invalid pulses per quarter note: ${ppq}`,
      path: childPath(path, "ppq"),
    });
  }
  const tempoMap = score.tempoMap;
  issues.push(...validateTempoMap(tempoMap, childPath(path, "tempoMap")));
  const parts = score.parts;
  issues.push(...validateParts(
    parts,
    hasValidPPQ ? ppq : undefined,
    childPath(path, "parts"),
  ));
  
  return { valid: issues.length === 0, issues };
}

function childPath(parent: string, child: string): string {
  return parent === "" ? child : `${parent}.${child}`;
}
