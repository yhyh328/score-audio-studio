import type { TimeSignature } from "../model/timeSignature";

/**
 * Calculates a measure's length in ticks from a validated time signature and PPQ.
 */
export function calculateMeasureLengthTicks(
  timeSignature: TimeSignature,
  ppq: number,
): number {
  return timeSignature.numerator * (ppq * 4 / timeSignature.denominator);
}
