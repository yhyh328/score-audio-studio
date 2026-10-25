import type { TickPlaybackEvent } from "../model/playbackEvent.js";
import {
    isNonNegativeInteger,
    isPositiveInteger
} from "../utils/numberPredicates.js";
import type { TempoSegment } from "../model/tempoSegment.js";
import { 
    convertTickToAbsoluteSeconds 
} from "./convertTickToAbsoluteSeconds.js";

/**
 * Converts an event tick to the nearest discrete sample-frame position.
 *
 * Returns a non-negative safe integer sample index.
 * Throws if sampleRate is not a positive safe integer 
 * or if the resulting sample position cannot be represented as a safe integer.
 */

export function tickToSamplePosition(
    event: TickPlaybackEvent,
    sampleRate: number,
    ppq: number,
    segments: TempoSegment[],
    scoreEndTick: number,
): number {
    if (!isPositiveInteger(sampleRate)) {
        throw new Error("Sample rate must be a positive safe integer");
    }
    const seconds = convertTickToAbsoluteSeconds(
        ppq,
        event.tick,
        segments,
        scoreEndTick,
    );
    const samplePosition = Math.round(seconds * sampleRate);
    if (
        !isNonNegativeInteger(samplePosition)
        ||
        0xff_ff_ff_ff < samplePosition  // check samplePosition is 32 bits
    ) {
        throw new Error(`Sample position exceeds uint32 range: ${samplePosition}`);
    }
    return samplePosition;
}
