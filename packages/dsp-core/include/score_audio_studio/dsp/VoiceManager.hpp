#pragma once

#include <cstdint>
#include <vector>

#include "score_audio_studio/dsp/Voice.hpp"

namespace score_audio_studio::dsp {
/**
 * Manages the polyphonic voices of a single instrument instance.
 *
 * Independently controlled instruments or parts should use separate
 * VoiceManager instances.
 */
class VoiceManager
{
public:
    void prepare(
        const double sampleRate,
        const std::size_t maxVoices,
        const OscSettings& oscSettings,
        const AdsrSettings& adsrSettings
    ) noexcept;
    /**
     * noteOn() returns the voice key 
     * required to match the corresponding noteOff()
     */
    std::uint64_t noteOn(
        const std::uint8_t midiNoteNumber,
        const float velocity
    ) noexcept;
    void render(
        float* left,
        float* right,
        const std::uint32_t startFrame,
        const std::uint32_t endFrame
    ) noexcept;
    void noteOff(
        const std::uint64_t voiceKey
    ) noexcept;
    void reset() noexcept;
    
    [[nodiscard]] std::uint8_t activeVoiceCount() noexcept;
    
    bool isVoiceActive(
        const std::uint64_t voiceKey
    ) noexcept;

private:
    double sampleRate_{};
    std::size_t maxVoices_{};
    std::uint64_t voiceKey_{1};
    std::vector<Voice> voices_{};

    [[nodiscard]] Voice& allocate() noexcept;
    [[nodiscard]] Voice* findVoice(
        const std::uint64_t voiceKey
    ) noexcept;
};

} // namespace
