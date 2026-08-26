/**
 * A constant-tempo interval used to convert score ticks into absolute time.
 *
 * `startSeconds` is the accumulated playback time at `startTick`,
 * including all preceding tempo segments.
 */
export interface TempoSegment {
    startTick: number;
    endTick: number | null;
    bpm: number;
    startSeconds: number;
}
