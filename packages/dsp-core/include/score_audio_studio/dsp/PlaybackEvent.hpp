#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace score_audio_studio::dsp {

enum class PlaybackEventType : std::uint8_t {
    /**
     * Matches the Phase 2 tie-break policy:
     * noteOff precedes noteOn at the same sample offset.
     */
    noteOff,
    noteOn
};
/**
 * PlaybackEvent is adapted from TickPlaybackEvent in the package "playback-compiler"
 * by a runtime adapter before being consumed by DSP Core.
 */
struct PlaybackEvent {
    PlaybackEventType type;
    std::uint8_t midiNoteNumber;
    std::uint16_t partId;
    std::uint32_t noteId;
    std::uint32_t sampleOffset;
    float velocity;
};

// Binary layout contract for the playback-runtime boundary
static_assert(
    std::is_trivially_copyable_v<PlaybackEvent>
);
static_assert(
    std::is_standard_layout_v<PlaybackEvent>
);
static_assert(
    offsetof(PlaybackEvent, type) == 0
);
static_assert(
    offsetof(PlaybackEvent, midiNoteNumber) == 1
);
static_assert(
    offsetof(PlaybackEvent, partId) == 2
);
static_assert(
    offsetof(PlaybackEvent, noteId) == 4
);
static_assert(
    offsetof(PlaybackEvent, sampleOffset) == 8
);
static_assert(
    offsetof(PlaybackEvent, velocity) == 12
);
static_assert(
    sizeof(PlaybackEvent) == 16
);
static_assert(
    alignof(PlaybackEvent) == 4
);
static_assert(
    std::endian::native == std::endian::little
);

}  // namespace score_audio_studio::dsp
