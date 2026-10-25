import type { ScoreDocument } from "./score.js";

export interface ProjectDocument {
	 schemaVersion : 1;
	 title         : string;
	 score         : ScoreDocument;
}
