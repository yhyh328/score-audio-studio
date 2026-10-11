#include <cassert>

#include "score_audio_studio/dsp/VoiceManager.hpp"

namespace score_audio_studio::dsp {

void VoiceManager::prepare(
    const double sampleRate,
    const std::size_t maxVoices,
    const OscSettings& oscSettings,
    const AdsrSettings& adsrSettings
) noexcept
{
    assert(0 < sampleRate && std::isfinite(sampleRate));
    sampleRate_ = sampleRate;
    maxVoices_ = maxVoices;
    voices_.resize(maxVoices);
    for (Voice& voice: voices_) {
        voice.prepare(
            sampleRate,
            oscSettings,
            adsrSettings
        );
    }
}  // VoiceManager::prepare

std::uint64_t VoiceManager::noteOn(
    const std::uint8_t midiNoteNumber,
    const float velocity
) noexcept
{
    /**
     * noteOn() returns the voice key 
     * required to match the corresponding noteOff()
     */
    Voice& voice = allocate();
    voice.noteOn(
        voiceKey_++,
        midiNoteNumber,
        velocity
    );
    return voice.getKey();
}  // VoiceManager::noteOn

Voice& VoiceManager::allocate() noexcept
{
    /**
     * Voice allocation order:
     *
     * 1. Use an inactive voice.
     * 2. Use a releasing voice with the lowest envelope level.
     * 3. Steal the oldest active voice.
     *
     * Ties are resolved by choosing the highest storage index
     * for deterministic behavior.
     */

    std::size_t i, j;

    // 1. find an inactive voce
    for (Voice& voice: voices_) {
        if
        (
            !voice.isActive()
            &&
            !voice.isReleasing()
        )
        {
            return voice;
        }
    }

    // 2. find a releasing voice with the lowest envelope level
    // 　⇒ the selected voice is still active
    j = maxVoices_;
    float lowestLevel = 1.0f;
    for (i = 0; i < maxVoices_; ++i) {
        const Voice& voice = voices_[i];
        const bool isReleasing = voice.isReleasing();
        const float level = voice.getLevel();
        if (isReleasing && level <= lowestLevel) {
            j = i;
            lowestLevel = level;
        }
    }
    if (j < maxVoices_) {
        Voice& voice = voices_[j];
        voice.reset();
        return voice;
    }

    // 3. find the smallest key, which means the oldest
    // 　⇒ replace the next voice with the oldest one
    // 　⇒ the selected voice is still active
    j = maxVoices_;
    std::uint64_t oldestKey = 0xffff'ffff'ffff'ffffULL;
    for (i = 0; i < maxVoices_; ++i) {
        const Voice& voice = voices_[i];
        const std::uint64_t voiceKey = voice.getKey();
        if (voiceKey <= oldestKey) {
            j = i;
            oldestKey = voiceKey;
        }
    }
    Voice& voice = voices_[j];
    voice.reset();
    return voice;
}  // VoiceManager::allocate

void VoiceManager::render(
    float* left,
    float* right,
    const std::uint32_t startFrame,
    const std::uint32_t endFrame
) noexcept
{
    for (Voice& voice : voices_) {
        if (voice.isActive()) {
            voice.render(
                left,
                right,
                startFrame,
                endFrame
            );
        }
    }
}  // VoiceManager::render

void VoiceManager::noteOff(
    const std::uint64_t voiceKey
) noexcept
{
    Voice* voice = findVoice(voiceKey);
    if (voice != nullptr) {
        assert
        (
            voice->isActive()
            &&
            !voice->isReleasing()
        );
        voice->noteOff();
    }
}  // VoiceManager::noteOff

Voice* VoiceManager::findVoice(
    const std::uint64_t voiceKey
) noexcept
{
    for (Voice& voice : voices_) {
        if (voice.getKey() == voiceKey) {
            return &voice;
        }
    }
    return nullptr;
}  // VoiceManager::findVoice

void VoiceManager::reset() noexcept
{
    for (Voice& voice : voices_) {
        voice.reset();
    }
}  // VoiceManager::reset

std::uint8_t VoiceManager::activeVoiceCount()
noexcept
{
    std::uint8_t res = 0;
    for (Voice& voice : voices_) {
        if (voice.isActive()) res++;
    }
    return res;
}  // VoiceManager::activeVoiceCount

bool VoiceManager::isVoiceActive(
    const std::uint64_t voiceKey
) noexcept
{
    Voice* voice = findVoice(voiceKey);
    if (voice != nullptr && voice->isActive()) {
        return true;
    }
    return false;
}  // VoiceManager::isVoiceActive

}  // namespace
