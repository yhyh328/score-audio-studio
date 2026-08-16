import type { EntityId } from "../model/entityId";
import type { Measure } from "../model/measure";
import type { TimeSignature } from "../model/timeSignature";
import type { ScoreEvent } from "../model/scoreEvent";
import { calculateMeasureLengthTicks } from "../conversion/calculateMeasureLengthTicks";
import { isPositiveInteger } from "./numberPredicates";
import { validateScoreEvents } from "./validateScoreEvents";
import { isUniqueEntityId,
         entityIdIssue } from "./validateUniqueEntityId";
import { isValidTimeSignature } from "./isValidTimeSignature";						
import type { ValidationIssue } from "./validationTypes";							

export function validateMeasures(
    measures: readonly Measure[], 
    ppq: number | undefined,
    path: string, 
    usedEntityIds: Set<EntityId>
): ValidationIssue[] {

    const issues: ValidationIssue[] = [];

    if (measures.length === 0) {
        issues.push({
            target: "measure",
            severity: "error",
            code: "EMPTY_MEASURES",
            message: "Part must contain at least one measure.",
            path,
        });
        return issues;
    }

    const usedMeasureNumbers = new Set<number>();
    let previousMeasureNumber: number | undefined;
    for (const [index, measure] of measures.entries()) {
        const measureId = measure.id;
        const measurePath: string = `${path}[${index}]`;
        if (!isUniqueEntityId(measureId, usedEntityIds)) {
            issues.push(entityIdIssue(measureId, measurePath));
        }
        const number = measure.number;
        const hasValidNumber = isPositiveInteger(number);
        if (!hasValidNumber) {
            issues.push({
                target: "measure",
                severity: "error",
                code: "INVALID_MEASURE_NUMBER",
                message: "Measure number must be a positive integer.",
                path: `${measurePath}.number`,
            });
        } else if (usedMeasureNumbers.has(number)) {
            issues.push({
                target: "measure",
                severity: "error",
                code: "DUPLICATED_MEASURE_NUMBER",
                message: `Measure number must be unique: ${number}`,
                path: `${measurePath}.number`,
            });
        }

        if (
            hasValidNumber &&
            previousMeasureNumber !== undefined &&
            number < previousMeasureNumber
        ) {
            issues.push({
                target: "measure",
                severity: "error",
                code: "UNSORTED_MEASURES",
                message: "Measures must be sorted by ascending number.",
                path: `${measurePath}.number`,
            });
        }

        if (hasValidNumber) {
            usedMeasureNumbers.add(number);
            previousMeasureNumber = number;
        }
        const timeSignature: TimeSignature = measure.timeSignature;	
        const numerator = timeSignature.numerator;
        const denominator = timeSignature.denominator;							
        const hasValidTimeSignature = isValidTimeSignature(numerator, denominator);
        if (!hasValidTimeSignature) {
            issues.push({
                target: "measure",
                severity: "error",
                code: "INVALID_TIME_SIGNATURE",
                message: "The time signature is invalid.",
                path: `${measurePath}.timeSignature`
            });
        }
        const events: ScoreEvent[] = measure.events;
        const measureLengthTicks =
            ppq !== undefined && hasValidTimeSignature
                ? calculateMeasureLengthTicks(timeSignature, ppq)
                : undefined;
        issues.push(
            ...validateScoreEvents(
                events, 
                measureLengthTicks,
                `${measurePath}.events`, 
                usedEntityIds
            )
        );
    }
    return issues;
}
