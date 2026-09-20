#include <iostream>
#include <cassert>

#include "score_audio_studio/dsp/Oscillator.hpp"

namespace {

    using namespace score_audio_studio::dsp;

    /**
     * sampleRate / frequency = 56320.0 / 220.0 = 256.0
     * There are 256 samples per a cycle.
     */
    constexpr double kSampleRate = 56320.0;
    constexpr std::uint8_t kMidiNoteNumber = 57; // A3

    constexpr double kTolerance = 1e-12;

    constexpr WaveType kWaveTypes[] = {
        WaveType::Sine,
        WaveType::Triangle,
        WaveType::Square,
        WaveType::Sawtooth
    };

    using SampleBuffer = \
        std::array<double, static_cast<std::size_t>(kSampleRate)>;

    SampleBuffer generateSamples(Oscillator& osc)
    {
        std::array<double, static_cast<std::size_t>(kSampleRate)> samples;
        for (std::uint32_t i = 0; i < samples.size(); ++i) {
            double sample = osc.renderSample();
            samples[i] = sample;
            assert(
                std::isfinite(sample) && "Sample value is not finite"
            );
        }
        return samples;
    } // generateSamples

    bool nearlyEqual(double actual, double expected)
    {
        return std::abs(actual - expected) < kTolerance;
    } // nearlyEqual

    void testDefaultSine()
    {
        /**
         * designating neither "WaveType" nor "numHarmonics".
         * it becomes a sine wave, which set as the default.
         * and its "numHarmonics" should be 1.
         */
        Oscillator osc;
        osc.prepare(kSampleRate);
        osc.setWaveType();                  // Sine Wave
        osc.setFrequency(kMidiNoteNumber);  // 220.0 Hz

        SampleBuffer samples = generateSamples(osc);

        const double frequency = osc.getFrequency();
        const std::uint32_t circle = kSampleRate / frequency;   // 256
        const std::uint32_t quarterCycleSamples = circle / 4;   // 64

        // not spun or spun 360 degree
        double spin000 = samples[quarterCycleSamples * 0];
        // spun 90 degree
        double spin090 = samples[quarterCycleSamples * 1];
        // spun 180 degree
        double spin180 = samples[quarterCycleSamples * 2];
        // spun 270 degree
        double spin270 = samples[quarterCycleSamples * 3];

        assert
        (
            nearlyEqual(spin000,  0.0)
            &&
            "Wrong Sample"
        );
        assert
        (
            nearlyEqual(spin090,  1.0)
            &&
            "Wrong Sample"
        );
        assert
        (
            nearlyEqual(spin180,  0.0)
            &&
            "Wrong Sample"
        );
        assert
        (
            nearlyEqual(spin270, -1.0)
            &&
            "Wrong Sample"
        );
    } // testDefaultSine

    void testDefaultWaveForms()
    {
        /**
         * designating "WaveType", but no "numHarmonics".
         * the default "numHarmonics" for each "WaveType" is below:
         *
         * - Sine: 1 (number of harmonics for sine wave is only 1)
         * - Triangle and Square: 3 (they emphasize only odd harmonics)
         * - Sawtooth: 2 (it emphasizes odd and even harmonics)
         */
        for (const auto& type : kWaveTypes) {
            Oscillator osc;
            osc.prepare(kSampleRate);
            osc.setWaveType(type);
            osc.setFrequency(kMidiNoteNumber);  // 220.0 Hz

            SampleBuffer samples = generateSamples(osc);

            const double frequency = osc.getFrequency();
            const std::uint32_t circle = kSampleRate / frequency;   // 256
            const std::uint32_t quarterCycleSamples = circle / 4;   // 64

            // not spun or spun 360 degree
            double spin000 = samples[quarterCycleSamples * 0];
            // spun 90 degree
            double spin090 = samples[quarterCycleSamples * 1];
            // spun 180 degree
            double spin180 = samples[quarterCycleSamples * 2];
            // spun 270 degree
            double spin270 = samples[quarterCycleSamples * 3];

            switch(type) {
                case WaveType::Sine:
                    assert
                    (
                        nearlyEqual(spin000,  0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin090,  1.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin180,  0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin270, -1.0)
                        &&
                        "Wrong Sample"
                    );
                    break;
                case WaveType::Triangle:
                    assert
                    (
                        nearlyEqual(spin000,   0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin090,  10.0 / 9.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin180,   0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin270, -10.0 / 9.0)
                        &&
                        "Wrong Sample"
                    );
                    break;
                case WaveType::Square:
                    assert
                    (
                        nearlyEqual(spin000,  0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin090,  2.0 / 3.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin180,  0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin270, -2.0 / 3.0)
                        &&
                        "Wrong Sample"
                    );
                    break;
                case WaveType::Sawtooth:
                    assert
                    (
                        nearlyEqual(spin000,  0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin090,  1.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin180,  0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin270, -1.0)
                        &&
                        "Wrong Sample"
                    );
                    break;
                default:
                    assert(false && "Wrong Wave Type");
                    break;
            }
        }
    }  // testDefaultWaveForms

    void testHarmonicLimit()
    {
        /** designating both "WaveType" and "numHarmonics".
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
         */
        const std::uint8_t excessiveNumHarmonics = 65;

        for (const auto& type : kWaveTypes) {
            Oscillator osc;
            osc.prepare(kSampleRate);
            osc.setWaveType(type, excessiveNumHarmonics);
            osc.setFrequency(kMidiNoteNumber);  // 220.0 Hz

            SampleBuffer samples = generateSamples(osc);

            const double frequency = osc.getFrequency();
            const std::uint32_t circle = kSampleRate / frequency;   // 256
            const std::uint32_t quarterCycleSamples = circle / 4;   // 64

            // not spun or spun 360 degree
            double spin000 = samples[quarterCycleSamples * 0];
            // spun 90 degree
            double spin090 = samples[quarterCycleSamples * 1];
            // spun 180 degree
            double spin180 = samples[quarterCycleSamples * 2];
            // spun 270 degree
            double spin270 = samples[quarterCycleSamples * 3];

            switch(type) {
                case WaveType::Sine:
                    assert
                    (
                        nearlyEqual(spin000,   0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin090,   1.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin180,   0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert(
                        nearlyEqual(spin270,  -1.0)
                        &&
                        "Wrong Sample"
                    );
                    break;
                case WaveType::Triangle:
                    /**
                     * At phase 0.25:
                     * 1 + 1/3² + 1/5² + ... + 1/63²
                     * = approximately 1.2258886857019062
                     */
                    assert
                    (
                        nearlyEqual(spin000,  0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin090,  1.2258886857019062)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin180,  0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin270, -1.2258886857019062)
                        &&
                        "Wrong Sample"
                    );
                    break;
                case WaveType::Square:
                    /**
                     * At phase 0.25:
                     * 1 - 1/3 + 1/5 - ... - 1/63
                     * = approximately 0.7775875684246718
                     */
                    assert
                    (
                        nearlyEqual(spin000,  0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin090,  0.7775875684246718)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin180,  0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin270, -0.7775875684246718)
                        &&
                        "Wrong Sample"
                    );
                    break;
                case WaveType::Sawtooth:
                    /**
                     * At phase 0.25, every even harmonic is zero:
                     * 1 - 1/3 + 1/5 - ... - 1/63
                     * = approximately 0.7775875684246718
                     */
                    assert
                    (
                        nearlyEqual(spin000,  0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin090,  0.7775875684246718)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin180,  0.0)
                        &&
                        "Wrong Sample"
                    );
                    assert
                    (
                        nearlyEqual(spin270, -0.7775875684246718)
                        &&
                        "Wrong Sample"
                    );
                    break;
                default:
                    assert(false && "Wrong Wave Type");
                    break;
            }
        }
    }  // testHarmonicLimit

    void testReset()
    {
        /**
         * Advance the oscillator beyond one full cycle.
         * With phaseIncrement = 1.0 / 256.0, the phase wraps back to 0.0
         * after every 256 rendered samples.
         */
        Oscillator osc;
        osc.prepare(kSampleRate);
        osc.setWaveType();                  // Sine Wave
        osc.setFrequency(kMidiNoteNumber);  // 220.0 Hz

        SampleBuffer samplesBeforeReset = generateSamples(osc);

        osc.renderSample();  // Move away from phase_ 0 before resetting.
        osc.reset();

        for (std::uint32_t i = 0; i < samplesBeforeReset.size(); ++i) {
            const double sampleAfterReset = osc.renderSample();
            assert(
                nearlyEqual(samplesBeforeReset[i], sampleAfterReset)
                &&
                "reset() did not restore the initial oscillator phase_"
            );
        }
    }  // testReset

    void testAliasingAllowed()
    {
        /**
         * MIDI note 127 corresponds to approximately 12543.85 Hz.
         *
         * With a sample rate of 8000 Hz:
         *
         * - Nyquist frequency = 4000 Hz
         * - 1st harmonic = 12543.85 Hz (above Nyquist)
         *
         * Therefore, even the fundamental frequency exceeds the Nyquist limit.
         * All generated harmonics are aliased when allowAliasing_ is enabled.
         */
        const double aSampleRate = 8000.0;
        const std::uint8_t aMidiNoteNumber = 127; // G9

        for (const auto& type : kWaveTypes) {

            Oscillator osc1; // Normal
            osc1.prepare(aSampleRate);
            osc1.setWaveType(type);
            osc1.setFrequency(aMidiNoteNumber); // 12,543.85... Hz
            SampleBuffer samples1 = generateSamples(osc1);

            Oscillator osc2; // Aliasing
            osc2.prepare(aSampleRate);
            osc2.setWaveType(type);
            osc2.setFrequency(aMidiNoteNumber); // 12,543.85... Hz
            osc2.setAllowAliasing(true);
            SampleBuffer samples2 = generateSamples(osc2);

            double energy1 = 0.0;
            double energy2 = 0.0;
            for (std::uint32_t i = 0; i < samples1.size(); ++i) {
                const double sample1 = samples1[i];
                const double sample2 = samples2[i];
                assert(std::isfinite(sample1));
                assert(std::isfinite(sample2));
                energy1 += sample1 * sample1;
                energy2 += sample2 * sample2;
            }
            assert
            (
                nearlyEqual(energy1, 0.0)
                &&
                "Expected silence for an above-Nyquist frequency"
            );
            assert
            (
                energy2 > kTolerance
                &&
                "Expected a non-silent signal when aliasing is enabled"
            );
        }
    }  // testAliasingAllowed

    void testHarmonicsAboveNyquistAreFiltered()
    {
        /**
         * MIDI note 57 corresponds to 220 Hz.
         *
         * With a sample rate of 8800 Hz:
         *
         *     Nyquist frequency = 4400 Hz
         *     20th harmonic     = 4400 Hz (Nyquist)
         *     21st harmonic     = 4620 Hz (Aliasing begins)
         *
         * Therefore, harmonics above the 20th harmonic are aliased
         * when allowAliasing is enabled.
         */
        const double aSampleRate = 8800.0;
        const std::uint8_t aMidiNoteNumber = 57; // A3

        for (const auto& type : kWaveTypes) {

            if (type == WaveType::Sine) {
                continue;
            }

            Oscillator osc1; // Normal
            osc1.prepare(aSampleRate);
            osc1.setWaveType(type, 64); // kMaxHarmsNum_ is 64
            osc1.setFrequency(aMidiNoteNumber); // 220.0 Hz
            SampleBuffer samples1 = generateSamples(osc1);

            Oscillator osc2; // Aliasing
            osc2.prepare(aSampleRate);
            osc2.setWaveType(type, 64); // kMaxHarmsNum_ is 64
            osc2.setFrequency(aMidiNoteNumber); // 220.0 Hz
            osc2.setAllowAliasing(true);
            SampleBuffer samples2 = generateSamples(osc2);

            bool differs = false;

            for (std::uint32_t i = 0; i < samples1.size(); ++i) {
                const double sample1 = samples1[i];
                const double sample2 = samples2[i];
                assert(std::isfinite(sample1));
                assert(std::isfinite(sample2));
                if (std::abs(sample1 - sample2) > 1e-12) {
                    differs = true;
                    break;
                }
            }
            assert(differs);
        }

    }  // testHarmonicsAboveNyquistAreFiltered

}  // namespace

int main()
{
    testDefaultSine();
    testDefaultWaveForms();
    testHarmonicLimit();
    testReset();
    testAliasingAllowed();
    testHarmonicsAboveNyquistAreFiltered();
    return 0;
}
