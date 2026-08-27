import type { TempoSegment } from "../model/tempoSegment";						
import { ticksToSeconds } from "./ticksToSeconds";
						
export function convertTickToAbsoluteSeconds(
	ppq: number,					
	tick: number,					
	segments: TempoSegment[],					
	scoreEndTick: number					
): number {						
	for (const segment of segments) {
		const { 				
			startTick, 			
			endTick, 			
			bpm, 			
			startSeconds 			
		} = segment;				
		if (endTick) {				
			if (startTick <= tick && tick <= endTick) {
				return startSeconds + ticksToSeconds(
						tick - startTick,
					   	bpm,
					   	ppq
					);	
			}			
		} else {				
			if (startTick <= tick &&			
				tick <= scoreEndTick) {		
				return startSeconds + ticksToSeconds(
						tick - startTick,
					   	bpm,
					   	ppq
					);	
			}			
		}				
	}					
	return -1 // something went wrong					
}						