#include <cassert>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "score_audio_studio/dsp/AdsrEnvelope.hpp"

namespace
{
    using namespace score_audio_studio::dsp;

    constexpr double kSampleRate = 8000.0;

    constexpr float kAttackTargetGain = 0.9f,
                    kDecayTargetGain  = 0.6f;

    bool gLinear         = false,
         gExponential    = false,
         gGenerateInputs = false;
    
    void generateInputs
    (
        const std::string dirName,
        const EnvelopeCurve curve,
        const double attackSeconds,
        const double decaySeconds,
        const double sustainSeconds,
        const double releaseSeconds,
        const std::vector<float>& gainVec
    )
    {
        /**	
         * Plot ADSR envelope curves from CSV files for visual verification.
         *
         * This function is expected to be called by
         * score-audio-studio/tools/visualize_adsr_envelope.py,
         * to generate CSV files like below:
         *
         *  score-audio-studio/docs/Adsr-Envelope/
		 *  ├── Linear
		 *  │   └── A[attack_secs]-D[decay_secs]-S[sustain_secs]-R[release_secs].csv
		 *  └── Exponential
		 *      └── A[attack_secs]-D[decay_secs]-S[sustain_secs]-R[release_secs].csv
         *
         * The CSV files are then converted into ADSR envelope curve graphs.
         */

        // Generate a CSV file name.
        const std::string attackSecondsStr =\
            std::to_string(static_cast<int>(std::round(attackSeconds * 1000.0))) + "ms";
        const std::string decaySecondsStr =\
            std::to_string(static_cast<int>(std::round(decaySeconds * 1000.0))) + "ms";
        const std::string sustainSecondsStr =\
            std::to_string(static_cast<int>(std::round(sustainSeconds * 1000.0))) + "ms";
        const std::string releaseSecondsStr =\
            std::to_string(static_cast<int>(std::round(releaseSeconds * 1000.0))) + "ms";

        const std::string fileName =\
            "A" + attackSecondsStr + "-D" + decaySecondsStr +
            "-S" + sustainSecondsStr + "-R" + releaseSecondsStr;

        // Choose the waveform directory name
        std::filesystem::path inputDir =\
            std::filesystem::path {
                "packages/dsp-core/tests/data/Adsr-Envelope/"
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

        csv << "# sample_rate=" << kSampleRate << '\n';
        csv << "# curve="
            << ((curve == EnvelopeCurve::Linear)
                ? "Linear"
                : "Exponential")
            << '\n';

        csv << "# attack_seconds=" << attackSeconds << '\n';
        csv << "# attack_target=" << kAttackTargetGain << '\n';

        csv << "# decay_seconds=" << decaySeconds << '\n';
        csv << "# decay_target=" << kDecayTargetGain << '\n';

        csv << "# sustain_seconds=" << sustainSeconds << '\n';
        csv << "# sustain_target=" << kDecayTargetGain << '\n';

        csv << "# release_seconds=" << releaseSeconds << '\n';
        csv << "# release_target=0\n";

        csv << "gains\n";

        // Put gains into the CSV file.
        std::size_t i = 0;
        while (i < gainVec.size()) { 
            csv << gainVec[i++] << '\n';
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
    }  // generateInputs

    constexpr float kTolerance = 1e-6;

    void runAdsrScenario(
        AdsrEnvelope& envelope,
        EnvelopeCurve curve,
        double attackSeconds,
        double decaySeconds,
        double sustainSeconds,
        double releaseSeconds
    )
    {
        std::cout << "Sample Rate: " << kSampleRate << std::endl;
        std::cout << std::endl;

        std::cout << "Attack ⇒   " 
                  << "seconds: "         << attackSeconds
                  << "    target gain: " << kAttackTargetGain
                  << std::endl;
        std::cout << "Decay ⇒   " 
                  << "seconds: "         << decaySeconds
                  << "    target gain: " << kDecayTargetGain
                  << std::endl;
        std::cout << "Sustain ⇒   " 
                  << "seconds: "         << sustainSeconds
                  << std::endl;
        std::cout << "Release ⇒   " 
                  << "seconds: "         << releaseSeconds
                  << std::endl;
        std::cout << std::endl;

        const std::uint32_t attackSamples  =\
            static_cast<std::uint32_t>(std::round(kSampleRate * attackSeconds));
        const std::uint32_t decaySamples   =\
            static_cast<std::uint32_t>(std::round(kSampleRate * decaySeconds));
        const std::uint32_t sustainSamples =\
            static_cast<std::uint32_t>(std::round(kSampleRate * sustainSeconds));
        const std::uint32_t releaseSamples =\
            static_cast<std::uint32_t>(std::round(kSampleRate * releaseSeconds));

        switch (curve) {
            case EnvelopeCurve::Linear:
                std::cout << "Testing ADSR envelope with a linear curve..."
                          << std::endl;
                break;
            case EnvelopeCurve::Exponential:
                std::cout << "Testing ADSR envelope with a exponential curve..."
                          << std::endl;
                break;
        }
        std::cout << std::endl;

        // use for generateInputs() to create CSV files for visual verification.
        std::vector<float> gainVec;

        /**
         * 1. Set ADSR features.
         *
         * The stage progression follows the standard order:
         *     Attack, Decay, Sustain, Release
         */
        envelope.setAttackFeatures(
            attackSeconds,
            kAttackTargetGain
        );
        envelope.setDecayFeatures(
            decaySeconds,
            kDecayTargetGain
        );
        envelope.setSustainFeatures(sustainSeconds);
        envelope.setReleaseFeatures(releaseSeconds);

        /**
         * 2. Prepare ADSR envelope.
         *
         * Set sample rate
         * Store ADSR features
         * Set envelope curve
         *
         *  ⬇
         * Idle => Attack => Decay => Sustain => Release
         */
        envelope.prepare(kSampleRate, curve);

        /**
         * 3. noteOn
         *
         *           ⬇
         * Idle => Attack => Decay => Sustain => Release
         */
        envelope.noteOn();

        /**
         * 4. process
         */
        float prvGain = 0.0f,
              curGain = envelope.getGain();
        
        if (gGenerateInputs) {
            gainVec.push_back(curGain);
        }

        //           ⬇
        // Idle => Attack => Decay => Sustain => Release
        if (attackSamples == 0) {
            std::cout << "    Stage Attack is skipped." << std::endl;
            prvGain = curGain;
        }
        else {
            std::cout << "    Stage Attack is processing..." << std::endl;
            for (std::uint32_t i = 0; i < attackSamples; ++i)
            {
                envelope.process();
                curGain = envelope.getGain();
                std::cout << "    gain: " << curGain << "\n";
                /**
                 * If the Decay stage has zero samples,
                 * the last Attack process may advance directly to the Decay target gain.
                 * Therefore, the Attack target gain may not be observable
                 * on the last Attack sample.
                 */
                if 
                (
                    decaySamples == 0 && i == attackSamples - 1
                ) {
                    assert
                    (
                        std::abs(curGain - kDecayTargetGain) < kTolerance
                    );
                }
                else {
                    assert(prvGain - curGain <= kTolerance);
                }
                prvGain = curGain;
                if (gGenerateInputs) {
                    gainVec.push_back(curGain);
                }
            }
        }
        std::cout << std::endl;

        //                     ⬇
        // Idle => Attack => Decay => Sustain => Release
        if (decaySamples == 0) {
            std::cout << "    Stage Decay is skipped." << std::endl;
            assert
            (
                std::abs(curGain - kDecayTargetGain) < kTolerance
            );
            prvGain = curGain;
        }
        else {
            std::cout << "    Stage Decay is processing..." << std::endl;
            for (std::uint32_t i = 0; i < decaySamples; ++i)
            {
                envelope.process();
                curGain = envelope.getGain();
                std::cout << "    gain: " << curGain << "\n";
                assert(prvGain >= curGain - kTolerance);
                prvGain = curGain;
                if (gGenerateInputs) {
                    gainVec.push_back(curGain);
                }
            }
        }
        std::cout << std::endl;

        //                               ⬇
        // Idle => Attack => Decay => Sustain => Release
        std::cout << "    Stage Sustain is processing..." << std::endl;
        std::cout << "    gain: " << curGain << "\n";
        for (std::uint32_t i = 0; i < sustainSamples; ++i)
        {
            envelope.process();
            curGain = envelope.getGain();
            assert(prvGain == curGain);
            prvGain = curGain;
            if (gGenerateInputs) {
                gainVec.push_back(curGain);
            }
        }
        std::cout << std::endl;

        /**
         * 5. noteOff
         *
         *                                          ⬇
         * Idle => Attack => Decay => Sustain => Release
         */
        envelope.noteOff();
        /**
         * If Release has samples, subsequent process() calls
         * move the gain from the current level to zero.
         * A zero-sample Release reaches zero immediately.
         */
        if (releaseSamples == 0) {
            curGain = envelope.getGain();
            std::cout << "    Stage Release is skipped." << std::endl;
            assert
            (curGain == 0.0f);
            prvGain = curGain;
        }
        else {
            std::cout << "    Stage Release is processing..." << std::endl;
            for (std::uint32_t i = 0; i < releaseSamples; ++i)
            {
                envelope.process();
                curGain = envelope.getGain();
                std::cout << "    gain: " << curGain << "\n";
                assert(prvGain >= curGain - kTolerance);
                prvGain = curGain;
                if (gGenerateInputs) {
                    gainVec.push_back(curGain);
                }
            }
        }
        std::cout << std::endl;

        //  ⬇
        // Idle => Attack => Decay => Sustain => Release
        assert(envelope.getGain() == 0.0f);
        std::cout << "    Return to stage Idle" << std::endl;
        std::cout << "    gain: " << envelope.getGain() << std::endl;

        if (gGenerateInputs) {
            // Separate the CTest log from the CSV generation output.
            std::cout << std::endl;
            std::cout << std::endl;
            std::cout << "Generating CSV files..." << std::endl;

            const std::string dirName =\
                (curve == EnvelopeCurve::Linear) ? "Linear" : "Exponential";

            generateInputs(
                dirName,
                curve,
                attackSeconds,
                decaySeconds,
                sustainSeconds,
                releaseSeconds,
                gainVec
            );
        }
    }

    void help(std::ostream& output)
    {
        output << "Usage: adsr_envelope_test [--generate-inputs]\n"
               << "       adsr_envelope_test [--help]\n"
               << "\n"
               << "Options:\n"
               << "  --generate-inputs    Input generation.\n"
               << "  -h,  --help          Show this help message.\n";
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
        if (argument != "--generate-inputs") {
            std::cerr << "Invalid arguments.\n\n";
            help(std::cerr);
            return 1;
        }
        // generateInputs() ON
        gGenerateInputs = true;
    }
    if (argc != 1 && !gGenerateInputs) {
        std::cerr << "Invalid arguments.\n\n";
        help(std::cerr);
        return 1;
    }
    try {
        AdsrEnvelope envelope;
        for (EnvelopeCurve curve : { EnvelopeCurve::Linear, EnvelopeCurve::Exponential })
        {
            runAdsrScenario(
                envelope,
                curve,
                0.5,    // attackSeconds
                0.1,    // decaySeconds
                3.0,    // sustainSeconds
                1.0     // releaseSeconds
            );
            runAdsrScenario(
                envelope,
                curve,
                0.0,    // attackSeconds
                0.1,    // decaySeconds
                3.0,    // sustainSeconds
                1.0     // releaseSeconds
            );
            runAdsrScenario(
                envelope,
                curve,
                0.5,    // attackSeconds
                0.0,    // decaySeconds
                3.0,    // sustainSeconds
                1.0     // releaseSeconds
            );
            runAdsrScenario(
                envelope,
                curve,
                0.5,    // attackSeconds
                0.1,    // decaySeconds
                0.0,    // sustainSeconds
                1.0     // releaseSeconds
            );
            runAdsrScenario(
                envelope,
                curve,
                0.5,    // attackSeconds
                0.1,    // decaySeconds
                3.0,    // sustainSeconds
                0.0     // releaseSeconds
            );
            runAdsrScenario(
                envelope,
                curve,
                0.0,    // attackSeconds
                0.0,    // decaySeconds
                0.0,    // sustainSeconds
                0.0     // releaseSeconds
            );
            if (gGenerateInputs) {
                std::cout << "CSV generation complete." << std::endl;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
