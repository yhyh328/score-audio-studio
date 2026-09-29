#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>

#include "score_audio_studio/dsp/AdsrEnvelope.hpp"

namespace score_audio_studio::dsp {

void AdsrEnvelope::setAttackFeatures(
    const double seconds,
    const float targetGain
) noexcept
{
    assert(0.0 <= seconds);
    assert
    (
        0.0f <= targetGain
        &&
        targetGain <= 1.0f
    );
    adsrSettings_[1] = {
        .seconds = seconds,
        .targetGain = targetGain
    };
    isSetFeatures_[1] = true;
}  // AdsrEnvelope::setAttackFeatures

void AdsrEnvelope::setDecayFeatures(
    const double seconds,
    const float targetGain
) noexcept
{
    assert
    (
        isSetFeatures_[1] && "Set Attack first"
    );
    assert(0.0 <= seconds);
    assert
    (
        0.0f <= targetGain
        &&
        targetGain <= 1.0f
    );
    adsrSettings_[2] = {
        .seconds = seconds,
        .targetGain = targetGain
    };
    isSetFeatures_[2] = true;
}  // AdsrEnvelope::setDecayFeatures

void AdsrEnvelope::setSustainFeatures(
    const double seconds
) noexcept
{
    assert
    (
        isSetFeatures_[2] && "Set Decay first"
    );
    assert(0.0 <= seconds);

    // sustain decay's gain
    const float sustainGain =\
        adsrSettings_[2].targetGain;

    StageFeatures features = {
        .seconds = seconds,
        .targetGain = sustainGain
    };
    adsrSettings_[3] = features;
    isSetFeatures_[3] = true;
}  // AdsrEnvelope::setSustainFeatures

void AdsrEnvelope::setReleaseFeatures(
    const double seconds
) noexcept
{
    assert
    (
        isSetFeatures_[3] && "Set Sustain first"
    );
    assert(0.0 <= seconds);
    StageFeatures features = {
        .seconds = seconds,
        .targetGain = 0.0f // always zero
    };
    adsrSettings_[4] = features;
    isSetFeatures_[4] = true;
}  // AdsrEnvelope::setReleaseFeatures

void AdsrEnvelope::prepare(
    const double sampleRate,
    const EnvelopeCurve curve
) noexcept
{
    assert
    (
        isSetFeatures_[4] && "Set Release first"
    );
    assert
    (
        0.0 < sampleRate
        &&
        std::isfinite(sampleRate)
    );
    sampleRate_ = sampleRate;

    if
    (
        curve != EnvelopeCurve::Linear
        &&
        curve != EnvelopeCurve::Exponential
    )
    {
        assert(false && "Invalid envelope curve");
    }
    curve_ = curve;
}  // AdsrEnvelope::prepare

void AdsrEnvelope::noteOn() noexcept
{
    setStage(EnvelopeStage::Attack);
}  // AdsrEnvelope::noteON

void AdsrEnvelope::setStage(
    const EnvelopeStage stage
) noexcept
{
    if (stage == EnvelopeStage::Idle) {
        reset();
        return;
    }
    stage_ = stage;
    elapsedSamples_ = 0;
    gainIncrement_ = 0.0f;
    std::size_t index = static_cast<size_t>(stage_);
    targetGain_ = adsrSettings_[index].targetGain;
}  // AdsrEnvelope::setStage

void AdsrEnvelope::process() noexcept
// process() gets called every (1 / sampleRate_) seconds.
{
    switch (stage_) {
        case EnvelopeStage::Idle:
            break;
        case EnvelopeStage::Attack:
            updateGain();
            break;
        case EnvelopeStage::Decay:
            updateGain();
            break;
        case EnvelopeStage::Sustain:
            break;
        case EnvelopeStage::Release:
            updateGain();
            break;
    }
}  // AdsrEnvelope::process

void AdsrEnvelope::updateGain() noexcept
{
    std::size_t index = static_cast<size_t>(stage_);

    bool isStageComplete = false;

    const auto [
        seconds, targetGain
    ] = adsrSettings_[index];
    targetGain_ = targetGain;

    if (seconds <= 0.0) {
        isStageComplete = true;
    }
    else {
        const std::uint64_t totalSamples =\
            std::llround(sampleRate_ * seconds);
        if (totalSamples == 0) {
            isStageComplete = true;
        }
        else {
            const std::uint64_t remainingSamples =\
                totalSamples - elapsedSamples_;

            const float gainDelta = targetGain_ - gain_;

            switch (curve_) {
                case EnvelopeCurve::None:
                    break;
                case EnvelopeCurve::Linear:
                    gainIncrement_ =\
                        gainDelta / static_cast<float>(remainingSamples);
                    break;
                case EnvelopeCurve::Exponential:
                    /**
                     * coeff_ is the filter coefficient
                     * per samples to close 99.9% of the gain gap.
                     */
                    const float coeff = static_cast<float>(
                        1.0 - std::exp(
                            -6.9 / static_cast<double>(totalSamples)
                        )
                    );
                    gainIncrement_ = coeff * gainDelta;
                    break;
            }
            gain_ += gainIncrement_;
            ++elapsedSamples_;

            if (totalSamples <= elapsedSamples_) {
                isStageComplete = true;
            }
        }
    }
    if (isStageComplete) {
        gain_ = targetGain_;
        EnvelopeStage nextStage =\
            static_cast<EnvelopeStage>((index + 1) % 5);
        setStage(nextStage);
    }

}  // AdsrEnvelope::updateGain

void AdsrEnvelope::noteOff() noexcept
{
    setStage(EnvelopeStage::Release);
}  // AdsrEnvelope::noteOff

void AdsrEnvelope::reset() noexcept
{
    stage_ = EnvelopeStage::Idle;
    elapsedSamples_ = 0;
    gain_ = 0.0f;
    targetGain_ = 0.0f;
    gainIncrement_ = 0.0f;
}  // AdsrEnvelope::reset

void AdsrEnvelope::clear() noexcept
{
    std::fill(
        std::begin(adsrSettings_),
        std::end(adsrSettings_),
        StageFeatures{}
    );
    std::fill(
        std::begin(isSetFeatures_),
        std::end(isSetFeatures_),
        false
    );
    reset();
}  // AdsrEnvelope::clear

float AdsrEnvelope::getGain() noexcept
{
    return gain_;
}  // AdsrEnvelope::getGain

}  // namespace score_audio_studio::dsp
