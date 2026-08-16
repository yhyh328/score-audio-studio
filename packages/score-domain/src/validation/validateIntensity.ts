import type { Intensity } from "../model/intensity";
import { isPositiveInteger } from "./numberPredicates";
import type { ValidationIssue } from "./validationTypes";

/**
 * MIDI velocity is encoded as a 7-bit data value, so the protocol range is
 * 0 through 127. This score model stores velocity for a sounding Note On,
 * where velocity 0 is excluded because it is conventionally interpreted as
 * Note Off. Therefore, a velocity override must be an integer from 1 to 127.
 */
function isValidVelocity(velocity: number): boolean {
    return isPositiveInteger(velocity) && velocity <= 127;
}

export function validateIntensity(
    intensity: Intensity,
    path: string,
): ValidationIssue[] {
    const issues: ValidationIssue[] = [];
    const velocity = intensity.velocity;
    if (velocity !== undefined && !isValidVelocity(velocity)) {
        issues.push({
            target: "event",
            severity: "error",
            code: "INVALID_VELOCITY",
            message: "Velocity override must be an integer between 1 and 127; MIDI Note On velocity 0 is interpreted as Note Off.",
            path: `${path}.velocity`,
        });
    }
    return issues;
}
