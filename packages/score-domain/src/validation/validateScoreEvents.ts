import type { EntityId } from "../model/entityId.js";
import type { Intensity } from "../model/intensity.js";
import type { Pitch } from "../model/pitch.js";
import type { ScoreEvent } from "../model/scoreEvent.js";
import { isNonNegativeInteger,
isPositiveInteger } from "./numberPredicates.js";
import { validateIntensity } from "./validateIntensity.js";
import { validatePitches } from "./validatePitches.js";
import { isUniqueEntityId,
entityIdIssue } from "./validateUniqueEntityId.js";
import type { ValidationIssue } from "./validationTypes.js";

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
            issues.push(...validateIntensity(intensity, `${eventPath}.intensity`))
        }
    }
    return issues;
}
