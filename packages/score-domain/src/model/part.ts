import type { EntityId } from "./entityId.js";
import type { Measure }  from "./measure.js";
					
export interface Part {					
	id      : EntityId;	
	name	: string;	
	measures: Measure[];	
}
