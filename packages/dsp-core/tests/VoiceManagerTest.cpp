#include <algorithm>
#include <cassert>
#include <cmath>
#include <set>
#include <span>

#include "score_audio_studio/dsp/VoiceManager.hpp"

namespace {
    using namespace score_audio_studio::dsp;

    constexpr double kSampleRate = 48000.0;
    constexpr std::uint32_t kFrameCount = 512;
    constexpr float kVelocity = 1.0f;
    constexpr std::size_t kMaxVoices = 16;

    constexpr OscSettings kOscSettings = {
        .waveType = WaveType::Triangle,
        .numHarmonics = 64,
        .allowAliasing = true
    };
    constexpr AdsrSettings kAdsrSettings = {
        .attack = {
            .seconds = 0.002,
            .targetGain = kVelocity
        },
        .decay = {
            .seconds = 0.8,
            .targetGain = 0.1f
        },
        .sustainSeconds = 3.0,
        .releaseSeconds = 0.2
    };

    VoiceManager manager;

    std::size_t i;
    std::uint32_t frameCount,
                  renderedSamples;
    std::uint64_t voiceKey;

    void clearBuffers
    (
        std::span<float> l,
        std::span<float> r
    ) noexcept
    {
        std::fill(std::begin(l), std::end(l), 0.0f);
        std::fill(std::begin(r), std::end(r), 0.0f);
    } // clearBuffers
}

int main()
{
    using namespace score_audio_studio::dsp;

    {// single-note test
        float l[kFrameCount]{};
        float r[kFrameCount]{};
        manager.prepare(
            kSampleRate,
            kMaxVoices,
            kOscSettings,
            kAdsrSettings
        );
        {// active  phase
            voiceKey = manager.noteOn(69, kVelocity); // A4
            // check that a valid voice key was issued
            assert(voiceKey != 0);
            manager.render(l, r, 0, kFrameCount);
            bool hasNonZeroSample = false;
            for (i = 0; i < kFrameCount; i++) {
                if (l[i] != 0.0f || r[i] != 0.0f) {
                    hasNonZeroSample = true;
                    break;
                }
            }
            assert(hasNonZeroSample);
        }
        {// release phase
            manager.noteOff(voiceKey);
            const std::uint32_t releaseSamples =
                static_cast<std::uint32_t>(
                    std::round(
                    kSampleRate * kAdsrSettings.releaseSeconds
                ));
            std::uint32_t firstFrameCount;
            {// 1. render the first release block and verify that the release tail actually produces audio
                firstFrameCount = std::min(kFrameCount, releaseSamples);
                clearBuffers(l, r);
                manager.render(l, r, 0, firstFrameCount);
                bool hasNonZeroSample = false;
                for (i = 0; i < firstFrameCount; ++i) {
                    if (l[i] != 0.0f || r[i] != 0.0f) {
                        hasNonZeroSample = true;
                        break;
                    }
                }
                assert(hasNonZeroSample);
            }
            {// 2. advance through the rest of the release stage
                renderedSamples = firstFrameCount;
                while (renderedSamples < releaseSamples) {
                    frameCount = std::min(
                        kFrameCount,
                        releaseSamples - renderedSamples
                    );
                    clearBuffers(l, r);
                    manager.render(l, r, 0, frameCount);
                    renderedSamples += frameCount;
                }
            }
            {// 3. after enough time for Release to complete, the next block should be silent.
                clearBuffers(l ,r);
                manager.render(l, r, 0, kFrameCount);
                for (i = 0; i < kFrameCount; i++) {
                    assert(l[i] == 0.0f);
                    assert(r[i] == 0.0f);
                }
            }
        }
        {// reset test
            voiceKey = manager.noteOn(69, kVelocity);
            assert(voiceKey != 0);
            manager.render(l, r, 0, kFrameCount);
            bool hasNonZeroSample = false;
            for (i = 0; i < kFrameCount; i++) {
                if (l[i] != 0.0f || r[i] != 0.0f) {
                    hasNonZeroSample = true;
                    break;
                }
            }
            assert(hasNonZeroSample);
            manager.reset();
            // due to reset() it renders silence
            clearBuffers(l ,r);
            manager.render(l, r, 0, kFrameCount);
            for (i = 0; i < kFrameCount; i++) {
                assert(l[i] == 0.0f);
                assert(r[i] == 0.0f);
            }
        }
    }
    {// multiple-note test
        float l[kFrameCount]{};
        float r[kFrameCount]{};
        std::uint8_t midiNoteNumber = 69; // A4
        std::uint32_t firstFrameCount,
                      releaseSamples;
        std::array<std::uint64_t, kMaxVoices> voiceKeys{};
        manager.prepare(
            kSampleRate,
            kMaxVoices,
            kOscSettings,
            kAdsrSettings
        );
        {// active  phase
            i = 0;
            while (i < kMaxVoices) {
                // A4 to C6
                voiceKey = manager.noteOn(
                    midiNoteNumber++, kVelocity
                );
                // check that a valid voice key was issued
                assert(voiceKey != 0);
                voiceKeys[i++] = voiceKey;
            }
            manager.render(l, r, 0, kFrameCount);
            bool hasNonZeroSample = false;
            for (i = 0; i < kFrameCount; i++) {
                if (l[i] != 0.0f || r[i] != 0.0f) {
                    hasNonZeroSample = true;
                    break;
                }
            }
            assert(hasNonZeroSample);
            std::set<std::uint64_t> voiceKeysSet(
                std::begin(voiceKeys), std::end(voiceKeys)
            ); // check that voice key are unique
            assert(voiceKeysSet.size() == kMaxVoices);
        }
        {// release phase
            i = 0;
            while (i < kMaxVoices) {
                voiceKey = voiceKeys[i++];
                manager.noteOff(voiceKey);
            }
            releaseSamples = static_cast<std::uint32_t>(
                std::round(
                    kSampleRate * kAdsrSettings.releaseSeconds
                )
            );
            {// render the first release block and verify that the release tail actually produces audio
                firstFrameCount = std::min(kFrameCount, releaseSamples);
                clearBuffers(l, r);
                manager.render(l, r, 0, firstFrameCount);
                bool hasNonZeroSample = false;
                for (i = 0; i < firstFrameCount; ++i) {
                    if (l[i] != 0.0f || r[i] != 0.0f) {
                        hasNonZeroSample = true;
                        break;
                    }
                }
                assert(hasNonZeroSample);
            }
            {// advance through the rest of the release stage
                renderedSamples = firstFrameCount;
                while (renderedSamples < releaseSamples) {
                    frameCount = std::min(
                        kFrameCount,
                        releaseSamples - renderedSamples
                    );
                    clearBuffers(l ,r);
                    manager.render(l, r, 0, frameCount);
                    renderedSamples += frameCount;
                }
            }
            {// after enough time for Release to complete, the next block should be silent.
                clearBuffers(l ,r);
                manager.render(l, r, 0, kFrameCount);
                for (i = 0; i < kFrameCount; i++) {
                    assert(l[i] == 0.0f);
                    assert(r[i] == 0.0f);
                }
            }
            manager.reset();
        }
        {// maximum voice count is reached
            {// find an inactive voice
                i = 0;
                // turn kMaxVoices - 1 inactive voices to active
                while (i < kMaxVoices - 1) {
                    voiceKey = manager.noteOn( // C6
                        midiNoteNumber, kVelocity
                    );
                    // check that a valid voice key was issued
                    assert(voiceKey != 0);
                    voiceKeys[i++] = voiceKey;
                }
                // count of inactive voices is one
                assert(manager.activeVoiceCount() == kMaxVoices - 1);

                // turn the inactive voice to active
                voiceKey = manager.noteOn(midiNoteNumber, kVelocity);
                assert(voiceKey != 0); // check that a valid voice key was issued
                manager.render(l, r, 0, kFrameCount);

                // all voices are active
                assert(manager.activeVoiceCount() == kMaxVoices);
                
                manager.reset();
                clearBuffers(l, r);
            }
            std::uint64_t voiceJey, voiceLey;
            {// find a releasing voice with the lowest envelope level
                i = 0;
                // turn all inactive voices to active
                while (i < kMaxVoices) {
                    voiceKey = manager.noteOn( // C6
                        midiNoteNumber, kVelocity
                    );
                    // check that a valid voice key was issued
                    assert(voiceKey != 0);
                    voiceKeys[i++] = voiceKey;
                }
                // all voices are active
                assert(manager.activeVoiceCount() == kMaxVoices);

                clearBuffers(l, r);
                manager.render(l, r, 0, kFrameCount);
                clearBuffers(l, r);

                // !! THE TEST BELOW REQUIRES
                // !! kSampleRate * releaseSeconds SHOULD NOT BE ZERO
                assert
                (
                    kSampleRate * kAdsrSettings.releaseSeconds
                    >
                    1e-12
                );
                for (i = 0; i < 4; i ++) {
                    if (i < 3) { // noteOff()
                        voiceKey = voiceKeys[i];
                        assert(manager.isVoiceActive(voiceKey));
                        manager.noteOff(voiceKey);
                        manager.render(l, r, 0, kFrameCount);
                    }
                    if (0 < i) { // noteOn()
                        voiceLey = manager.noteOn(69, kVelocity); // A4
                        // check that a valid voice key was issued
                        assert(voiceLey != 0);
                        // check that the key is not same with previous one
                        assert(voiceLey != voiceJey);
                        // check a voice with the new key is active
                        assert(manager.isVoiceActive(voiceLey));
                        // check the preivous key is not active more
                        assert(!manager.isVoiceActive(voiceJey));
                    }
                    voiceJey = voiceKey;
                }
                manager.reset();
                clearBuffers(l, r);
            }
            {// find the smallest key which means the oldest
                i = 0;
                // turn all inactive voices to active
                while (i < kMaxVoices) {
                    voiceKey = manager.noteOn( // C6
                        midiNoteNumber, kVelocity
                    );
                    // check that a valid voice key was issued
                    assert(voiceKey != 0);
                    voiceKeys[i++] = voiceKey;
                }
                voiceJey = std::ranges::min(voiceKeys);
                // this test calls no noteOff()
                voiceLey = manager.noteOn(midiNoteNumber, kVelocity);
                assert(voiceJey < voiceLey);
                assert(!manager.isVoiceActive(voiceJey));
                assert(manager.isVoiceActive(voiceLey));

                manager.reset();
                clearBuffers(l, r);
            }
        }
        {// reset test
            voiceKey = manager.noteOn(69, kVelocity);
            assert(voiceKey != 0);
            manager.render(l, r, 0, kFrameCount);
            bool hasNonZeroSample = false;
            for (i = 0; i < kFrameCount; i++) {
                if (l[i] != 0.0f || r[i] != 0.0f) {
                    hasNonZeroSample = true;
                    break;
                }
            }
            assert(hasNonZeroSample);
            manager.reset();
            // due to reset() it renders silence
            clearBuffers(l ,r);
            manager.render(l, r, 0, kFrameCount);
            for (i = 0; i < kFrameCount; i++) {
                assert(l[i] == 0.0f);
                assert(r[i] == 0.0f);
            }
        }
    }
    return 0;
}
