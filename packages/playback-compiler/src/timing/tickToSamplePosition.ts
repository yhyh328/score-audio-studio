import type { TickPlaybackEvent } from "../model/playbackEvent";
import type { TempoSegment } from "../model/tempoSegment";
import { 
    convertTickToAbsoluteSeconds 
} from "./convertTickToAbsoluteSeconds";

/**
 * Converts an event tick to the nearest discrete sample-frame position.
 *
 * The result uses bigint to represent sample positions as integer frame
 * indices consistently with SamplePlaybackEvent.
 *
 * The intermediate calculation uses number and is precise only while the
 * rounded sample position remains within Number's safe integer range.
 */

export function tickToSamplePosition(
    event: TickPlaybackEvent,
    sampleRate: number,
    ppq: number,
    segments: TempoSegment[],
    scoreEndTick: number,
): bigint {
    const seconds = convertTickToAbsoluteSeconds(
        ppq,
        event.tick,
        segments,
        scoreEndTick,
    );

    return BigInt(Math.round(seconds * sampleRate));
}