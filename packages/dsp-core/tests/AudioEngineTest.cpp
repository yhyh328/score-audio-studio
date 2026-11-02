#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "score_audio_studio/dsp/AudioEngine.hpp"

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
    using namespace score_audio_studio::dsp;

    constexpr double sampleRate = 48000.0;
    constexpr std::uint32_t maxBlockSize = 512;
    constexpr float masterGain = 1.0f;
    
    const std::vector<std::uint8_t> buffer =
        runPlaybackCompiler();

    assert(!buffer.empty() && "playback buffer is empty");

    AudioEngine engine;

    engine.prepare(
        sampleRate,
        maxBlockSize,
        masterGain,
        buffer
    );

    float left[maxBlockSize] = {0.0f};
    float right[maxBlockSize] = {0.0f};
    std::fill_n(left, maxBlockSize, -1.0f);
    std::fill_n(right, maxBlockSize, 1.0f);

    constexpr std::uint32_t frameCount = 512;
    constexpr std::uint32_t blockStartFrame = 0;

    engine.process(
        left,
        right,
        frameCount,
        blockStartFrame
    );

    bool hasNonZeroSample = false;
    for (std::uint32_t i = 0; i < frameCount; ++i) {
        assert(std::isfinite(left[i]) && "left channel sample is not finite");
        assert(std::isfinite(right[i]) && "right channel sample is not finite");
        if (left[i] != 0.0f || right[i] != 0.0f) {
            hasNonZeroSample = true;
        }
    }
    assert(hasNonZeroSample && "output buffer is silent");

    engine.reset();

    return 0;
}
