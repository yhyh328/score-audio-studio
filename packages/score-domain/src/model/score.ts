import type { Part }        from "./part.js";
import type { TempoEvent }  from "./tempo.js";


export const DEFAULT_PPQ = 480;

export interface ScoreDocument {
	schemaVersion : 1;
	ppq           : number;
	tempoMap      : TempoEvent[];
	parts         : Part[];
}
