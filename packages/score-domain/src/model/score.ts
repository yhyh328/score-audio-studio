import type { Part }        from "./part";					
import type { TempoEvent }  from "./tempo";	


export const DEFAULT_PPQ = 480;
										
export interface ScoreDocument {					
	schemaVersion : 1;				
	ppq           : number;				
	tempoMap      : TempoEvent[];				
	parts         : Part[];				
}					