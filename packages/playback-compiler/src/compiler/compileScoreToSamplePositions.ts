import type {
    ScoreDocument,
    TempoEvent,
} from "@score-audio-studio/score-domain";
import type {
    TickPlaybackEvent,
    SamplePlaybackEvent
} from "../model/playbackEvent.js";
import type {
    TempoSegment
} from '../model/tempoSegment.js';
import {
    compileScoreToTicks
} from "./compileScoreToTicks.js";
import {
    buildTempoSegments
} from "../timing/buildTempoSegments.js";
import {
    tickToSamplePosition
} from "../timing/tickToSamplePosition.js";

export function compileScoreToSamplePositions(
    score: ScoreDocument,
    sampleRate: number
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
                    sampleRate,
                    score.ppq,
                    tempoSegments,
                    scoreEndTick
                )
            } as SamplePlaybackEvent;
        }
    );
}
