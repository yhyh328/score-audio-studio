import { describe, expect, it } from "vitest";
import type { TimeSignature } from "../src/model/timeSignature";
import { isValidTimeSignature } from "../src/validation/isValidTimeSignature";

const denominators: readonly TimeSignature["denominator"][] = [
    // only 1, 2, 4, 8 and 16 can be denominator in this project.
    1, 2, 4, 8, 16
];

function makeTimeSignatures(
    numerators: readonly number[],
): TimeSignature[] { 
    return numerators.flatMap(
        (numerator) => 
            denominators.map(
                (denominator) => ({
                    numerator,
                    denominator
            })
        )
    );
}

describe("validateTimeSignature", () => {

    const validTimeSignatures = makeTimeSignatures([
        1, 
        2, 
        3, 
        4, 
        5, 
        29,
        30 // max time signature numerator is 30.
    ]);
    it.each(validTimeSignatures)(
        "accepts time signature: $numerator/$denominator",
        ({numerator, denominator}) => {
        const result: boolean = isValidTimeSignature(
            numerator, denominator
        )
        expect(result).toBe(true);
    })

    const invalidTimeSignatures = makeTimeSignatures([
        0, 
        -1, 
        0.5, 
        31,
        100,
        1000,
        Number.NaN, 
        Number.POSITIVE_INFINITY, 
        Number.MAX_SAFE_INTEGER,
        Number.MAX_VALUE
    ]);
    it.each(invalidTimeSignatures)(
        "rejects time signature: $numerator/$denominator",
        ({numerator, denominator}) => {
        const result: boolean = isValidTimeSignature(
            numerator, denominator
        )
        expect(result).toBe(false);
    })
    
})
  