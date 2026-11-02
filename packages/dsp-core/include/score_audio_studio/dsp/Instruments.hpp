#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "score_audio_studio/playback_runtime/PlaybackEvent.hpp"
#include "score_audio_studio/dsp/VoiceManager.hpp"

namespace score_audio_studio::dsp {

struct InstSettings
{
    std::string id;
    std::size_t numVoices;
    VoiceManager voiceManager;
    OscSettings oscSettings;
    AdsrSettings adsrSettings;
#if 0 // TODO: Implement after the required effect components are available.
    EffectChain effectChain;
#endif
    float pan;
    float level;
    std::vector<float> scratchLeft;
    std::vector<float> scratchRight;
};

bool isValidVal(
    const int val,
    const int boundary
) noexcept;
bool isValidVal(
    const float val
) noexcept;
bool isValidVal(
    const double val
) noexcept;

WaveType setWaveType(
    const std::string val
) noexcept;
EnvelopeCurve setCurve(
    const std::string val
) noexcept;

[[nodiscard]] const std::vector<InstSettings>&
getInstrumentPresets();

class Instrument {

public:
    Instrument(
        const InstSettings& preset,
        const std::size_t channelSize,
        const double sampleRate
    );
    ~Instrument();

    [[nodiscard]]
    InstSettings& getSettings() noexcept;

private:
    InstSettings settings_{};

};

} // namespace score_audio_studio::dsp
