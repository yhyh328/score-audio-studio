#include <bit>
#include <cassert>
#include <cerrno>
#include <cmath>
#include <span>
#include <stdexcept>
#include <stdio.h>
#include <vector>

#include "score_audio_studio/playback_runtime/PlaybackRuntime.hpp"

std::vector<std::uint8_t> runPlaybackCompiler() {
#ifdef _WIN32
    FILE* process = _popen(
        "node \"./emitPlaybackBuffer.mjs\"",
        "rb"
    );
#else
    FILE* process = popen(
        "node \"./emitPlaybackBuffer.mjs\"",
        "r"
    );
#endif
    if (process == nullptr) {
        throw std::runtime_error(
            "Failed to get playback buffer."
        );
    }
    std::vector<std::uint8_t> buffer;
    std::array<std::uint8_t, 256> chunk;
    while (true) {
        const std::size_t count = std::fread(
            chunk.data(),
            1,
            chunk.size(),
            process
        );
        buffer.insert(
            buffer.end(),
            chunk.begin(),
            chunk.begin() +
                static_cast<std::ptrdiff_t>(count)
        );
        if (count < chunk.size()) {
            if (std::ferror(process) != 0) {
                pclose(process);
                throw std::runtime_error(
                    "Failed to read playback buffer."
                );
            }
            break;
        }
    }
#ifdef _WIN32
    const int status = _pclose(process);
#else
    const int status = pclose(process);
#endif
    if (status != 0) {
        throw std::runtime_error(
            "Playback compiler failed"
        );
    }
    return buffer;
}  // runPlaybackcompiler

int main()
{
    using score_audio_studio::playback_runtime::PlaybackRuntime;
    using score_audio_studio::dsp::PlaybackEvent;

    const std::vector<std::uint8_t> buffer =
        runPlaybackCompiler();

    PlaybackRuntime runtime;
    runtime.prepare(
        std::span<const std::uint8_t>(
            buffer.data(),
            buffer.size()
        )
    );

    std::uint32_t frameCount = 512;

    // tick / ppq * 60 / bpm * sampleRate = 24000
    std::uint32_t blockStartFrame = 24000;

    const std::span<const PlaybackEvent> firstBlock =
        runtime.eventsForBlock(0, frameCount);

    // The first note in `test-score.json` is a three-note chord
    // at sample position 0.
    assert(firstBlock.size() == 3);
    for (const PlaybackEvent& event : firstBlock) {
        assert(event.samplePosition == 0);
        assert(0.0f <= event.velocity);
        assert(event.velocity <= 1.0f);
    }

    // The next fixture event after sample position 0 is at `blockStartFrame`,
    // so the range [1, 1 + frameCount) contains no events.
    const std::span<const PlaybackEvent> noEvents =
        runtime.eventsForBlock(1, frameCount);
    assert(noEvents.empty());

    const std::span<const PlaybackEvent> secondBlock =
        runtime.eventsForBlock(blockStartFrame, 512);
    // noteOff pitches(3) + noteOn pitches(3) = 6
    assert(secondBlock.size() == 6);

    return 0;
}
