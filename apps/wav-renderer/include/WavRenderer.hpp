#pragma once

#include <cstdint>
#include <string>

#include "score_audio_studio/dsp/AudioEngine.hpp"

class WavRenderer {

public:
    using AudioEngine =
        score_audio_studio::dsp::AudioEngine;
    void renderWavFile(
        AudioEngine& engine,
        const std::string& path,
        std::uint32_t totalFrames
    );

};
