import type { TickPlaybackEvent } from "../model/playbackEvent";

function comparator(
    a: TickPlaybackEvent,
    b: TickPlaybackEvent,
): number {
    if (a.tick !== b.tick) {
        return a.tick - b.tick;
    }

    if (a.type !== b.type) {
        return a.type === "noteOff" ? -1 : 1;
    }

    return a.midiNote - b.midiNote;
}

export function sortTickPlaybackEvents(
    tickPlaybackEvents: readonly TickPlaybackEvent[],
): TickPlaybackEvent[] {
    return tickPlaybackEvents.toSorted(comparator);
}