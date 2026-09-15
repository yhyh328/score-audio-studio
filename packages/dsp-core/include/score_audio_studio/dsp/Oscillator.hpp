#pragma once

#include <cmath>
#include <cstdint>
#include <numbers>
#include <array>

namespace score_audio_studio::dsp {

enum class WaveType {
    Sine,
    Triangle,
    Square,
    Sawtooth
};

class Oscillator {

public:
    void prepare(
        double sampleRate,
        WaveType type = WaveType::Sine,
        std::uint8_t numHarmonics = 1
    ) noexcept;
    void setFrequency(
        std::uint8_t midiNoteNumber
    ) noexcept;
    double renderSample() noexcept;

private:
    static constexpr std::uint8_t kMaxHarmsNum_ = 64;
    
    void calcHarmonicAmps() noexcept;
    
    WaveType type_{WaveType::Sine};
   
    double sampleRate_;
    double frequency_;
    double phase_;
  
    std::array<double, kMaxHarmsNum_> harmonicAmps_;
    
    // default to 1 for sine wave
    std::uint8_t numHarmonics_{1};

    // Allows frequencies above Nyquist for aliasing experiments.
    bool allowAliasing_{false}; 
};

}  // namespace score_audio_studio::dsp
