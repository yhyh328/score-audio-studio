import type { EntityId }   from "./entityId.js";
import type { ScoreEvent } from "./scoreEvent.js";
import type { TimeSignature } from "./timeSignature.js"
							
export interface Measure {					
	id           	: EntityId;				
	number       	: number;				
	timeSignature	: TimeSignature;				
	events       	: ScoreEvent[];				
}					
