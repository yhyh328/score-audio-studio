type PlaybackEventType = "noteOn" | "noteOff";

export interface TickPlaybackEvent {
    type: PlaybackEventType;
    tick: number;
    partId: string;
    noteId: string;
    midiNote: number;
    velocity: number; // 0 -> noteOff
}
