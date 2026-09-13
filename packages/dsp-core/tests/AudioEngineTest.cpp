#include <algorithm>
#include <cassert>
#include <cstdint>

#include "score_audio_studio/dsp/AudioEngine.hpp"

int main()
{
    using namespace score_audio_studio::dsp;

    constexpr double sampleRate = 48000.0;
    constexpr std::uint32_t maxBlockSize = 512;
    constexpr std::uint32_t maxVoices = 16;

    AudioEngine engine;

    engine.prepare(
        sampleRate,
        maxBlockSize,
        maxVoices
    );

    float left[maxBlockSize] = {0.0f};
    float right[maxBlockSize] = {0.0f};
    std::fill_n(left, maxBlockSize, -1.0f);
    std::fill_n(right, maxBlockSize, 1.0f);

    constexpr std::uint32_t frameCount = 512;
    constexpr std::uint32_t eventCount = 0;
    const PlaybackEvent* events = nullptr;

    engine.process(
        left,
        right,
        frameCount,
        events,
        eventCount
    );

    for (std::uint32_t i = 0; i < frameCount; ++i) {
        assert(left[i] == 0.0f);
        assert(right[i] == 0.0f);
    }

    // TODO: Implement after the required voice, event, and effect components are available.
    // engine.reset();

    return 0;
}
