import type {
    EntityId,
    Intensity,
    Measure,
    NoteEvent,
    Part,
    Pitch,
    ScoreDocument,
    ScoreEvent,
    ValidationIssue,
} from "@score-audio-studio/score-domain";

import {
    DEFAULT_DYNAMIC_VELOCITY_MAP,
    calculateMeasureLengthTicks,
    pitchToMidiNoteNumber,
    validateScoreDocument,
} from "@score-audio-studio/score-domain";

import type { TickPlaybackEvent } from "../model/playbackEvent";
import { sortTickPlaybackEvents } from "./sortTickPlaybackEvents";

interface CompiledScoreTicks {
    events: TickPlaybackEvent[];
    scoreEndTick: number;
}

type CompileScoreToTicksResult =
    | {
          success: true;
          events: TickPlaybackEvent[];
          scoreEndTick: number;
      }
    | {
          success: false;
          validation: ValidationIssue[];
      };

function compileNoteEventToTicks(
    partId: EntityId,
    noteEvent: NoteEvent,
    measureStartTick: number,
): TickPlaybackEvent[] {
    const noteOnTick =
        measureStartTick + noteEvent.offsetTicks;

    const noteOffTick =
        noteOnTick + noteEvent.durationTicks;

    const intensity: Intensity = noteEvent.intensity;

    const velocity =
        intensity.velocity ??
        DEFAULT_DYNAMIC_VELOCITY_MAP[intensity.dynamic];

    return noteEvent.pitches.flatMap((pitch: Pitch) => {
        const midiNote = pitchToMidiNoteNumber(pitch);

        const noteOnEvent: TickPlaybackEvent = {
            type: "noteOn",
            tick: noteOnTick,
            noteId: noteEvent.id,
            partId,
            midiNote,
            velocity,
        };

        const noteOffEvent: TickPlaybackEvent = {
            ...noteOnEvent,
            type: "noteOff",
            tick: noteOffTick,
            velocity: 0,
        };

        return [
            noteOnEvent,
            noteOffEvent,
        ];
    });
}

function compileValidatedScoreToTicks(
    score: ScoreDocument,
): CompiledScoreTicks {
    const result: TickPlaybackEvent[] = [];
    let scoreEndTick = 0;

    const parts: Part[] = score.parts;

    for (const part of parts) {
        let measureStartTick = 0;
        const partId = part.id;
        const measures: Measure[] = part.measures;

        for (const measure of measures) {
            const playbackEvents = measure.events
                .filter(
                    (event: ScoreEvent): event is NoteEvent =>
                        event.type === "note",
                )
                .flatMap((event) =>
                    compileNoteEventToTicks(
                        partId,
                        event,
                        measureStartTick,
                    ),
                );

            result.push(...playbackEvents);

            measureStartTick += calculateMeasureLengthTicks(
                measure.timeSignature,
                score.ppq,
            );
        }

        scoreEndTick =
            measureStartTick < scoreEndTick
                ? scoreEndTick
                : measureStartTick;
    }

    return {
        events: sortTickPlaybackEvents(result),
        scoreEndTick,
    };
}

export function compileScoreToTicks(
    score: ScoreDocument,
): CompileScoreToTicksResult {
    const validation = validateScoreDocument(score);

    if (!validation.valid) {
        return {
            success: false,
            validation: validation.issues,
        };
    }

    const compiled = compileValidatedScoreToTicks(score);

    return {
        success: true,
        events: compiled.events,
        scoreEndTick: compiled.scoreEndTick,
    };
}