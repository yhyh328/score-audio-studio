type PlaybackEventType = "noteOn" | "noteOff";

export interface BasePlaybackEvent {
    midiNote: number;
    velocity: number; // 0 -> noteOff
}

export interface TickPlaybackEvent extends BasePlaybackEvent {
    type: PlaybackEventType;
    partId: string;
    noteId: string;
    tick: number;
}

export interface SamplePlaybackEvent extends BasePlaybackEvent {
    type: PlaybackEventType;
    partId: string;
    noteId: string;
    samplePosition: number;
}

export interface BinaryPlaybackEvent extends BasePlaybackEvent {
    type: number;
    partId: number;
    noteId: number;
    samplePosition: number;
}
