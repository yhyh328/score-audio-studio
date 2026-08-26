import type { TempoEvent } from "@score-audio-studio/score-domain";
import type { TempoSegment } from "../model/tempoSegment";

function ticksToSeconds(
    tick: number,
    bpm: number,
    ppq: number,
) {
    return tick / ppq * 60 / bpm;
}
/**
 * Converts tempo-change events into constant-tempo segments.
 *
 * Each segment stores the accumulated playback time at its start tick,
 * allowing ticks to be converted to absolute seconds without repeatedly
 * recalculating all preceding tempo changes.
 *
 * Assumes tempoEvents are sorted by tick and begin at tick 0.
 */
export function buildTempoSegments(
    ppq: number,
    tempoEvents: TempoEvent[],
): TempoSegment[] {
    
    let accumulatedSeconds = 0;

    const result: TempoSegment[] = [];

    for (const [index, event] of tempoEvents.entries()) {
        const nextEvent = tempoEvents[index + 1];

        const startTick = event.tick;
        const endTick = nextEvent?.tick ?? null;
        const bpm = event.bpm;

        result.push({
            startTick,
            endTick,
            bpm,
            startSeconds: accumulatedSeconds,
        });

        if (endTick !== null) {
            accumulatedSeconds += ticksToSeconds(
                endTick - startTick,
                bpm,
                ppq,
            );
        }
    }
    return result;
}
