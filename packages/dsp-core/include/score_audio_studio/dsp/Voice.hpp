#pragma once

#include <cstdint>

#include "score_audio_studio/dsp/Oscillator.hpp"
#include "score_audio_studio/dsp/AdsrEnvelope.hpp"

namespace score_audio_studio::dsp {

struct OscSettings
{   // Sine default: one active harmonic.
    WaveType waveType{WaveType::Sine};
    std::uint8_t numHarmonics{1};
    bool allowAliasing{false};
};
struct AdsrSettings
{
    StageFeatures attack {
        .seconds = 0.0,
        .targetGain = 1.0f
    };
    StageFeatures decay {
        .seconds = 0.0,
        .targetGain = 1.0f
    };
    double sustainSeconds{0.0};
    double releaseSeconds{0.0};
    EnvelopeCurve curve{EnvelopeCurve::Linear};
};

class Voice {

public:
    void prepare(
        const double SampleRate,
        const OscSettings& oscSettings,
        const AdsrSettings& adsrSettings
    ) noexcept;
    void noteOn(
        const std::uint64_t key,
        const std::uint8_t midiNoteNumber,
        const float velocity
    ) noexcept;
    void render(
        float* left,
        float* right,
        std::uint32_t startFrame,
        std::uint32_t endFrame
    ) noexcept;
    void noteOff() noexcept;
    void reset() noexcept;

    [[nodiscard]] bool isActive() const noexcept;
    [[nodiscard]] bool isReleasing() const noexcept;
    [[nodiscard]] float getLevel() const noexcept;
    [[nodiscard]] std::uint64_t getKey() const noexcept;

private:
    /**
     * Used to match NoteOn and NoteOff events.
     *
     * The key identifies the note currently assigned to this voice.
     * Key generation and voice allocation are managed by VoiceManager.
     */
    std::uint64_t key_{0};

    Oscillator osc_;
    AdsrEnvelope adsr_;

    float velocity_{0.0f};

    /**
     * "active_" becomes true, since this voice is playing.
     *
     * "releasing_" becomes true, since this voice is releasing.
     *
     * If the release stage reaches the end,
     * both "active_" and "releasing_" become false.
     */
    bool active_{false};
    bool releasing_{false};
};

}  // namespace score_audio_studio::dsp