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
	/**
	 * Phase 1 validation and buildTempoSegments guarantee
	 * that every playback tick is covered by a tempo segment.
	 * Reaching this point indicates an internal invariant violation. */
	throw new Error(
		`Invariant violation: no tempo segment covers tick ${tick}.`,
	);
}
