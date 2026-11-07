#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <set>
#include <stdexcept>

#include "score_audio_studio/dsp/AudioEngine.hpp"

namespace score_audio_studio::dsp {

void AudioEngine::prepare(
    const double sampleRate,
    const std::uint32_t maxBlockSize,
    const float masterGain,
    const std::vector<std::uint8_t> buffer
)
{
    if (sampleRate <= 0 || !std::isfinite(sampleRate)) {
        assert(false && "invalid sample rate");
        return;
    }
    sampleRate_ = sampleRate;
    if (maxBlockSize <= 0) {
        assert(false && "invalid block size");
        return;
    }
    maxBlockSize_ = maxBlockSize;
    setMasterGain(masterGain);
    loadPlaybackEvents(buffer);
    setInstruments(maxBlockSize_);
}  // AudioEngine::prepare

void AudioEngine::setMasterGain(
    const float masterGain
) noexcept
{
    if (masterGain < 0.0f || 1.0f < masterGain) {
        assert(false && "invalid master gain");
        return;
    }
    masterGain_ = masterGain;
}  // AudioEngine::setMasterGain

void AudioEngine::loadPlaybackEvents(
    std::span<const std::uint8_t> buffer
)
{
    runtime_.prepare(
        std::span<const std::uint8_t>(
            buffer.data(),
            buffer.size()
        )
    );
}  // AudioEngine::setPlaybackEvents

void AudioEngine::setInstruments(
    const std::size_t channelSize
) noexcept
{
    const auto events = runtime_.getEvents();
    const auto instrumentPresets = getInstrumentPresets();

    if (events.size() == 0) {
        return;
    }

    std::set<std::uint16_t> usedIndices{};

    for (const PlaybackEvent& event : events) {
        /**
         * While developing `AudioEngine.cpp`,
         * I just realized that part model in phase 1 `score-domain`
         * should have instrument type.
         *
         * I will use partId temporarily instead of it.
         */
        std::uint16_t index = event.partId;

        if (instrumentPresets.size() < index) {
            assert(false && "invalid instrument");
            return;
        }

        if (usedIndices.find(index)
            !=
            usedIndices.end()) {
            continue; // already registered instrument preset
        }

        usedIndices.insert(index);

        instruments_.push_back(
            new Instrument(
                instrumentPresets[index], channelSize, sampleRate_
            )
        );
    }
}  // AudioEngine::setInstrument

void AudioEngine::reset() noexcept
{
    for (Instrument* instrument : instruments_) {
        delete instrument;
    }
    instruments_.clear();
}  // AudioEngine::reset

void AudioEngine::process(
    float* left,
    float* right,
    std::uint32_t frameCount,
    std::uint32_t blockStartFrame
) noexcept
{
    const auto events = runtime_.eventsForBlock(
        blockStartFrame,
        frameCount
    );
    std::uint32_t sampleOffset;
#ifndef NDEBUG
    std::size_t i = 0;
    assert(frameCount <= maxBlockSize_);
    assert(frameCount == 0 || (left != nullptr && right != nullptr));
    for (i = 0; i < events.size(); i++) {
        const PlaybackEvent& event = events[i];
        assert(blockStartFrame <= event.samplePosition);
        sampleOffset = event.samplePosition - blockStartFrame;
        assert(sampleOffset < frameCount);
        if (0 < i) {
            assert(
                events[i - 1].samplePosition
                <=
                event.samplePosition
            );
        }
    }
#endif
    if (0 < frameCount) {
        std::fill_n(left, frameCount, 0.0f);
        std::fill_n(right, frameCount, 0.0f);
    }
    std::uint32_t frameCursor = 0;
    for (const PlaybackEvent& event : events) {
        sampleOffset = event.samplePosition - blockStartFrame;
        renderVoices(
            left,
            right,
            frameCursor,
            sampleOffset
        );
        applyPlaybackEvent(event);
        frameCursor = sampleOffset;
    }
    renderVoices(
        left,
        right,
        frameCursor,
        frameCount
    );
}  // AudioEngine::process

void AudioEngine::renderVoices(
    float* left,
    float* right,
    const std::uint32_t startFrame,
    const std::uint32_t endFrame
) noexcept
{
    std::size_t i;
    for (Instrument* instrument : instruments_) {
        InstSettings& settings = instrument->getSettings();
        std::fill_n(
            settings.scratchLeft.data() + startFrame,
            endFrame - startFrame,
            0.0f
        );
        std::fill_n(
            settings.scratchRight.data() + startFrame,
            endFrame - startFrame,
            0.0f
        );
        // pan will be applied later commit
        settings.voiceManager.render(
            settings.scratchLeft.data(),
            settings.scratchRight.data(),
            startFrame,
            endFrame
        );
        for (i = startFrame; i < endFrame; ++i) {
            left[i]  += settings.scratchLeft[i]  * masterGain_;
            right[i] += settings.scratchRight[i] * masterGain_;
        }
    }
}  // AudioEngine::renderVoices

void AudioEngine::applyPlaybackEvent(
    const PlaybackEvent& event
) noexcept
{
    /**
     * startFrame and endFrame are block-relative frame offsets.
     *
     * In stereo output, frame i corresponds to
     * left[i] and right[i].
     */
    /**
     * While developing `AudioEngine.cpp`,
     * I just realized that part model in phase 1 `score-domain`
     * should have instrument type.
     *
     * I will use partId temporarily instead of it.
     */
    const std::uint16_t index = event.partId;
    Instrument* instrument = instruments_[index - 1];
    InstSettings& settings = instrument->getSettings();
    VoiceManager& voiceManager = settings.voiceManager;

    const std::uint32_t noteId = event.noteId;
    switch (event.type) {
        case PlaybackEventType::noteOff:
            voiceManager.noteOff(noteId);
            break;
        case PlaybackEventType::noteOn:
            voiceManager.noteOn(
                noteId,
                event.midiNoteNumber,
                event.velocity
            );
            break;
        default: // something went wrong
            assert(false && "corrupted PlaybackEventType");
            std::terminate();
    }
}  // AudioEngine::applyPlaybackEvent

double AudioEngine::getSampleRate() const noexcept
{
    return sampleRate_;
}  // AudioEngine::getSampleRate

std::uint32_t AudioEngine::getMaxBlockSize() const noexcept
{
    return maxBlockSize_;
}  // AudioEngine::getMaxBlockSize

void AudioEngine::rewind() noexcept
{
    runtime_.rewind();
    for (Instrument* instrument : instruments_) {
        instrument->getSettings().voiceManager.reset();
    }
}  // AudioEngine::rewind

}  // namespace score_audio_studio::dsp
