#pragma once

#include <cstdint>
#include <cstddef>
#include <limits>

namespace score_audio_studio::dsp {

enum class EnvelopeStage
{
    Idle,   // silent state
    Attack, // noteOn
    Decay,  // approaching sustain gain
    Sustain,// holding sustain gain
    Release // noteOff
};

enum class EnvelopeCurve
{
    None,
    Linear,
    Exponential
};

/**
 * The stage begins approaching the target gain immediately
 * and reaches it within the specified duration.
 */
struct StageFeatures
{
    double seconds;
    float targetGain;
};

class AdsrEnvelope
{
public:
    void setAttackFeatures(
        const double seconds,
        const float targetGain
    ) noexcept;
    void setDecayFeatures(
        const double seconds,
        const float targetGain
    ) noexcept;
    void setSustainFeatures(
        const double seconds
    ) noexcept;
    void setReleaseFeatures(
        const double seconds
    ) noexcept;

    void prepare(
        const double sampleRate,
        const EnvelopeCurve curve
    ) noexcept;

    void noteOn() noexcept;  // Starts the Attack stage.
    void process() noexcept; // Advances the envelope by one sample.
    void noteOff() noexcept; // Starts the Release stage.

    // Resets the envelope to the idle state while preserving its settings.
    void reset() noexcept;
    // Clears all attack, decay, sustain, and release settings.
    void clear() noexcept;

    [[nodiscard]] float getGain() noexcept;
    [[nodiscard]] bool isIdle() noexcept;

private:
    StageFeatures adsrSettings_[5]{};

    // The number of samples for each stage.
    std::uint32_t adsrSamples_[5]{};
    
    bool isSetFeatures_[5]{};

    double sampleRate_{
        std::numeric_limits<double>::quiet_NaN()
    };

    EnvelopeCurve curve_{
        EnvelopeCurve::None
    };
    EnvelopeStage stage_{
        EnvelopeStage::Idle
    };

    std::uint32_t elapsedSamples_{};

    float gain_{};          // Current envelope gain
    float targetGain_{};    // Destination gain
    float gainIncrement_{}; // Gain added per sample

    void setStage(
        const EnvelopeStage stage
    ) noexcept;
    void skipZeroSampleStage(
        const std::size_t index
    ) noexcept;
    void updateGain() noexcept;
};

}  // namespace score_audio_studio::dsp
