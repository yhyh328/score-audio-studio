import type { TickPlaybackEvent } from "../model/playbackEvent";
import type { TempoSegment } from "../model/tempoSegment";
import { 
    convertTickToAbsoluteSeconds 
} from "./convertTickToAbsoluteSeconds";

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