#include <iostream>
#include <cassert>

#include "score_audio_studio/dsp/Oscillator.hpp"

int main()
{
    using namespace score_audio_studio::dsp;
    /**
     * phaseIncrement = frequency / sampleRate
     *                = 220.0 / 56320.0 = 1.0 / 256.0
     * There are 256 samples per a cycle.
     */
    constexpr double sampleRate = 56320.0;
    constexpr std::uint8_t midiNoteNumber = 57; // A3
    /**
     * Nyquist frequency = 48000.0 / 2.0 = 24000.0 Hz.
     *
     * For A4 = 440 Hz:
     * - 53rd harmonic = 23320 Hz (below Nyquist)
     * - 54th harmonic = 23760 Hz (below Nyquist)
     * - 55th harmonic = 24200 Hz (above Nyquist)
     *
     * Sawtooth uses both odd and even harmonics.
     * Triangle and Square use odd harmonics only.
     */
    constexpr double aliasSampleRate = 48000.0;
    constexpr std::uint8_t aliasMidiNoteNumber = 69; // A4
    /**
     * TEST MODE
     * mode 0: designating neither "WaveType" nor "numHarmonics".
     * it becomes a sine wave, which set as the default.
     * and its "numHarmonics" should be 1.
     *
     * mode 1: designating "WaveType", but no "numHarmonics".
     * the default "numHarmonics" for each "WaveType" is below:
     *
     * - Sine: 1 (number of harmonics for sine wave is only 1)
     * - Triangle and Square: 3 (they emphasize only odd harmonics)
     * - Sawtooth: 2 (it emphasizes odd and even harmonics)
     *
     * mode 2: designating both "WaveType" and "numHarmonics".
     * according to the project contract,
     * the user-provided numHarmonics value may be adjusted as follows:
     *
     * (WaveType) & (numHarmonics) ⇒ (final numHarmonics)
     *
     * - Sine        more than 1   ⇒ 1
     * - Triangle    1             ⇒ 3
     * - Triangle    more than 64  ⇒ 64
     * - Square      1             ⇒ 3
     * - Square      more than 64  ⇒ 64
     * - Sawtooth    1             ⇒ 2
     * - Sawtooth    more than 64  ⇒ 64
     *
     * mode 3: ALIASING TEST
     * frequencies above the Nyquist limit are disallowed by default.
     * aliasing experiments are only allowed
     * when "allowAliasing_" is explicitly enabled.
     */
    constexpr std::uint8_t testModes[] = { 0, 1, 2, 3 };
    constexpr WaveType types[] = {
        WaveType::Sine,
        WaveType::Triangle,
        WaveType::Square,
        WaveType::Sawtooth
    };
    constexpr std::uint8_t excessiveNumHarmonics = 65; // kMaxHarmsNum_ is 64
    constexpr std::uint32_t samplesPerCycle = 256;

    Oscillator osc;

    for (const auto& mode : testModes) {
        for (const auto& type : types) {
            if (mode == 0) {
                osc.prepare(sampleRate);
                osc.setWaveType(); // Sine wave
                osc.setFrequency(midiNoteNumber);
            }
            else if (mode == 1) {
                osc.prepare(sampleRate);
                osc.setWaveType(type);
                osc.setFrequency(midiNoteNumber);
            }
            else if (mode == 2) {
                osc.prepare(sampleRate);
                osc.setWaveType(type, excessiveNumHarmonics);
                osc.setFrequency(midiNoteNumber);
            }
            else if (mode == 3) {
                osc.setAllowAliasing(true);
                osc.prepare(aliasSampleRate);
                osc.setWaveType(type, 64); // kMaxHarmsNum_ is 64
                osc.setFrequency(aliasMidiNoteNumber);
            }
            // phase_ is 0.0 before reset
            std::array<double, samplesPerCycle> samplesBeforeReset{};

            for (std::uint32_t i = 0; i < samplesPerCycle; ++i) {
                double sample = osc.renderSample();
                samplesBeforeReset[i] = sample;
                assert(
                    std::isfinite(sample) && "Sample value is not finite"
                );
            }
            std::cout << std::endl;

            osc.reset();

            // phase_ is 0.0 after reset
            for (std::uint32_t i = 0; i < samplesPerCycle; ++i) {
                const double sampleAfterReset = osc.renderSample();
                assert(
                    std::isfinite(sampleAfterReset) && "Sample value is not finite"
                );
                assert(
                    std::abs(samplesBeforeReset[i] - sampleAfterReset) <= 1e-12
                );
            }

            osc.reset(); // reset again after "reset test"

            if (mode == 0) break; // mode 0 only tests the default sine wave
        }
    }
}
