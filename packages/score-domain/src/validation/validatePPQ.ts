/**
 * Validates PPQ (Pulses Per Quarter Note).
 *
 * PPQ defines the number of ticks representing one quarter note.
 * This project uses integer tick positions and durations rather than
 * fractional or rational tick values.
 *
 * A valid PPQ must be a positive integer.
 *
 * This project additionally limits PPQ to 32767 (0x7FFF) for
 * compatibility with the time-division field of Standard MIDI Files (SMF).
 *
 * The SMF time-division field is 16 bits wide, but its most significant
 * bit (MSB) is not part of the PPQ value:
 *
 * - MSB = 0: PPQ-based timing.
 *   The remaining 15 bits represent the PPQ value.
 *
 * - MSB = 1: SMPTE-based timing.
 *   The field is interpreted as frame-based timing rather than PPQ.
 *
 * Therefore, PPQ-based SMF timing can use values from
 * 1 to 32767 (0x0001 to 0x7FFF).
 *
 * Not every valid PPQ provides the same rhythmic resolution when using
 * integer ticks. If a rhythmic subdivision results in a fractional tick
 * value, it cannot be represented exactly and requires approximation.
 *
 * For example:
 *
 * - An odd PPQ cannot represent an eighth note exactly in integer ticks
 *   because an eighth note requires PPQ / 2 ticks.
 *
 * - A PPQ not divisible by 3 cannot represent a quarter-note triplet
 *   exactly in integer ticks because it requires PPQ / 3 ticks.
 *
 * - Smaller PPQ values provide coarser timing resolution, making fine
 *   rhythmic subdivisions more likely to require approximation.
 *
 * These are representational limitations of the integer-tick model,
 * not PPQ validation errors. This validator therefore accepts any
 * positive integer within the SMF-compatible PPQ range.
 *
 * These are representational limitations of the integer-tick model,
 * not PPQ validation errors. This validator therefore accepts any
 * positive integer within the SMF-compatible PPQ range.
 *
 * This project uses 480 PPQ as the default resolution because it provides
 * sufficient timing resolution for typical musical use and allows many
 * common rhythmic subdivisions, including binary subdivisions, triplets,
 * and quintuplets, to be represented exactly using integer ticks.
 *
 * The default value does not restrict valid PPQ values; other PPQ values
 * within the supported range are accepted for flexibility and MIDI
 * interoperability.
 */


const MAX_PPQ = 0x7fff;

export const DEFAULT_PPQ = 480;

export function validatePPQ(ppq: number): boolean {
    return (Number.isSafeInteger(ppq) // check if the ppq is a safe integer
        &&  ppq > 0               // check if the ppq is positive
        &&  ppq <= MAX_PPQ        // check if the ppq is not greater than the max value
    );
}
