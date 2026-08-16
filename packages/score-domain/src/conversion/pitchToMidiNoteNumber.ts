import type { Alter, Pitch, Step } from "../model/pitch";

const NATURAL_SEMITONE_BY_STEP = {
  C: 0,
  D: 2,
  E: 4,
  F: 5,
  G: 7,
  A: 9,
  B: 11,
} as const;

const VALID_ALTERS: readonly Alter[] = [-2, -1, 0, 1, 2];
const MIN_MIDI_NOTE_NUMBER = 0;
const MAX_MIDI_NOTE_NUMBER = 127;

export function pitchToMidiNoteNumber(pitch: Pitch): number {
  const naturalSemitone = (NATURAL_SEMITONE_BY_STEP as Partial<Record<Step, number>>)[pitch.step];
  const hasValidStep = naturalSemitone !== undefined;
  const hasValidAlter = VALID_ALTERS.includes(pitch.alter);
  const hasValidOctave = Number.isSafeInteger(pitch.octave);

  if (!hasValidStep || !hasValidAlter || !hasValidOctave) {
    throw new RangeError("Pitch must use a valid step, alter, and integer octave.");
  }

  const midiNoteNumber = 12 * (pitch.octave + 1) + naturalSemitone + pitch.alter;
  if (
    !Number.isSafeInteger(midiNoteNumber) ||
    midiNoteNumber < MIN_MIDI_NOTE_NUMBER ||
    midiNoteNumber > MAX_MIDI_NOTE_NUMBER
  ) {
    throw new RangeError("Pitch must resolve to a MIDI note number between 0 and 127.");
  }

  return midiNoteNumber;
}
