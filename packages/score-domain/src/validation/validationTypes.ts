export type ValidationSeverity =
  | "error"
  | "fatal";

export type ValidationErrorCode =
  | "INVALID_SCHEMA_VERSION"
  | "INVALID_PPQ"
  | "EMPTY_TEMPO_MAP"
  | "TEMPO_MAP_MUST_START_AT_ZERO"
  | "UNSORTED_TEMPO_MAP"
  | "DUPLICATE_TEMPO_TICK"
  | "INVALID_TEMPO_TICK"
  | "INVALID_BPM"
  | "EMPTY_PARTS"
  | "DUPLICATE_ENTITY_ID"
  | "EMPTY_MEASURES"
  | "INVALID_MEASURE_NUMBER"
  | "DUPLICATE_MEASURE_NUMBER"
  | "UNSORTED_MEASURES"
  | "INVALID_TIME_SIGNATURE"
  | "INVALID_EVENT_OFFSET"
  | "INVALID_EVENT_DURATION"
  | "EVENT_EXCEEDS_MEASURE"
  | "MISSING_PITCH"
  | "INVALID_VELOCITY"
  | "INVALID_PITCH"
  | "INCONSISTENT_TIME_SIGNATURE";

export type ValidationTarget =
  | "entityId"
  | "event"
  | "measure"
  | "part"
  | "ppq"
  | "schemaVersion"
  | "scoreEvent"
  | "tempo";

export interface ValidationIssue {
  target: ValidationTarget;
  code: ValidationErrorCode;
  severity: ValidationSeverity;
  path: string;
  message: string;
}

export interface ValidationResult {
  valid: boolean;
  issues: ValidationIssue[];
}
