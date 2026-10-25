import { describe, expect, it } from "vitest";
import type {
    SamplePlaybackEvent,
    BinaryPlaybackEvent
} from "../../src/model/playbackEvent" 
import {
    compileSamplePositionsToBinary
} from "../../src/compiler/compileSamplePositionsToBinary";

function createSamplePlaybackEvents(): SamplePlaybackEvent[] {
    const noteIds: string[] = [
        `measure-${crypto.randomUUID()}`,
        `measure-${crypto.randomUUID()}`,
        `measure-${crypto.randomUUID()}`,
        `measure-${crypto.randomUUID()}`,
        `measure-${crypto.randomUUID()}`,
    ];
    const partIds: string[] = [
        `part-${crypto.randomUUID()}`,
        `part-${crypto.randomUUID()}`,
        `part-${crypto.randomUUID()}`,
        `part-${crypto.randomUUID()}`,
        `part-${crypto.randomUUID()}`
    ];
    const res: SamplePlaybackEvent[] = [];
    let [ noteId, partId ]: string[] = [ "", "" ];
    let [ i, j ]: number[] = [ -1, -1 ];
    let samplePosition: number = 0;
    for (i = 0; i < 50; i++) {
        j = Math.floor(i / 10);
        noteId = noteIds[j]!;
        partId = partIds[j]!;
        res.push({
            type: "noteOn",
            midiNote: 60,
            velocity: 72,
            samplePosition: samplePosition,
            noteId: noteId,
            partId: partId
        });
        samplePosition++;
        res.push({
            type: "noteOff",
            midiNote: 60,
            velocity: 0,
            samplePosition: samplePosition,
            noteId: noteId,
            partId: partId
        });
    }
    return res;
}

describe("compileSamplePositionsToBinary", () => {
    it("compiles binary playback events", () => {
        const samplePlaybackEvents: SamplePlaybackEvent[] = 
            createSamplePlaybackEvents();
        const binaryPlaybackEvents: BinaryPlaybackEvent[] =
            compileSamplePositionsToBinary(
                samplePlaybackEvents
            );
        let quotient: number = 0;
        for (let i = 0; i < binaryPlaybackEvents.length; i++) {
            const sEvent = samplePlaybackEvents[i];
            const bEvent = binaryPlaybackEvents[i];
            // check type
            (sEvent!.type == "noteOff") ? expect(bEvent!.type).toBe(0)
                                        : expect(bEvent!.type).toBe(1);
            // check midiNoteNumber
            expect(sEvent!.midiNote).toBe(bEvent!.midiNote);
            // check velocity
            expect(Math.fround(sEvent!.velocity / 127))
            .toBe(Math.fround(bEvent!.velocity));
            // check samplePosition
            expect(sEvent!.samplePosition).toBe(bEvent!.samplePosition);
            // check noteId
            expect(Number.isInteger(bEvent!.noteId)).toBe(true);
            if (i % 10 != 0) {
                const prevBEvent = binaryPlaybackEvents[i - 1];
                expect(bEvent!.noteId).toBe(prevBEvent!.noteId);
                if (quotient != i + 1 / 10) {
                    quotient++;
                }
            } 
            // check partId
            expect(Number.isInteger(bEvent!.partId)).toBe(true);
        }
    });
})
