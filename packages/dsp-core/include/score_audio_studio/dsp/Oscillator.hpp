#pragma once

#include <cmath>
#include <cstdint>
#include <array>
#include <limits>

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
        const double sampleRate
    ) noexcept;
    void setFrequency(
        const std::uint8_t midiNoteNumber
    ) noexcept;
    void setWaveType(
        const WaveType type = WaveType::Sine,
        const std::uint8_t numHarmonics = 1
    ) noexcept;
    double renderSample() noexcept;
    void reset() noexcept;
    void setAllowAliasing(bool enabled) noexcept;

private:
    static constexpr std::uint8_t kMaxHarmsNum_ = 64;

    void calcHarmonicAmps() noexcept;
    void wrapPhase() noexcept;

    WaveType type_{WaveType::Sine};

    double sampleRate_{std::numeric_limits<double>::quiet_NaN()};
    double frequency_{std::numeric_limits<double>::quiet_NaN()};
    double phase_{0.0};

    // Sine default: fundamental amplitude 1.0, all others 0.0.
    std::array<double, kMaxHarmsNum_> harmonicAmps_{1.0};

    // Sine default: one active harmonic.
    std::uint8_t numHarmonics_{1};

    // Allows frequencies above Nyquist for aliasing experiments.
    bool allowAliasing_{false};
};

}  // namespace score_audio_studio::dsp
