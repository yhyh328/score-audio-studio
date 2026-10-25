#pragma once

#include <cstdint>
#include <span>

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
     * 1. Every PlaybackEvent::samplePosition must be greater than or equal to
     *    blockStartFrame.
     * 2. For every event,
     *    PlaybackEvent::samplePosition - blockStartFrame must be less than
     *    frameCount.
     * 3. Events must be sorted in ascending order by samplePosition.
     * 4. Events with the same samplePosition must be in deterministic order.
     * 5. frameCount must not exceed the configured maximum block size.
     * 6. If 0 < frameCount, left and right must point to valid writable buffers.
     *    If frameCount == 0, left and right may be null.
     * 7. If 0 < eventCount, events must point to a valid event array.
     * 8. This function does not allocate, block, or throw.
     */
    void process(
        float* left,
        float* right,
        std::uint32_t frameCount,
        std::uint32_t blockStartFrame,
        std::span<const PlaybackEvent> events
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
