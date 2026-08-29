import { describe, expect, it } from "vitest";
import { convertTickToAbsoluteSeconds } from "../../src/timing/convertTickToAbsoluteSeconds";

const ppq: number = 960;

describe("convertTickToAbsoluteSeconds", () => {
    it.each([
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
    ])
})