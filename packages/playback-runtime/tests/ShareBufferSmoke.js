import assert from "node:assert/strict";
import { createRequire } from "node:module";
import { resolve } from "node:path";

const addOnArgument = process.argv[2];

if (!addOnArgument) {
    throw new Error("Native add-on path is required");
}

const require = createRequire(import.meta.url);
const addOn = require(resolve(addOnArgument));

const RECORD_SIZE = 16;  // sizeof(PlaybackEvent) == 16
const buffer = new SharedArrayBuffer(RECORD_SIZE);

const view = new DataView(buffer);

/**
 * struct PlaybackEvent {
 *  PlaybackEventType type;
 *  std::uint8_t midiNoteNumber;
 *  std::uint16_t partId;
 *  std::uint32_t noteId;
 *  std::uint32_t sampleOffset;
 *  float velocity;
 * };
 */
/**
 *  view.setUintX(byteOffset, value, littleEndian)
 */
// type
view.setUint8(0x00, 1);

// midiNoteNumber
view.setUint8(0x01, 69, true);

// partId
view.setUint16(0x02, 0, true);

// noteId
view.setUint32(0x04, 0, true);

// sampleOffset
view.setUint32(0x08, 0, true);

// velocity
view.setFloat32(0x0c, 1, true);

const consumedCount = addOn.consume(buffer, 1);

assert.equal(consumedCount, 1);

assert.throws(
    () => addOn.consume(new ArrayBuffer(RECORD_SIZE), 1),
    TypeError,
);

assert.throws(
    () => addOn.consume(new SharedArrayBuffer(15), 1),
    RangeError,
);

console.log("SharedBuffer smoke test passed");
