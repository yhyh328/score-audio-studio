import type { TempoEvent } from "../model/tempo";
import { isNonNegativeInteger } from "./numberPredicates";
import { validateTempoEvent } from "./validateTempoEvent";
import type { ValidationIssue } from "./validationTypes";

export function validateTempoMap(
  tempoMap: readonly TempoEvent[], path: string
): ValidationIssue[] {
  const issues: ValidationIssue[] = [];

  if (tempoMap.length === 0) {
    issues.push({
      target: "tempo",
      severity: "error",
      code: "EMPTY_TEMPO_MAP",
      message: "Tempo map must contain at least one event.",
      path,
    });
    return issues;
  }

  const firstTempo = tempoMap[0];
  if (firstTempo !== undefined && firstTempo.tick !== 0) {
    issues.push({
      target: "tempo",
      severity: "error",
      code: "TEMPO_MAP_MUST_START_AT_ZERO",
      message: "Tempo map must start at tick 0.",
      path: `${path}[0].tick`,
    });
  }

  const usedTicks = new Set<number>();
  let previousValidTick: number | undefined;
  for (const [index, tempo] of tempoMap.entries()) {
    issues.push(...validateTempoEvent(tempo, `${path}[${index}]`));
    const tick = tempo.tick;
    if (isNonNegativeInteger(tick)) {
      if (usedTicks.has(tick)) {
        issues.push({
          target: "tempo",
          severity: "error",
          code: "DUPLICATED_TEMPO_TICK",
          message: `Tempo map contains duplicate tick ${tick}.`,
          path: `${path}[${index}].tick`,
        });
      }

      if (previousValidTick !== undefined && tick < previousValidTick) {
        issues.push({
          target: "tempo",
          severity: "error",
          code: "UNSORTED_TEMPO_MAP",
          message: "Tempo map must be sorted by ascending tick.",
          path: `${path}[${index}].tick`,
        });
      }

      usedTicks.add(tick);
      previousValidTick = tick;
    }
  }

  return issues;
}
