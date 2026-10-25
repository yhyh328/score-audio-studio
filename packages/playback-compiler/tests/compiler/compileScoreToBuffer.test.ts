import {
    describe,
    expect,
    it
} from "vitest";

import type {
    ScoreDocument
} from "@score-audio-studio/score-domain";

import {
    DEFAULT_PPQ
} from "@score-audio-studio/score-domain";

import {
    compileScoreToBuffer
} from "../../src/compiler/compileScoreToBuffer.js";

describe("compileScoreToBuffer", () => {
    const SAMPLE_RATE = 48_000;
    const HEADER_SIZE = 24;
    const EVENT_SIZE = 16;
    const LITTLE_ENDIAN = true;

    it("compiles a score into the playback buffer layout", () => {
        const score: ScoreDocument = {
            schemaVersion: 1,
            ppq: DEFAULT_PPQ,
            tempoMap: [{
                tick: 0,
                bpm: 120
            }],
            parts: [{
                id: `part-${crypto.randomUUID()}`,
                name: "Piano",
                measures: [{
                    id: `measure-${crypto.randomUUID()}`,
                    number: 1,
                    timeSignature: {
                        numerator: 4,
                        denominator: 4
                    },
                    events: [{
                        type: "note",
                        id: `event-${crypto.randomUUID()}`,
                        offsetTicks: 0,
                        durationTicks: 480,
                        pitches: [{
                            step: "C",
                            alter: 0,
                            octave: 4
                        }],
                        intensity: {
                            dynamic: "mf"
                        }
                    }]
                }]
            }]
        };

        const bytes = compileScoreToBuffer(
            score,
            SAMPLE_RATE
        );

        // noteOn + noteOff
        expect(bytes.byteLength).toBe(
            HEADER_SIZE + EVENT_SIZE * 2
        );

        const view = new DataView(
            bytes.buffer,
            bytes.byteOffset,
            bytes.byteLength
        );

        // magic
        expect(view.getUint8(0)).toBe(0x53);
        expect(view.getUint8(1)).toBe(0x41);
        expect(view.getUint8(2)).toBe(0x53);
        expect(view.getUint8(3)).toBe(0x42);
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
        ).toBe(SAMPLE_RATE);
        // event count
        expect(
            view.getUint32(16, LITTLE_ENDIAN)
        ).toBe(2);
        // event size
        expect(
            view.getUint32(20, LITTLE_ENDIAN)
        ).toBe(EVENT_SIZE);

        const noteOnOffset = HEADER_SIZE;
        const noteOffOffset = HEADER_SIZE + EVENT_SIZE;

        // noteOn
        expect( // type
            view.getUint8(noteOnOffset)
        ).toBe(1);
        expect( // midiNote
            view.getUint8(noteOnOffset + 1)
        ).toBe(60);
        expect( // partId
            view.getUint16(
                noteOnOffset + 2,
                LITTLE_ENDIAN
            )
        ).toBe(1);
        expect( // noteId
            view.getUint32(
                noteOnOffset + 4,
                LITTLE_ENDIAN
            )
        ).toBe(1);
        expect( // samplePosition
            view.getUint32(
                noteOnOffset + 8,
                LITTLE_ENDIAN
            )
        ).toBe(0);
        expect( // velocity
            view.getFloat32(
                noteOnOffset + 12,
                LITTLE_ENDIAN
            )
        ).toBe(Math.fround(72 / 127));

        // noteOff
        expect( // type
            view.getUint8(noteOffOffset)
        ).toBe(0);

        expect( // midiNote
            view.getUint8(noteOffOffset + 1)
        ).toBe(60);

        expect( // partId
            view.getUint16(
                noteOffOffset + 2,
                LITTLE_ENDIAN
            )
        ).toBe(1);
        expect( // noteId
            view.getUint32(
                noteOffOffset + 4,
                LITTLE_ENDIAN
            )
        ).toBe(1);
        expect( // samplePosition
            view.getUint32(
                noteOffOffset + 8,
                LITTLE_ENDIAN
            )
        ).toBe(24_000);
        expect( // velocity
            view.getFloat32(
                noteOffOffset + 12,
                LITTLE_ENDIAN
            )
        ).toBe(0);
    });

    it("rejects an invalid score", () => {
        const invalidScore = {
            schemaVersion: 2,
            ppq: 0,
            tempoMap: [],
            parts: []
        } as unknown as ScoreDocument;

        expect(
            () => compileScoreToBuffer(
                invalidScore,
                SAMPLE_RATE
            )
        ).toThrow("Score compilation failed");
    });
});
