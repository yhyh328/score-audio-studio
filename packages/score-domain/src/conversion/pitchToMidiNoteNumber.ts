import type { Pitch } from "../model/pitch";

const NATURAL_SEMITONE_BY_STEP = {
  C: 0,
  D: 2,
  E: 4,
  F: 5,
  G: 7,
  A: 9,
  B: 11,
} as const;

export function pitchToMidiNoteNumber(pitch: Pitch): number {
  const octave = pitch.octave;
  const naturalSemitone = NATURAL_SEMITONE_BY_STEP[pitch.step];
  const alter = pitch.alter;
  const midiNoteNumber = 12 * (octave + 1) + naturalSemitone + alter;
  return midiNoteNumber;
}
