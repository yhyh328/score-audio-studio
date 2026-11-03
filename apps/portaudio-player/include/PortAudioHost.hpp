#pragma once

#include <cstdint>
#include <portaudio.h>
#include <vector>

#include "score_audio_studio/dsp/AudioEngine.hpp"

class PortAudioHost {

using AudioEngine = score_audio_studio::dsp::AudioEngine;

public:
    void prepare(
        AudioEngine& engine,
        const double sampleRate,
        const std::uint32_t maxBlockSize,
        const float masterGain,
        const std::vector<std::uint8_t> buffer
    );
    void play();
    void stop();

private:
    AudioEngine* engine_{nullptr};
    PaStream* stream_{nullptr};
    bool portaudioInitialized_{false};

    const int numChannels_{2}; // engine renders stereo non-interleaved
    unsigned long frameCount_{256};
    std::uint32_t blockStartFrame_{0};

    static int streamCallback(
        const void *input,
        void *output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData
    );

};
