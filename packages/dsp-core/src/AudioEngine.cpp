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
    const PlaybackEvent* events,
    std::uint32_t eventCount
) noexcept
{
#ifndef NDEBUG
    assert(frameCount <= maxBlockSize_);
    assert(frameCount == 0 || (left != nullptr && right != nullptr));
    assert(eventCount == 0 || events != nullptr);
    if (0 < eventCount) {
        assert(events[0].sampleOffset < frameCount);
    }
    for (std::size_t i = 1; i < eventCount; i++) {
        std::uint32_t prevOffset = events[i - 1].sampleOffset;
        std::uint32_t currOffset = events[i].sampleOffset;
        assert(prevOffset <= currOffset);
        assert(currOffset < frameCount);
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
