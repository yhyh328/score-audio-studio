import type { TempoEvent } from "../model/tempo";
import { isNonNegativeInteger } from "./numberPredicates";
import type { ValidationIssue } from "./validationTypes";
/**
 * These are pragmatic bounds for rejecting implausible input, not limits imposed by music theory or the score format. 
 * They cover ordinary musical tempos while leaving room for unusually slow or fast passages.
 */
const MIN_BPM: number = 20;
const MAX_BPM: number = 300;

export function validateTempoEvent(
    tempo: TempoEvent, 
    path: string): ValidationIssue[] {

  const issues: ValidationIssue[] = [];

  const tick = tempo.tick;
  if (!isNonNegativeInteger(tick)) {
    issues.push({
      target: "tempo",
      severity: "error",
      code: "INVALID_TEMPO_TICK",
      message: `Tempo tick must be a non-negative integer: ${tick}`,
      path: `${path}.tick`,
    });
  }

  const bpm = tempo.bpm;
  if (!Number.isInteger(bpm) || bpm < MIN_BPM || MAX_BPM < bpm) {
    issues.push({
      target: "tempo",
      severity: "error",
      code: "INVALID_BPM",
      message: `Tempo BPM must be an integer between 20 and 300: ${bpm}`,
      path: `${path}.bpm`,
    });
  }

  return issues;
}
