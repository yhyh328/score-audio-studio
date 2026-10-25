import type { EntityId }    from "./entityId.js";
import type { Pitch }       from "./pitch.js";
import type { Intensity }   from "./intensity.js";

export interface NoteEvent {
	type          : "note";
	id            : EntityId;
	offsetTicks   : number;
	durationTicks : number;
	pitches       : Pitch[];
	intensity     : Intensity;

}

export interface RestEvent {
	type          : "rest";
	id            : EntityId;
	offsetTicks   : number;
	durationTicks : number;
}

export type ScoreEvent = NoteEvent | RestEvent;
