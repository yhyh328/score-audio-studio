import type { EntityId } from "../model/entityId";
import type { Part } from "../model/part"
import { validateMeasures } from "./validateMeasures";
import { isUniqueEntityId,
         entityIdIssue } from "./validateUniqueEntityId";
import type { ValidationIssue } from "./validationTypes";

export function validateParts(
    parts: readonly Part[], ppq: number | undefined, path: string): ValidationIssue[] {

    const issues: ValidationIssue[] = [];

    if (parts.length === 0) {
        issues.push({
            target: "part",
            severity: "error",
            code: "EMPTY_PARTS",
            message: "Score must contain at least one part.",
            path: `${path}`,
        });
        return issues;
    }
    const usedEntityIds = new Set<EntityId>();
    for (const [index, part] of parts.entries()) {
        const partId = part.id;
        const partPath: string = `${path}[${index}]`
        if (!isUniqueEntityId(partId, usedEntityIds)) {
            issues.push(entityIdIssue(partId, partPath));
        }
        const measures = part.measures;
        issues.push(
            ...validateMeasures(measures, ppq, `${partPath}.measures`, usedEntityIds)
        );
    }
    return issues;
}
