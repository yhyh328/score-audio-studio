export function ticksToSeconds(
    tick: number,
    bpm: number,
    ppq: number,
) {
    return tick / ppq * 60 / bpm;
}