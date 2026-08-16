import type { EntityId }   from "./entityId";					
import type { ScoreEvent } from "./scoreEvent";
import type { TimeSignature } from "./timeSignature"	
							
export interface Measure {					
	id           	: EntityId;				
	number       	: number;				
	timeSignature	: TimeSignature;				
	events       	: ScoreEvent[];				
}					
