import { describe, expect, it } from "vitest";
import type { TempoSegment } from "../../src/model/tempoSegment";
import { buildTempoSegments } from "../../src/timing/buildTempoSegments";

const ppq: number = 480;

describe("buildTempoSegments", () => {
    
    /**
     * phase-1-score-domain rejects empty tempo map as default.
     */

    it("builds a single tempo segment", () => {
        expect(buildTempoSegments(ppq, [
            { tick: 0, bpm: 100 }
        ])).toEqual([
            {
                startTick: 0,
                endTick: null,
                bpm: 100,
                startSeconds: 0
            }
        ]);
    });

    it("builds multiple tempo segments", () => {
        expect(buildTempoSegments(ppq, [
            { tick: 0, bpm: 100 },
            { tick: 1, bpm: 120 },
            { tick: 100, bpm: 110 },
            { tick: 480, bpm: 200 },
            { tick: 5000, bpm: 50 },
            { tick: 9999, bpm: 20 },
            { tick: Number.MAX_SAFE_INTEGER - 2, bpm: 300 }
        ])).toEqual([
            {
                startTick: 0,
                endTick: 1,
                bpm: 100,
                startSeconds: 0
            },
            {
                startTick: 1,
                endTick: 100,
                bpm: 120,
                startSeconds: 0.00125
            },
            {
                startTick: 100,
                endTick: 480,
                bpm: 110,
                startSeconds: 0.104375
            },
            {
                startTick: 480,
                endTick: 5000,
                bpm: 200,
                startSeconds: 0.5361931818181818
            },
            {
                startTick: 5000,
                endTick: 9999,
                bpm: 50,
                startSeconds: 3.361193181818182
            },
            {
                startTick: 9999,
                endTick: 9_007_199_254_740_989,
                bpm: 20,
                startSeconds: 15.858693181818182,
            },
            {
                startTick: Number.MAX_SAFE_INTEGER - 2,
                endTick: null,
                bpm: 300,
                startSeconds: 56_294_995_342_084.555
            }
        ]);
    });

})