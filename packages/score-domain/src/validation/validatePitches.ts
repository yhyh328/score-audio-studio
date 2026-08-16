import type { Pitch } from "../model/pitch";
import { pitchToMidiNoteNumber } from "../conversion/pitchToMidiNoteNumber";
import type { ValidationIssue } from "./validationTypes";

/**
 * MIDI note numbers are encoded as 7-bit data values, so their range is
 * 0 through 127. With this project's C4 = 60 octave convention:
 *
 * - MIDI note number 0: C-1
 * - MIDI note number 60: C4 (middle C)
 * - MIDI note number 127: G9
 */
function isValidPitch(pitch: Pitch): boolean {
    try {
        pitchToMidiNoteNumber(pitch);
        return true;
    } catch (error) {
        if (error instanceof RangeError) return false;
        throw error;
    }
}

export function validatePitches(
    pitches: readonly Pitch[],
    path: string,
): ValidationIssue[] {
    const issues: ValidationIssue[] = [];

    if (pitches.length === 0) {
        issues.push({
            target: "event",
            severity: "error",
            code: "MISSING_PITCH",
            message: "Note must contain at least one pitch.",
            path,
        });
        return issues;
    }

    for (const [index, pitch] of pitches.entries()) {
        if (!isValidPitch(pitch)) {
            issues.push({
                target: "event",
                severity: "error",
                code: "INVALID_PITCH",
                message: "Pitch must resolve to a 7-bit MIDI note number between 0 (C-1) and 127 (G9).",
                path: `${path}[${index}]`,
            });
        }
    }

    return issues;
}
