import type { EntityId } from "./entityId";					
import type { Measure }  from "./measure";					
					
export interface Part {					
	id      : EntityId;	
	name	: string;	
	measures: Measure[];	
}
