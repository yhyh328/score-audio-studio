import type {
    BinaryPlaybackEvent
} from "../model/playbackEvent";

const EVENT_SIZE: number = 16;
const LITTLE_ENDIAN: boolean = true;

export function encodePlaybackEvents(
    events: readonly BinaryPlaybackEvent[]
): Uint8Array {
    const buffer = new ArrayBuffer(
        events.length * EVENT_SIZE
    );
    const view = new DataView(buffer);
    events.forEach((event, index) => {
        const offset = index * EVENT_SIZE;
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
    });
    return new Uint8Array(buffer);
}
