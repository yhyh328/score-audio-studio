#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string_view>

#include "score_audio_studio/dsp/AdsrEnvelope.hpp"

namespace
{
    using namespace score_audio_studio::dsp;

    constexpr double kSampleRate = 8000.0;

    constexpr float kAttackTargetGain = 0.9f,
                    kDecayTargetGain  = 0.6f;

    bool gLinear         = false,
         gExponential    = false;

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
            }
        }
        std::cout << std::endl;

        //  ⬇
        // Idle => Attack => Decay => Sustain => Release
        assert(envelope.getGain() == 0.0f);
        std::cout << "    Return to stage Idle" << std::endl;
        std::cout << "    gain: " << envelope.getGain() << std::endl;
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
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
