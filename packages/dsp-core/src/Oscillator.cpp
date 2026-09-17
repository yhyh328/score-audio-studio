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

    if (type != WaveType::Sine) {
        // check if user wants more harmonics 
        // than the minimum required for the wave type
        numHarmonics_ = std::max(numHarmonics, numHarmonics_);
    }
    // check if user's harmonics exceed the maximum allowed
    numHarmonics_ = std::min(numHarmonics_, kMaxHarmsNum_);

    #ifndef NDEBUG
    // verify sine wave
    if (type_ == WaveType::Sine || numHarmonics_ == 1) {
        assert(type_ == WaveType::Sine || numHarmonics_ == 1);
    }
    // verify triangle and sqaure waves
    else if (type_ == WaveType::Triangle || type_ == WaveType::Square) {
        assert(3 <= numHarmonics_  && numHarmonics_  <= kMaxHarmsNum_);
    }
    // verify sawtooth wave
    else if (type_ == WaveType::Sawtooth) {
        assert(2 <= numHarmonics_  && numHarmonics_  <= kMaxHarmsNum_);
    }
    #endif
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
    #ifndef NDEBUG
    if (!allowAliasing_) {
        assert(frequency_ <= sampleRate_ / 2.0);
    }
    #endif
}  // Oscillator::setFrequency

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
        std::uint8_t harmonicNum = i + 1;
        double harmonicPhase = phase_ * static_cast<double>(harmonicNum);
        double harmonicSample = std::sin(2 * std::numbers::pi * harmonicPhase);
        sample += harmonicAmps_[i] * harmonicSample;
    }
    wrapPhase();
    return sample;
}  // Oscillator::renderSample

void Oscillator::wrapPhase() noexcept
{
    const double phaseIncrement = frequency_ / sampleRate_;
    double nextPhase = phase_ + phaseIncrement;
    nextPhase = (1.0 <= nextPhase) ? (nextPhase - 1.0) : nextPhase;
    phase_ = nextPhase;
}

void Oscillator::reset() noexcept
{
    type_ = WaveType::Sine;
    sampleRate_ = 0.0 / 0.0;
    frequency_ = 0.0 /0.0;
    phase_ = 0.0;

    harmonicAmps_.fill(0.0);

    // default to 1 for sine wave
    numHarmonics_ = 1;
    
    // disallows aliasing experiments initially.
    allowAliasing_ = false;
}  // Oscillator::reset

}  // namespace score_audio_studio::dsp
