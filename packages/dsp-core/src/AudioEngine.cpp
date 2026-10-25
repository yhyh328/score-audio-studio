#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "score_audio_studio/dsp/AudioEngine.hpp"

namespace score_audio_studio::dsp {

void AudioEngine::prepare(
    double sampleRate,
    std::uint32_t maxBlockSize,
    std::uint32_t maxVoices
)
{
    assert(std::isfinite(sampleRate));
    assert(0 < sampleRate);
    sampleRate_ = sampleRate;

    assert(0 < maxBlockSize);
    maxBlockSize_ = maxBlockSize;

    assert(0 < maxVoices);
    maxVoices_ = maxVoices;
    
}  // AudioEngine::prepare

void AudioEngine::reset() noexcept
{
    // TODO: Implement after the required voice, event, and effect components are available.
}  // AudioEngine::reset

void AudioEngine::process(
    float* left,
    float* right,
    std::uint32_t frameCount,
    std::uint32_t blockStartFrame,
    std::span<const PlaybackEvent> events
) noexcept
{
    std::size_t i = 0;
    std::uint32_t sampleOffset;
#ifndef NDEBUG
    assert(frameCount <= maxBlockSize_);
    assert(frameCount == 0 || (left != nullptr && right != nullptr));
    for (i = 0; i < events.size(); i++) {
        const PlaybackEvent& event = events[i];
        assert(blockStartFrame <= event.samplePosition);
        sampleOffset = event.samplePosition - blockStartFrame;
        assert(sampleOffset < frameCount);
        if (0 < i) {
            assert(
                events[i - 1].samplePosition
                <=
                event.samplePosition
            );
        }
    }
#endif
    if (0 < frameCount) {
        std::fill_n(left, frameCount, 0.0f);
        std::fill_n(right, frameCount, 0.0f);
    }
#if 0 // TODO: Implement after the required voice, event, and effect components are available.
    std::uint32_t cursor = 0;
    for (std::size_t i = 0; i < eventCount; i++) {
        std::uint32_t sampleOffset = events[i].sampleOffset;
        renderVoices(cursor, sampleOffset);
        applyPlaybackEvent();
        cursor = sampleOffset;
    }
    renderVoices(cursor, frameCount);
    processEffectChain(left, right, frameCount);
#endif
}  // AudioEngine::process

}  // namespace score_audio_studio::dsp
