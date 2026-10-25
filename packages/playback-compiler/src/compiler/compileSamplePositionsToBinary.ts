import type {
    SamplePlaybackEvent,
    BinaryPlaybackEvent
} from '../model/playbackEvent.js';

function newType(type: string): number {
    return (type == "noteOff") ? 0 : 1;
}

const noteIdMap: Map<string, number> = new Map;

function newNoteId(noteId: string): number {
    let val = noteIdMap.get(noteId)
    if (val === undefined) {
        val = noteIdMap.size + 1;
        if (0xff_ff_ff_ff <= val) {
            throw new Error(
                "Capacity of notes number is exceed!"
            );
        }
        noteIdMap.set(
            noteId, val
        );
    }
    return val;
}

const partIdMap: Map<string, number> = new Map;

function newPartId(partId: string): number {
    let val = partIdMap.get(partId);
    if (val === undefined) {
        val = partIdMap.size + 1;
        partIdMap.set(
            partId, val
        )
    }
    return val;
}

function newVelocity(velocity: number): number {
    return velocity / 0x7f;
}

export function compileSamplePositionsToBinary(
    samplePlaybackEvents: SamplePlaybackEvent[]
): BinaryPlaybackEvent[] {
    return samplePlaybackEvents.map(
        (event: SamplePlaybackEvent) => {
            const {
                type: _,
                noteId: __,
                partId: ___,
                velocity: ____,
                ...baseEvent
            } = event;
            const type = newType(
                event.type
            );
            const noteId = newNoteId(
                event.noteId
            );
            const partId = newPartId(
                event.partId
            );
            const velocity = newVelocity(
                event.velocity
            );
            return {
                ...baseEvent,
                type: type,
                noteId: noteId,
                partId: partId,
                velocity: velocity
            } as BinaryPlaybackEvent;
        }
    );
}
