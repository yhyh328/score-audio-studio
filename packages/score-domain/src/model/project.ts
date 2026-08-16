import type { ScoreDocument } from "./score";		

export interface ProjectDocument {						
	 schemaVersion : 1;
	 title         : string;
	 score         : ScoreDocument;
}