import type { EntityId }    from "./entityId";						
import type { Pitch }       from "./pitch";						
import type { Intensity }   from "./intensity";						
						
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
