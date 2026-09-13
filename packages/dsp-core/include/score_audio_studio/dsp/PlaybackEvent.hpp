#pragma once

#include <cstdint>

namespace score_audio_studio::dsp {

enum class PlaybackEventType : std::uint8_t {
    noteOn,
    noteOff
};
/**
 * PlaybackEvent is adapted from TickPlaybackEvent in the package "playback-compiler"
 * by a runtime adapter before being consumed by DSP Core.
 */
struct PlaybackEvent {
    PlaybackEventType type;
    std::uint32_t sampleOffset;
    std::uint32_t voiceKey;
    std::uint8_t midiNoteNumber;
    float velocity;
};

}  // namespace score_audio_studio::dsp
