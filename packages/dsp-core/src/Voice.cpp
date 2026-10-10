#include <cassert>

#include "score_audio_studio/dsp/Voice.hpp"

namespace score_audio_studio::dsp {

void Voice::prepare(
    const double SampleRate,
    const OscSettings& oscSettings,
    const AdsrSettings& adsrSettings
) noexcept
{
    osc_.prepare(
        SampleRate
    );
    osc_.setWaveType(
        oscSettings.waveType,
        oscSettings.numHarmonics
    );
    osc_.setAllowAliasing(
        oscSettings.allowAliasing
    );
    adsr_.setAttackFeatures(
        adsrSettings.attack.seconds,
        adsrSettings.attack.targetGain
    );
    adsr_.setDecayFeatures(
        adsrSettings.decay.seconds,
        adsrSettings.decay.targetGain
    );
    adsr_.setSustainFeatures(
        adsrSettings.sustainSeconds
    );
    adsr_.setReleaseFeatures(
        adsrSettings.releaseSeconds
    );
    adsr_.prepare(
        SampleRate,
        adsrSettings.curve
    );
}  // Voice::prepare

void Voice::noteOn(
    std::uint64_t key,
    std::uint8_t midiNoteNumber,
    float velocity
) noexcept
{
    if (active_ || key == 0) {
        assert(false && "Voice is already active or key is invalid");
        return;
    }
    assert(0.0f <= velocity && velocity <= 1.0f);
    key_ = key;
    velocity_ = velocity;
    osc_.setFrequency(midiNoteNumber);
    adsr_.noteOn();
    active_ = true;
}  // Voice::noteOn

void Voice::render(
    float* left,
    float* right,
    std::uint32_t startFrame,
    std::uint32_t endFrame
) noexcept
{
    if (!active_) return;
    for (std::uint32_t i = startFrame; i < endFrame; ++i) {
        adsr_.process();
        if (adsr_.isIdle()) {
            #ifndef NDEBUG
            assert(releasing_);
            #endif
            reset();
            return;
        }
        const float sample = static_cast<float>(
            osc_.renderSample() * adsr_.getGain() * velocity_
        );
        left[i] += sample;
        right[i] += sample;
    }
}  // Voice::render

void Voice::noteOff() noexcept
{
    if (!active_ || releasing_) {
        assert(false && "Voice is inactive or already releasing");
        return;
    }
    adsr_.noteOff();
    if (adsr_.isIdle()) {
        reset();
        return;
    }
    releasing_ = true;
}  // Voice::noteOff

void Voice::reset() noexcept
{
    key_ = 0;
    velocity_ = 0.0f;
    osc_.reset();
    adsr_.reset();
    active_ = false;
    releasing_ = false;
}  // Voice::reset

bool Voice::isActive() const noexcept
{
    return active_;
}  // Voice::isActive

bool Voice::isReleasing() const noexcept
{
    return releasing_;
}  // Voice::isReleasing

float Voice::getLevel() const noexcept
{
    return adsr_.getGain();
}  // Voice::getLevel

std::uint64_t Voice::getKey() const noexcept
{
    return key_;
}  // Voice::getKey

}  // namespace
