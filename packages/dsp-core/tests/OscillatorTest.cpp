#include <iostream>
#include <cassert>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <initializer_list>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

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

    bool gGenerateInputs = false;
    void generateInputs(
        const std::string dirName,
        const WaveType type,
        const std::initializer_list<const SampleBuffer*> samplesArr,
        const std::uint32_t cycles
    ) {
        /**
         * Plot oscillator waveform samples from CSV files for visual verification.
         *
         * This function is expected to be called by
         * score-audio-studio/tools/visualize_oscillator.py,
         * to generate CSV files like below:
         *
         *  score-audio-studio/packages/dsp-core/tests/data/Oscillator/
         *  ├── test-default-sine
         *  │   └── Sine_yyyymmdd_hhmmss.csv
         *  ├── test-default-wave-forms
         *  │   ├── Sine_yyyymmdd_hhmmss.csv
         *  │   ├── Triangle_yyyymmdd_hhmmss.csv
         *  │   ├── Square_yyyymmdd_hhmmss.csv
         *  │   └── Sawtooth_yyyymmdd_hhmmss.csv
         *  ├── test-harmonic-limit
         *  │   ├── Sine_yyyymmdd_hhmmss.csv
         *  │   ├── Triangle_yyyymmdd_hhmmss.csv
         *  │   ├── Square_yyyymmdd_hhmmss.csv
         *  │   └── Sawtooth_yyyymmdd_hhmmss.csv
         *  ├── test-aliasing-allowed
         *  │   ├── Sine_yyyymmdd_hhmmss.csv
         *  │   ├── Triangle_yyyymmdd_hhmmss.csv
         *  │   ├── Square_yyyymmdd_hhmmss.csv
         *  │   └── Sawtooth_yyyymmdd_hhmmss.csv
         *  └── test-harmonics-above-nyquist-are-filtered
         *      ├── Triangle_yyyymmdd_hhmmss.csv
         *      ├── Square_yyyymmdd_hhmmss.csv
         *      └── Sawtooth_yyyymmdd_hhmmss.csv
         *
         * The CSV files are then converted into waveform graphs.
         */

        // Prevention of the Integer-Overflow from 2038.
        static_assert(
            8 <= sizeof(std::time_t),
            "CSV timestamps require at least 64-bit time_t"
        );

        // Generate a timestamp for the CSV file name.
        const std::string waveType =\
            (type == WaveType::Sine)     ? "Sine"    :
            (type == WaveType::Triangle) ? "Triangle":
            (type == WaveType::Square)   ? "Square"  : "Sawtooth";

        std::time_t now = std::time(nullptr);
        const std::tm* local = std::localtime(&now);
        char timestamp[16]; // "yyyymmdd_hhmmss"
        if
        (
            local == nullptr
            ||
            std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", local) == 0
        ) {
            throw std::runtime_error("Cannot create timestamp");
        }
        const std::string fileName = waveType + "_" + timestamp;

        // Choose the waveform directory name.
        std::filesystem::path inputDir =\
            std::filesystem::path{
                "packages/dsp-core/tests/data/Oscillator/"
            } / dirName;
        std::error_code error;
        std::filesystem::create_directories(inputDir, error);
        if (error) {
            throw std::runtime_error(
                "Cannot create directory " + inputDir.string() + ": " + error.message()
            );
        }

        // Set the CSV path.
        const std::filesystem::path csvPath = inputDir / (fileName + ".csv");
        std::ofstream csv(csvPath);
        if (!csv) {
            throw std::runtime_error("Cannot open file: " + csvPath.string());
        }

        // aliasing test needs two buffers.
        if (samplesArr.size() == 2) {
            csv << waveType << "_normal" << ","
                << waveType << "_aliasing\n";
        }
        else {
            csv << waveType << "\n";
        }

        // Put samples into the CSV file.
        for (std::size_t i = 0; i < cycles; ++i) {
            bool first = true;
            for (const SampleBuffer* samples : samplesArr) {
                if (!first) {
                    csv << ',';
                }
                csv << (*samples)[i];
                first = false;
            }
            csv << '\n';
        }
        csv.close();
        if (!csv) {
            throw std::runtime_error("Cannot write file: " + csvPath.string());
        }

        std::cout << "  "
                  << fileName
                  << " is generated: "
                  << dirName
                  << std::endl;
    } // generateInputs

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
        if (gGenerateInputs) {
            const std::string dirName = "test-default-sine";
            generateInputs(dirName, WaveType::Sine, {&samples}, circle * 10);
        }
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
            if (gGenerateInputs) {
                const std::string dirName = "test-default-wave-forms";
                generateInputs(dirName, type, {&samples}, circle * 10);
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
            if (gGenerateInputs) {
                const std::string dirName = "test-harmonic-limit";
                generateInputs(dirName, type, {&samples}, circle * 10);
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
        // This test doesn't generate CSV files.
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
            if (gGenerateInputs) {
                const std::string dirName = "test-aliasing-allowed";
                generateInputs(dirName, type, {&samples1, &samples2}, 256);
            }
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
            if (gGenerateInputs) {
                const std::string dirName = "test-harmonics-above-nyquist-are-filtered";
                generateInputs(dirName, type, {&samples1, &samples2}, 256);
            }
        }

    }  // testHarmonicsAboveNyquistAreFiltered

    void help(std::ostream& output)
    {
        output << "Usage: oscillator_test [--generate-inputs | --help]\n"
               << "\n"
               << "Options:\n"
               << "  (no arguments)      Run the oscillator tests.\n"
               << "  --generate-inputs   Input generation.\n"
               << "  -h, --help          Show this help message.\n";
    }  // help

}  // namespace

int main(int argc, char* argv[])
{
    if (argc == 2) {
        const std::string_view argument(argv[1]);
        if (argument == "-h" || argument == "--help") {
            help(std::cout);
            return 0;
        }
        if (argument == "--generate-inputs") {
            // generateInputs() ON
            gGenerateInputs = true;
            // Separate the CTest log from the CSV generation output.
            std::cout << std::endl;
            std::cout << std::endl;
            std::cout << "Generating CSV files..." << std::endl;
        }
    }
    if (argc != 1 && !gGenerateInputs) {
        std::cerr << "Invalid arguments.\n\n";
        help(std::cerr);
        return 1;
    }
    try {
        testDefaultSine();
        testDefaultWaveForms();
        testHarmonicLimit();
        testReset();
        testAliasingAllowed();
        testHarmonicsAboveNyquistAreFiltered();
        if (gGenerateInputs) {
            std::cout << "CSV generation complete." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
