#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "score_audio_studio/dsp/PlaybackEvent.hpp"

namespace score_audio_studio::playback_runtime {

using PlaybackEvent =
    score_audio_studio::dsp::PlaybackEvent;

class PlaybackRuntime
{
public:
    void prepare(
        std::span<const std::uint8_t> buffer
    );
    std::span<const PlaybackEvent> eventsForBlock(
        std::uint32_t blockStartFrame,
        std::uint32_t frameCount
    ) noexcept;
private:
    double sampleRate_{};
    std::vector<PlaybackEvent> playbackEvents_{};
    std::size_t nextEventIndex_{};
};

}  // namespace
