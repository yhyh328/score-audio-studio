#include <cassert>
#include <cstdint>
#include <iostream>
#include <string_view>

#include "score_audio_studio/dsp/AdsrEnvelope.hpp"

namespace
{
    using namespace score_audio_studio::dsp;

    constexpr double kSampleRate = 8000.0;

    constexpr double kAttackSeconds  = 0.5,
                     kDecaySeconds   = 0.1,
                     kSustainSeconds = 3.0,
                     kReleaseSeconds = 1.0;

    constexpr float kAttackTargetGain = 0.9f,
                    kDecayTargetGain  = 0.6f;

    bool gLinear         = false,
         gExponential    = false;

    constexpr std::uint32_t kAttackSamples  =\
                            static_cast<std::uint32_t>(kSampleRate * kAttackSeconds),
                            kDecaySamples   =\
                            static_cast<std::uint32_t>(kSampleRate * kDecaySeconds),
                            kSustainSamples =\
                            static_cast<std::uint32_t>(kSampleRate * kSustainSeconds),
                            kReleaseSamples =\
                            static_cast<std::uint32_t>(kSampleRate * kReleaseSeconds);

    void runAdsrScenario(
        AdsrEnvelope& envelope,
        EnvelopeCurve curve
    )
    {
        std::cout << "Sample Rate: " << kSampleRate << std::endl;
        std::cout << std::endl;

        std::cout << "Attack ⇒   " 
                  << "seconds: "         << kAttackSeconds
                  << "    target gain: " << kAttackTargetGain
                  << std::endl;
        std::cout << "Decay ⇒   " 
                  << "seconds: "         << kDecaySeconds
                  << "    target gain: " << kDecayTargetGain
                  << std::endl;
        std::cout << "Sustain ⇒   " 
                  << "seconds: "         << kSustainSeconds
                  << std::endl;
        std::cout << "Release ⇒   " 
                  << "seconds: "         << kReleaseSeconds
                  << std::endl;
        std::cout << std::endl;

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
            kAttackSeconds,
            kAttackTargetGain
        );
        envelope.setDecayFeatures(
            kDecaySeconds,
            kDecayTargetGain
        );
        envelope.setSustainFeatures(kSustainSeconds);
        envelope.setReleaseFeatures(kReleaseSeconds);

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
        //           ⬇
        // Idle => Attack => Decay => Sustain => Release
        std::cout << "    Stage Attack is processing..." << std::endl;
        for (std::uint32_t i = 0; i < kAttackSamples; ++i)
        {
            envelope.process();
            std::cout << "    gain: " << envelope.getGain() << "\n";
        }
        std::cout << std::endl;

        //                     ⬇
        // Idle => Attack => Decay => Sustain => Release
        std::cout << "    Stage Decay is processing..." << std::endl;
        for (std::uint32_t i = 0; i < kDecaySamples; ++i)
        {
            envelope.process();
            std::cout << "    gain: " << envelope.getGain() << "\n";
        }
        std::cout << std::endl;

        //                               ⬇
        // Idle => Attack => Decay => Sustain => Release
        std::cout << "    Stage Sustain is processing..." << std::endl;
        for (std::uint32_t i = 0; i < kSustainSamples; ++i)
        {
            envelope.process();
            std::cout << "    gain: " << envelope.getGain() << "\n";
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
         * noteOff() does not silence the envelope immediately.
         * Subsequent process() calls are required to move the
         * gain from the current level to zero over the release time.
         */
        std::cout << "    Stage Release is processing..." << std::endl;
        for (std::uint32_t i = 0; i < kReleaseSamples; ++i)
        {
            envelope.process();
            std::cout << "    gain: " << envelope.getGain() << "\n";
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
        runAdsrScenario(envelope, EnvelopeCurve::Linear);
        runAdsrScenario(envelope, EnvelopeCurve::Exponential);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
