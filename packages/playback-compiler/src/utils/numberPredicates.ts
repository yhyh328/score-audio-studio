/**
 * Kept in sync with the equivalent predicates in score-domain.
 * Duplicated here to avoid coupling playback-compiler to validation internals.
 */
export function isNonNegativeInteger(value: number): boolean {
  return Number.isSafeInteger(value) && value >= 0;
}

export function isPositiveInteger(value: number): boolean {
  return Number.isSafeInteger(value) && value > 0;
}
