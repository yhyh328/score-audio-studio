import type { ProjectDocument } from "../model/project";
import { isValidSchemaVersion } from "./validateSchemaVersion";
import { validateScoreDocument } from "./validateScoreDocument";
import type { ValidationIssue, ValidationResult } from "./validationTypes";

export function validateProjectDocument(project: ProjectDocument): ValidationResult {
  if (!isValidSchemaVersion("project", project.schemaVersion)) {
    const issue: ValidationIssue = {
      target: "schemaVersion",
      severity: "fatal",
      code: "INVALID_SCHEMA_VERSION",
      message: `Invalid schema version: ${project.schemaVersion}`,
      path: "schemaVersion",
    };
    return { valid: false, issues: [issue] };
  }

  return validateScoreDocument(project.score, "score");
}
