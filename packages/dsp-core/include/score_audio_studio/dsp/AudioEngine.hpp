#pragma once

#include <cstdint>

#include "score_audio_studio/dsp/PlaybackEvent.hpp"

namespace score_audio_studio::dsp {

class AudioEngine {

public:
    void prepare(
        double sampleRate,
        std::uint32_t maxBlockSize,
        std::uint32_t maxVoices
    );
    void reset() noexcept;
    /**
     * Process contract:
     *
     * 1. Every PlaybackEvent::sampleOffset must be less than frameCount.
     * 2. Events must be sorted in ascending order by sampleOffset.
     * 3. Events with the same sampleOffset must be in deterministic order.
     * 4. frameCount must not exceed the configured maximum block size.
     * 5. If 0 < frameCount, left and right must point to valid writable buffers.
     *    If frameCount == 0, left and right may be null.
     * 6. If 0 < eventCount, events must point to a valid event array.
     * 7. This function does not allocate, block, or throw.
     */
    void process(
        float* left,
        float* right,
        std::uint32_t frameCount,
        const PlaybackEvent* events,
        std::uint32_t eventCount
    ) noexcept;

private:
    /**
     * Static engine configuration values that do not change during playback.
     */
    double sampleRate_;
    std::uint32_t maxBlockSize_;
    std::uint32_t maxVoices_;
    /**
     * DSP processing components and persistent runtime state.
     */
#if 0 // TODO: Implement after the required voice, event, and effect components are available.
    VoiceManager voiceManager_;
    EffectChain effectChain_;
#endif
};

}  // namespace score_audio_studio::dsp
