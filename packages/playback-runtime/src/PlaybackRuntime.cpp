#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstring>
#include <stdexcept>

#include "score_audio_studio/playback_runtime/PlaybackRuntime.hpp"

namespace score_audio_studio::playback_runtime {

void PlaybackRuntime::prepare(
    std::span<const std::uint8_t> buffer
)
{
    // velocity should be 32 bits
    static_assert(sizeof(float) == 4);
    // sample rate should be 64 bits
    static_assert(sizeof(double) == 8);
    static_assert(
        std::endian::native == std::endian::little
    );

    constexpr std::size_t headerSizeExpected = 24;
    if (buffer.size() < headerSizeExpected) {
        throw std::runtime_error(
            "Playback buffer is smaller than header"
        );
    }

    if (
        buffer[0] != 0x53 || // S
        buffer[1] != 0x41 || // A
        buffer[2] != 0x53 || // S
        buffer[3] != 0x42    // B
    ) {
        throw std::runtime_error("Invalid magic");
    }

    const std::uint16_t version =
        static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(buffer[4])
            |
            static_cast<std::uint32_t>(buffer[5]) << 8
        );
    constexpr std::array supportedVersions{
        std::uint16_t{1}
    };
    if (
        std::ranges::find(
            supportedVersions,
            version
        ) == supportedVersions.end()
    ) {
        throw std::runtime_error("Invalid version");
    }

    const std::uint16_t headerSize =
        static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(buffer[6])
            |
            static_cast<std::uint32_t>(buffer[7]) << 8
        );
    if (headerSize != 24) {
        throw std::runtime_error("Invalid header size");
    }

    double sampleRate{};
    std::memcpy(&sampleRate, buffer.data() + 8, 8);
    if (sampleRate <= 0 || !std::isfinite(sampleRate)) {
        throw std::runtime_error("Invalid sample rate");
    }
    sampleRate_ = sampleRate;

    const std::uint32_t eventCount =
        static_cast<std::uint32_t>(buffer[16])
        |
        static_cast<std::uint32_t>(buffer[17]) << 8
        |
        static_cast<std::uint32_t>(buffer[18]) << 16
        |
        static_cast<std::uint32_t>(buffer[19]) << 24;

    const std::uint32_t eventSize =
        static_cast<std::uint32_t>(buffer[20])
        |
        static_cast<std::uint32_t>(buffer[21]) << 8
        |
        static_cast<std::uint32_t>(buffer[22]) << 16
        |
        static_cast<std::uint32_t>(buffer[23]) << 24;
    if (eventSize != sizeof(PlaybackEvent)) {
        throw std::runtime_error("Invalid event size");
    }

    const std::uint64_t expectedBufferSize =
        static_cast<std::uint64_t>(headerSize) +
        static_cast<std::uint64_t>(eventCount) *
        static_cast<std::uint64_t>(eventSize);
    if (
        static_cast<std::uint64_t>(buffer.size())
        !=
        expectedBufferSize
    ) {
        throw std::runtime_error("Invalid buffer");
    }

    playbackEvents_.resize(eventCount);

    const std::size_t payloadSize =
        static_cast<std::size_t>(eventCount)
        *
        static_cast<std::size_t>(eventSize);

    if (payloadSize != 0) {
        std::memcpy(
            playbackEvents_.data(),
            buffer.data() + headerSize,
            payloadSize
        );
    }
}  // PlaybackRuntime::prepare

std::span<const PlaybackEvent>PlaybackRuntime::eventsForBlock(
    std::uint32_t blockStartFrame,
    std::uint32_t frameCount
) noexcept
{
    while (
        nextEventIndex_ < playbackEvents_.size()
        &&
        (
            playbackEvents_[nextEventIndex_].samplePosition
            <
            blockStartFrame
        )
    ) {
        ++nextEventIndex_;
    }

    const std::size_t firstEventIndex = nextEventIndex_;

    while (
        nextEventIndex_ < playbackEvents_.size()
    ) {
        const std::uint32_t samplePosition =
            playbackEvents_[nextEventIndex_].samplePosition;
        const std::uint32_t sampleOffset =
            samplePosition - blockStartFrame;
        if (frameCount <= sampleOffset) {
            break;
        }
        ++nextEventIndex_;
    }

    const std::span<const PlaybackEvent> allEvents {
        playbackEvents_.data(),
        playbackEvents_.size()
    };

    return allEvents.subspan(
        firstEventIndex,
        nextEventIndex_ - firstEventIndex
    );
}  // PlaybackRuntime::eventsForBlock

}  // namespace
