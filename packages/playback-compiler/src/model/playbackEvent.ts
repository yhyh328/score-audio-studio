type PlaybackEventType = "noteOn" | "noteOff";

export interface TickPlaybackEvent {
    type: PlaybackEventType;
    tick: number;
    partId: string;
    noteId: string;
    midiNote: number;
    velocity: number; // 0 -> noteOff
}

export interface SecondsPlaybackEvent extends TickPlaybackEvent {
    seconds: number;
}

export interface SamplePlaybackEvent extends TickPlaybackEvent {
    samplePosition: bigint;
}