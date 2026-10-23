import type {
    ScoreDocument,
    TempoEvent,
} from "@score-audio-studio/score-domain";
import type {
    TickPlaybackEvent,
    SamplePlaybackEvent
} from "../model/playbackEvent";
import type {
    TempoSegment
} from '../model/tempoSegment';
import {
    compileScoreToTicks
} from "./compileScoreToTicks";
import {
    buildTempoSegments
} from "../timing/buildTempoSegments";
import {
    tickToSamplePosition
} from "../timing/tickToSamplePosition";

// I haven't decided yet where users should input TMP_SAMPLE_RATE,
// so TMP_SAMPLE_RATE is temporarily set in this script.
// TMP_SAMPLE_RATE might be removed once I decide where it should be configured.
const TMP_SAMPLE_RATE: number = 48000;

export function compileScoreToSamplePositions(
    score: ScoreDocument
): SamplePlaybackEvent[] {
    const compiled = compileScoreToTicks(score);
    if (!compiled.success) {
        throw new Error(
            `Score compilation failed: ${
                JSON.stringify(compiled.validation)
            }`
        );
    }
    const ppq: number = score.ppq;
    const tempoMap: TempoEvent[] = score.tempoMap;
    const tickPlaybackEvents: TickPlaybackEvent[] =
        compiled.events;
    const tempoSegments: TempoSegment[] =
        buildTempoSegments(ppq, tempoMap); 
    const scoreEndTick: number = compiled.scoreEndTick;
    return tickPlaybackEvents.map(
        (event: TickPlaybackEvent) => {
            const { tick: _, ...baseEvent } = event;
            return {
                ...baseEvent,
                samplePosition: tickToSamplePosition(
                    event,
                    TMP_SAMPLE_RATE,
                    score.ppq,
                    tempoSegments,
                    scoreEndTick
                )
            } as SamplePlaybackEvent;
        }
    );
}
