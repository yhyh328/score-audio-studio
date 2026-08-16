/**
 * Diatonic step of a pitch.
 */
export type Step = "C" | "D" | "E" | "F" | "G" | "A" | "B";
/**
 * Chromatic alteration of a pitch.
 *  -2: double flat
 *  -1: flat
 *   0: natural
 *   1: sharp
 *   2: double sharp
 */
export type Alter = -2 | -1 | 0 | 1 | 2;
/**
 * Represents a musical pitch using scientific pitch notation (SPN).
 *
 * This project defines middle C as C4 (MIDI note number 60).
 *
 * Example:	
 * { step: "C", alter:  1, octave: 4 } represents C#4.
 * { step: "D", alter: -1, octave: 4 } represents D♭4.
 */
export interface Pitch {						
	step  : Step;					
	alter : Alter;					
	octave: number;					
}						
