import { describe, expect, it } from "vitest";
import type {
    BinaryPlaybackEvent
} from "../../src/model/playbackEvent";
import {
    encodePlaybackEvents
} from "../../src/compiler/encodePlaybackEvents";

describe("writeBinaryFile", () => {
    const TMP_SAMPLE_RATE: number = 48000;
    const HEADER_SIZE: number = 24;
    const EVENT_SIZE: number = 16;
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
            const bytes = encodePlaybackEvents(
                events,
                TMP_SAMPLE_RATE
            );
            expect(bytes.byteLength).toBe(
                HEADER_SIZE + EVENT_SIZE * events.length
            );
            const view = new DataView(
                bytes.buffer,
                bytes.byteOffset,
                bytes.byteLength
            );
            // magic
            expect(
                view.getUint8(0)
            ).toBe(0x53);
            expect(
                view.getUint8(1)
            ).toBe(0x41);
            expect(
                view.getUint8(2)
            ).toBe(0x53);
            expect(
                view.getUint8(3)
            ).toBe(0x42);
            // version
            expect(
                view.getUint16(4, LITTLE_ENDIAN)
            ).toBe(1);
            // header size
            expect(
                view.getUint16(6, LITTLE_ENDIAN)
            ).toBe(HEADER_SIZE);
            // sample rate
            expect(
                view.getFloat64(8, LITTLE_ENDIAN)
            ).toBe(TMP_SAMPLE_RATE);
            // event count
            expect(
                view.getUint32(16, LITTLE_ENDIAN)
            ).toBe(events.length);
            // event size
            expect(
                view.getUint32(20, LITTLE_ENDIAN)
            ).toBe(EVENT_SIZE);
            // playback events
            let offset = HEADER_SIZE;
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
                offset += EVENT_SIZE;
            }
        }
    );
});
