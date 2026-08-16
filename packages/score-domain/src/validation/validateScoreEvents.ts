import type { EntityId } from "../model/entityId";
import type { Intensity } from "../model/intensity";
import type { Pitch } from "../model/pitch";
import type { ScoreEvent } from "../model/scoreEvent";
import { isNonNegativeInteger,
         isPositiveInteger } from "./numberPredicates";
import { validateIntensity } from "./validateIntensity";
import { validatePitches } from "./validatePitches";
import { isUniqueEntityId,
         entityIdIssue } from "./validateUniqueEntityId";
import type { ValidationIssue } from "./validationTypes";	

export function validateScoreEvents(
    events: readonly ScoreEvent[], 
    measureLengthTicks: number | undefined,
    path: string, 
    usedEntityIds: Set<EntityId>
): ValidationIssue[] {

    const issues: ValidationIssue[] = [];

    for (const [index, event] of events.entries()) {
        const eventId = event.id;
        const eventPath: string = `${path}[${index}]`;
        const type = event.type;
        const offsetTicks = event.offsetTicks;
        const durationTicks = event.durationTicks;
        
        if (!isUniqueEntityId(eventId, usedEntityIds)) {
            issues.push(entityIdIssue(eventId, eventPath));
        }
        let 
        { 
            hasValidOffset, hasValidDuration 
        } = { 
            hasValidOffset: true, hasValidDuration: true 
        };
        if (!isNonNegativeInteger(offsetTicks)) {
            issues.push({
                target: "scoreEvent",
                severity: "error",
                code: "INVALID_EVENT_OFFSET",
                message: "Offset Ticks must be a non-negative integer.",
                path: `${eventPath}.offsetTicks`
            });	
            hasValidOffset = false;
        }
        if (!isPositiveInteger(durationTicks)) {
            issues.push({
                target: "scoreEvent",
                severity: "error",
                code: "INVALID_EVENT_DURATION",
                message: "Duration Ticks must be a positive integer.",
                path: `${eventPath}.durationTicks`
            });
            hasValidDuration = false;
        }
        if (hasValidOffset && hasValidDuration && 
            measureLengthTicks !== undefined &&
            offsetTicks + durationTicks > measureLengthTicks) {
            issues.push({
                target: "scoreEvent",
                severity: "error",
                code: "EVENT_EXCEEDS_MEASURE",
                message: "Event exceeds measure duration.",
                path: `${eventPath}.durationTicks`
            })
        }
        if (type === "note") {
            const pitches: Pitch[] = event.pitches;
            issues.push(...validatePitches(pitches, `${eventPath}.pitches`));
            const intensity: Intensity = event.intensity;
            issues.push(...validateIntensity(intensity, `${eventPath}`))
        }
    }
    return issues;
}
