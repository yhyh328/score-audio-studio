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
    elapsedSeconds_ = 0.0;
    updateGain();
}  // AdsrEnvelope::setStage

void AdsrEnvelope::process() noexcept
// process() gets called every (1 / sampleRate_) seconds.
{
    switch (stage_) {
        case EnvelopeStage::Idle:
            break;
        case EnvelopeStage::Attack:
            updateGain();
            if (targetGain_ <= gain_) {
                setStage(EnvelopeStage::Decay);
            }
            break;
        case EnvelopeStage::Decay:
            updateGain();
            if (gain_ <= targetGain_) {
                setStage(EnvelopeStage::Sustain);
            }
            break;
        case EnvelopeStage::Sustain:
            break;
        case EnvelopeStage::Release:
            updateGain();
            if (gain_ <= targetGain_) {
                setStage(EnvelopeStage::Idle);
            }
            break;
    }
}  // AdsrEnvelope::process

void AdsrEnvelope::updateGain() noexcept
{
    std::size_t index = static_cast<size_t>(stage_);

    const auto [
        seconds, targetGain
    ] = adsrSettings_[index];
    targetGain_ = targetGain;

    float remainingSeconds = seconds - elapsedSeconds_;

    // Avoid divison by zero
    if (remainingSeconds <= 0.0f || seconds <= 0.0f) {
        gain_ = targetGain_;
        gainIncrement_ = 0.0f;
        return;
    }

    const float samples = std::max(
        1.0f, static_cast<float>(sampleRate_ * remainingSeconds)
    );

    const float gainDelta = targetGain_ - gain_;

    switch (curve_) {
        case EnvelopeCurve::None:
            break;
        case EnvelopeCurve::Linear:
            gainIncrement_ = gainDelta / samples;
            break;
        case EnvelopeCurve::Exponential:
            /**
             * coeff_ is the filter coefficient
             * per samples to close 99.9% of the gain gap.
             */
            const float coeff = static_cast<float>(
                1.0 - std::exp(-6.9 / samples)
            );
            gainIncrement_ = coeff * gainDelta;
            break;
    }
    gain_ += gainIncrement_;
    elapsedSeconds_ += 1.0 / sampleRate_;

    // prevent overshoot
    gain_ = (0.0f < gainIncrement_) ? std::min(gain_, targetGain_)
                                    : std::max(gain_, targetGain_);
}  // AdsrEnvelope::updateGain

void AdsrEnvelope::noteOff() noexcept
{
    setStage(EnvelopeStage::Release);
}  // AdsrEnvelope::noteOff

void AdsrEnvelope::reset() noexcept
{
    stage_ = EnvelopeStage::Idle;
    elapsedSeconds_ = 0.0;
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
