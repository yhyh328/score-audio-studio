import type {
    BinaryPlaybackEvent
} from "../model/playbackEvent.js";

const HEADER_SIZE: number = 24;
const EVENT_SIZE: number = 16;
const LITTLE_ENDIAN: boolean = true;

export function encodePlaybackEvents(
    events: readonly BinaryPlaybackEvent[],
    sampleRate: number
): Uint8Array {
    const buffer = new ArrayBuffer(
        HEADER_SIZE + events.length * EVENT_SIZE
    );
    const view = new DataView(buffer);
    /** Uint8Array
     *  ┌──────────────────────┐
     *  │ magic = "SASB"       │ 0~3
     *  │ version = 1          │ 4~5
     *  │ headerSize = 24      │ 6~7
     *  │ sampleRate           │ 8~15
     *  │ eventCount           │ 16~19
     *  │ eventSize = 16       │ 20~23
     *  ├──────────────────────┤ ← offset 24
     *  │ PlaybackEvent[0]     │
     *  │ PlaybackEvent[1]     │
     *  │ ...                  │
     *  └──────────────────────┘
     */
    // magic
    view.setUint8(0, 0x53);                 // S (Score)
    view.setUint8(1, 0x41);                 // A (Audio)
    view.setUint8(2, 0x53);                 // S (Studio)
    view.setUint8(3, 0x42);                 // B (Binary)
    // version
    view.setUint16(4, 1, LITTLE_ENDIAN);    // version 1
    // header size
    view.setUint16(6, HEADER_SIZE, LITTLE_ENDIAN);
    // sample rate
    view.setFloat64(8, sampleRate, LITTLE_ENDIAN);
    // event count
    view.setUint32(16, events.length, LITTLE_ENDIAN);
    // event size
    view.setUint32(20, EVENT_SIZE, LITTLE_ENDIAN);
    // playback events
    let offset = HEADER_SIZE;
    events.forEach((event) => {
        if (
            !Number.isSafeInteger(event.samplePosition)
            ||
            event.samplePosition < 0
            ||
            0xff_ff_ff_ff < event.samplePosition
        ) {
            throw new RangeError(
                 `samplePosition must be a uint32: ${
                    event.samplePosition
                }`
            )
        }
        view.setUint8(
            offset,
            event.type,
        );
        view.setUint8(
            offset + 1,
            event.midiNote
        );
        view.setUint16(
            offset + 2,
            event.partId,
            LITTLE_ENDIAN
        );
        view.setUint32(
            offset + 4,
            event.noteId,
            LITTLE_ENDIAN
        );
        view.setUint32(
            offset + 8,
            event.samplePosition,
            LITTLE_ENDIAN
        );
        view.setFloat32(
            offset + 12,
            event.velocity,
            LITTLE_ENDIAN
        );
        offset += EVENT_SIZE;
    });
    return new Uint8Array(buffer);
}
