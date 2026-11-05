#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "score_audio_studio/playback_runtime/PlaybackEvent.hpp"
#include "score_audio_studio/playback_runtime/PlaybackRuntime.hpp"
#include "score_audio_studio/dsp/Instruments.hpp"

namespace score_audio_studio::dsp {

class AudioEngine {

using PlaybackRuntime =
    score_audio_studio::playback_runtime::PlaybackRuntime;

using PlaybackEvent =
    score_audio_studio::playback_runtime::PlaybackEvent;

public:
    void prepare(
        const double sampleRate,
        const std::uint32_t maxBlockSize,
        const float masterGain,
        const std::vector<std::uint8_t> buffer
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
        const std::uint32_t frameCount,
        const std::uint32_t blockStartFrame
    ) noexcept;

    double getSampleRate() const noexcept;
    std::uint32_t getMaxBlockSize() const noexcept;

private:
    /**
     * Static engine configuration values that do not change during playback.
     */
    double sampleRate_;
    std::uint32_t maxBlockSize_;
    /**
     * DSP processing components and persistent runtime state.
     */
    PlaybackRuntime runtime_;
    std::vector<Instrument*> instruments_{};
    /**
     * Runtime parameters that may change during playback.
     */
    float masterGain_{};
#if 0 // TODO: Implement after the required voice, event, and effect components are available.
    EffectChain effectChain_;
#endif
    void setMasterGain(
        const float masterGain
    ) noexcept;
    /**
     * Preparation helpers.
     */
    void loadPlaybackEvents(
        std::span<const std::uint8_t> buffer
    );
    void setInstruments(
        const std::size_t channelSize
    ) noexcept;
    /**
     * Audio processing helpers.
     */
    void renderVoices(
        float* left,
        float* right,
        const std::uint32_t startFrame,
        const std::uint32_t endFrame
    ) noexcept;
    void applyPlaybackEvent(
        const PlaybackEvent& event
    ) noexcept;
};

}  // namespace score_audio_studio::dsp
