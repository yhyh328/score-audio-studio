import type { Part }        from "./part";					
import type { TempoEvent }  from "./tempo";					
										
export interface ScoreDocument {					
	schemaVersion : 1;				
	ppq           : number;				
	tempoMap      : TempoEvent[];				
	parts         : Part[];				
}					