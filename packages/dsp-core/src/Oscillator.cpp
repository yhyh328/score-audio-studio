#include <algorithm>
#include <cassert>
#include <numbers>

#include "score_audio_studio/dsp/Oscillator.hpp"

namespace score_audio_studio::dsp {

void Oscillator::prepare(
    const double sampleRate
) noexcept
{
    assert(0 < sampleRate &&std::isfinite(sampleRate));
    sampleRate_ = sampleRate;
}  // Oscillator::prepare

void Oscillator::setWaveType(
    const WaveType type,
    const std::uint8_t numHarmonics
) noexcept
{
    // if user designates "type" and misses "numHarmonics"
    switch(type) {
        case WaveType::Sine:
            // number of harmonics for sine wave is only 1
            numHarmonics_ = 1;
            break;
        case WaveType::Triangle:
            // minimum number of harmonics for triangle wave is 3, 
            // and only odd harmonics are present
            numHarmonics_ = 3;
            break;
        case WaveType::Square:
            // minimum number of harmonics for square wave is 3, 
            // and only odd harmonics are present
            numHarmonics_ = 3;
            break;
        case WaveType::Sawtooth:
            // minimum number of harmonics for sawtooth wave is 2, 
            // and all harmonics are present
            numHarmonics_ = 2;
            break;
        default:
            #ifndef NDEBUG
            assert(false && "Invalid wave type");
            #endif
            // default to sine wave if invalid wave type is provided
            type_ = WaveType::Sine;
            numHarmonics_ = 1;
            break;
    }

    type_ = type;

    if (type_ != WaveType::Sine) {
        // check if user wants more harmonics 
        // than the minimum required for the wave type
        numHarmonics_ = std::max(numHarmonics, numHarmonics_);
    }
    // check if user's harmonics exceed the maximum allowed
    numHarmonics_ = std::min(numHarmonics_, kMaxHarmsNum_);

    calcHarmonicAmps();
} // Oscillator::setWaveType

void Oscillator::setFrequency(
    std::uint8_t midiNoteNumber
) noexcept
{
    assert(midiNoteNumber <= 127);
    frequency_ = 440.0 * std::pow(
        2.0, (static_cast<double>(midiNoteNumber) - 69.0) / 12.0
    );
}  // Oscillator::setFrequency

double Oscillator::getFrequency() noexcept
{
    return frequency_;
}  // Oscillator::getFrequency

void Oscillator::calcHarmonicAmps() noexcept
{
    std::uint8_t i, j;
    double gain = 1.0;
    switch (type_) {
        case WaveType::Sine:
            harmonicAmps_[0] = gain;
            break;
        case WaveType::Triangle:
            for (i = 0; i < numHarmonics_; i++) {
                j = i + 1;
                if (j % 2 == 0) {
                    harmonicAmps_[i] = 0.0;
                    continue;
                }
                harmonicAmps_[i] = gain / (j * j);
                gain *= -1.0;
            }
            break;
        case WaveType::Square:
            for (i = 0; i < numHarmonics_; i++) {
                j = i + 1;
                if (j % 2 == 0) {
                    harmonicAmps_[i] = 0.0;
                    continue;
                }
                harmonicAmps_[i] = gain / j;
            }
            break;
        case WaveType::Sawtooth:
            for (i = 0; i < numHarmonics_; i++) {
                j = i + 1;
                harmonicAmps_[i] = gain / j;
            }
            break;
        default:
            #ifndef NDEBUG
            assert(false && "Invalid wave type");
            #endif
            // default to sine wave if invalid wave type is provided
            harmonicAmps_[0] = gain;
            break;
    }
}  // Oscillator::calcHarmonicAmps

double Oscillator::renderSample() noexcept
{
    double sample = 0.0;

    for (std::uint8_t i = 0; i < numHarmonics_; ++i) {
        double harmonicNum = static_cast<double>(i) + 1.0;
        if (!allowAliasing_) {
            double harmonicFrequency = frequency_ * harmonicNum;
            if (harmonicFrequency > sampleRate_ / 2.0) {
                break;
            }
        }
        double harmonicPhase = phase_ * harmonicNum;
        double harmonicSample = std::sin(2 * std::numbers::pi * harmonicPhase);
        sample += harmonicAmps_[i] * harmonicSample;
    }
    wrapPhase();
    return sample;
}  // Oscillator::renderSample

void Oscillator::wrapPhase() noexcept
{
    // Keep phase within [0.0, 1.0) even if more than one cycle is crossed.
    phase_ += frequency_ / sampleRate_;
    phase_ -= std::floor(phase_);
}  // Oscillator::wrapPhase

void Oscillator::reset() noexcept
{
    phase_ = 0.0;
    allowAliasing_ = false;
}  // Oscillator::reset

void Oscillator::setAllowAliasing(bool enabled) noexcept
{
    allowAliasing_ = enabled;
}  // Oscillator::setAllowAliasing

}  // namespace score_audio_studio::dsp
