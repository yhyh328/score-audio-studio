import { describe, expect, it } from "vitest";
import type {
    BinaryPlaybackEvent
} from "../../src/model/playbackEvent";
import {
    encodePlaybackEvents
} from "../../src/compiler/encodePlaybackEvents";

describe("writeBinaryFile", () => {
    const LITTLE_ENDIAN: boolean = true;
    let [ noteId, partId ]: number[] = [
        0x00_00_00_00,
        0x00_00_00_00,
    ];
    it
    (
        "encodes a playback event into the expected 16-byte layout",
        () => {
            const events: BinaryPlaybackEvent[] = [{
                type: 1,
                midiNote: 60,
                velocity: 72,
                samplePosition: 0,
                noteId: noteId!,
                partId: partId!
            }, {
                type: 0,
                midiNote: 60,
                velocity: 0,
                samplePosition: 24_000,
                noteId: noteId!,
                partId: partId!
            }];
            const bytes = encodePlaybackEvents(events);
            expect(bytes.byteLength).toBe(16 * events.length);
            const view = new DataView(
                bytes.buffer,
                bytes.byteOffset,
                bytes.byteLength
            );
            let offset = 0;
            for (let i = 0; i < events.length; i++) {
                expect(
                    view.getUint8(offset + 0)
                ).toBe(events[i]!.type);
                expect(
                    view.getUint8(offset + 1)
                ).toBe(events[i]!.midiNote);
                expect(
                    view.getUint16(offset + 2, LITTLE_ENDIAN)
                ).toBe(events[i]!.partId);
                expect(
                    view.getUint32(offset + 4, LITTLE_ENDIAN)
                ).toBe(events[i]!.noteId);
                expect(
                    view.getUint32(offset + 8, LITTLE_ENDIAN)
                ).toBe(events[i]!.samplePosition);
                expect(
                    view.getFloat32(offset + 12, LITTLE_ENDIAN)
                ).toBe(events[i]!.velocity);
                offset += 16;
            }
        }
    );
});
