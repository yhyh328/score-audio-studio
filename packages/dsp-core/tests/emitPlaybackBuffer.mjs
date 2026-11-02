import {
    readFile
} from "node:fs/promises";
import {
    compileScoreToBuffer
} from "../../playback-compiler/dist/index.js";
/**
 * Tests use JSON fixtures, but production passes
 * the Phase 1 object conforming to
 * @score-audio-studio/score-domain's ScoreDocument
 * directly without JSON serialization.
 */
const scorePath = new URL(
    "../../../docs/Score/test-score.json",
    import.meta.url
);
const score = JSON.parse(
    await readFile(scorePath, "utf8")
);
const buffer = compileScoreToBuffer(
    score,
    48_000 // sampleRate
);
process.stdout.write(
    Buffer.from(
        buffer.buffer,
        buffer.byteOffset,
        buffer.byteLength
    )
);
