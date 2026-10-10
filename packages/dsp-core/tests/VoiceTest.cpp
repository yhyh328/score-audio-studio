#include <algorithm>
#include <cassert>
#include <cmath>

#include "score_audio_studio/dsp/Voice.hpp"

int main()
{
    using namespace score_audio_studio::dsp;

    constexpr double kSampleRate = 48000.0;
    constexpr std::uint32_t kFrameCount = 512;
    constexpr std::uint8_t kMidiNoteNumber = 57; // A3
    constexpr float kVelocity = 1.0f;

    constexpr std::uint64_t voiceKeys[] = { 1, 2 };

    constexpr OscSettings oscSettings[] = {
        {
            // default settings
        },
        {
            .waveType = WaveType::Triangle,
            .numHarmonics = 64,
            .allowAliasing = true
        }
    };

    constexpr AdsrSettings adsrSettings[] = {
        {
            // default settings
        },
        {
            .attack = {
                .seconds = 0.002,
                .targetGain = 1.0f
            },
            .decay = {
                .seconds = 0.8,
                .targetGain = 0.1f
            },
            .sustainSeconds = 3.0,
            .releaseSeconds = 0.2
        }
    };

    Voice voice;

    for (std::size_t i = 0; i < 2; i++) {

        float left[kFrameCount]{};
        float right[kFrameCount]{};

        voice.prepare(
            kSampleRate,
            oscSettings[i],
            adsrSettings[i]
        );
        // Voice is not active nor releasing yet
        assert(!voice.isActive() && !voice.isReleasing());

        voice.noteOn(
            voiceKeys[i],
            kMidiNoteNumber,
            kVelocity
        );
        // check if Voice reserves it's key without any changes
        assert(voice.getKey() == voiceKeys[i]);

        // Voice is active after noteOn()
        assert(voice.isActive() && !voice.isReleasing());

        std::uint32_t renderedSamples;
        renderedSamples = 0;

        const double noteSeconds =\
            adsrSettings[i].attack.seconds
            +
            adsrSettings[i].decay.seconds
            +
            adsrSettings[i].sustainSeconds;
        const std::uint32_t noteSamples = static_cast<std::uint32_t>(
            std::round(kSampleRate * noteSeconds)
        );
        while (renderedSamples < noteSamples) {
            const std::uint32_t frameCount = std::min(
                kFrameCount,
                noteSamples - renderedSamples
            );
            voice.render(
                left,
                right,
                0,
                frameCount
            );
            renderedSamples += frameCount;
            // Voice should be active still while rendering
            assert(voice.isActive() && !voice.isReleasing());
        }

        voice.noteOff();

        if (i == 0) {
            // if releaseSeconds is zero,
            // Voice doesn't need render() to get idle
            assert(!voice.isActive() && !voice.isReleasing());
        }

        renderedSamples = 0;
        const std::uint32_t releaseSamples = static_cast<std::uint32_t>(
            std::round(
                kSampleRate * adsrSettings[i].releaseSeconds
            )
        );
        while (renderedSamples < releaseSamples) {
            const std::uint32_t frameCount = std::min(
                kFrameCount,
                releaseSamples - renderedSamples
            );
            voice.render(
                left,
                right,
                0,
                frameCount
            );
            if (renderedSamples + frameCount >= releaseSamples)
            {   // Voice is idle 
                assert(!voice.isActive() && !voice.isReleasing());
                break;
            }
            else
            {   // Voice is releasing
                assert(voice.isActive() && voice.isReleasing());
            }
            renderedSamples += frameCount;
        }
        // reset() run automatically
        assert(voice.getKey() == 0);
        assert(!voice.isActive() && !voice.isReleasing());
        assert(voice.getLevel() == 0.0f);
    }

}  // namespace
