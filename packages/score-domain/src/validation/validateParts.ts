import type { EntityId } from "../model/entityId";
import type { Part } from "../model/part"
import { validateMeasures } from "./validateMeasures";
import { isUniqueEntityId,
         entityIdIssue } from "./validateUniqueEntityId";
import type { ValidationIssue } from "./validationTypes";
import type { TimeSignature } from "../model/timeSignature";

export function validateParts(
    parts: readonly Part[], ppq: number | undefined, path: string): ValidationIssue[] {

    const issues: ValidationIssue[] = [];

    const reference: Part | undefined = parts[0];
    if (reference === undefined) {
        issues.push({
            target: "part",
            severity: "error",
            code: "EMPTY_PARTS",
            message: "Score must contain at least one part.",
            path: `${path}`,
        });
        return issues;
    }
    const referenceByMeasureNumber: Partial<Record<number, TimeSignature>>= {};
    /**
     *  { number : timeSignature }
     *
     *  Maps each measure number to the reference part's time signature.
     *
     *  Polymeter support is out of scope for this phase.
     *  Corresponding measures in all parts must share a time signature.
     */
    for (const measure of reference.measures) {
        const number = measure.number;
        const timeSignature = measure.timeSignature;
        referenceByMeasureNumber[number] = timeSignature;
    }
    const usedEntityIds = new Set<EntityId>();
    for (const [index, part] of parts.entries()) {
        const partId = part.id;
        const partPath: string = `${path}[${index}]`
        if (!isUniqueEntityId(partId, usedEntityIds)) {
            issues.push(entityIdIssue(partId, partPath));
        }
        const measures = part.measures;
        for (const [jndex, measure] of measures.entries()) {
            const number = measure.number;
            const {
                numerator,
                denominator
            } = measure.timeSignature;
            const referenceTimeSignature = referenceByMeasureNumber[number];
            if (referenceTimeSignature === undefined) continue;
            if (
                numerator !== referenceTimeSignature.numerator ||
                denominator !== referenceTimeSignature.denominator
            ) {
                issues.push({
                    target: "measure",
                    severity: "error",
                    code: "INCONSISTENT_TIME_SIGNATURE",
                    message:
                        `Time signature for measure ${number} must match ` +
                        `${referenceTimeSignature.numerator}/` +
                        `${referenceTimeSignature.denominator}.`,
                    path: `${partPath}.measures[${jndex}].timeSignature`,
                });
            }
        }
        issues.push(
            ...validateMeasures(measures, ppq, `${partPath}.measures`, usedEntityIds)
        );
    }
    return issues;
}
