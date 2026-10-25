import type {
    ScoreDocument
} from "@score-audio-studio/score-domain";
import type {
    SamplePlaybackEvent,
    BinaryPlaybackEvent
} from "../model/playbackEvent.js";
import {
    compileScoreToSamplePositions
} from "../compiler/compileScoreToSamplePositions.js";
import {
    compileSamplePositionsToBinary
} from "../compiler/compileSamplePositionsToBinary.js";
import {
    encodePlaybackEvents
} from "../compiler/encodePlaybackEvents.js";

export function compileScoreToBuffer(
    score: ScoreDocument,
    sampleRate: number
): Uint8Array {
    const samplePlaybackEvents: SamplePlaybackEvent[] =
        compileScoreToSamplePositions(
            score,
            sampleRate
        );
    const binaryPlaybackEvents: BinaryPlaybackEvent[] =
        compileSamplePositionsToBinary(
            samplePlaybackEvents
        );
    return encodePlaybackEvents(
        binaryPlaybackEvents,
        sampleRate
    );
}
