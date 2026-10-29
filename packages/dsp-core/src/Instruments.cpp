#include "score_audio_studio/dsp/Instruments.hpp"

namespace score_audio_studio::dsp {

bool isValidVal(
    const int val,
    const int boundary
) noexcept
{
    return (0 < val && val <= boundary);
}

bool isValidVal(
    const float val
) noexcept
{
    return (0.0f <= val && val <= 1.0f);
}

bool isValidVal(
    const double val
) noexcept
{
    return (0.0 <= val);
}

WaveType setWaveType(
    const std::string val
) noexcept
{
    if (val == "Sine") {
        return WaveType::Sine;
    }
    if (val == "Triangle") {
        return WaveType::Triangle;
    }
    if (val == "Square") {
        return WaveType::Square;
    }
    if (val == "Sawtooth") {
        return WaveType::Sawtooth;
    }
    return WaveType::Sine;
}

EnvelopeCurve setCurve(
    const std::string val
) noexcept
{
    if (val == "None") {
        return EnvelopeCurve::None;
    }
    if (val == "Linear") {
        return EnvelopeCurve::Linear;
    }
    if (val == "Exponential") {
        return EnvelopeCurve::Exponential;
    }
    return EnvelopeCurve::None;
}

namespace {

const std::vector<Instrument> loadInstruments()
{
    std::vector<Instrument> instruments;

    /**
     * [Default Instruments]
     *
     * score-audio-studio/packages/dsp-core/config/default-instruments.json
     *
     * Violin (index = 0)
     * Viola (index = 1)
     * Violoncello (index = 2)
     * Acoustic Grand Piano (index = 3)
     *
     * [Custom Instruments]
     *
     * score-audio-studio/packages/dsp-core/config/custom-instruments.json
     *
     * might be saved in Phase 4
     */
    const std::filesystem::path configDir =
        std::filesystem::path(PROJECT_CONFIG_DIR);
    const std::string kFilePaths[] = {
        (configDir / "default-instruments.json").string(),
        (configDir / "custom-instruments.json").string()
    };
    const int supportedVersions[] = {
        1
    };
    const std::string waveTypes[] = {
        "Sine",
        "Triangle",
        "Square",
        "Sawtooth"
    };
    const std::string envelopeCurves[] = {
        "None",
        "Linear",
        "Exponential"
    };
    const int kMaxVoicesNum = 16;
    const int kMaxHarmonicsNum = 64;

    for (const auto& filePath : kFilePaths) {
        if (!std::filesystem::exists(filePath)) {
            throw std::runtime_error(
                "File path doesn't exist: " + filePath
            );
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error(
                "Failed to open file: " + filePath
            );
        }

        try {
            nlohmann::json document;
            file >> document;
            if (!document.is_object()) {
                throw std::runtime_error(
                    "Unsupported json format: " + filePath
                );
            }

            const int schemaVersion =
                document.at("schemaVersion").get<int>();
            if (std::ranges::find(supportedVersions, schemaVersion)
                ==
                std::end(supportedVersions)) {
                throw std::runtime_error(
                    "Unsupported schema version: " + filePath +
                    " -> " + std::to_string(schemaVersion)
                );
            }

            const auto& instrumentPresets =
                document.at("instrumentPresets");
            if (!instrumentPresets.is_array()) {
                throw std::runtime_error(
                    "instrumentPresets must be an array: " + filePath
                );
            }

            for (const auto& item : instrumentPresets) {

//////////////////////////////////////////////////////////////////
                const int numVoices = // int -> std::size_t
                    item["numVoices"].get<int>();
                if (!isValidVal(numVoices, kMaxVoicesNum)) {
                    throw std::runtime_error(
                        "Invalid voices number: " + filePath +
                        " -> " +
                        std::to_string(numVoices) + "\n" +
                        "0 < numVoices <= " +
                        std::to_string(kMaxVoicesNum)
                    );
                }

//////////////////////////////////////////////////////////////////
                const auto& oscSettings =
                    item["oscSettings"];

                const std::string waveType = // str -> WaveType
                    oscSettings["waveType"].get<std::string>();

                if (waveType != "Sine"      &&
                    waveType != "Triangle"  &&
                    waveType != "Square"    &&
                    waveType != "Sawtooth") {
                    throw std::runtime_error(
                        "Invalid wave type: " +
                        filePath + " -> " +
                        waveType + "\n" +
                        "valid wave types: " +
                        R"("Sine", "Triangle", "Square", "Sawtooth")"
                    );
                }

                const int numHarmonics = // int -> std::uint8_t
                    oscSettings["numHarmonics"].get<int>();
                if (!isValidVal(numHarmonics, kMaxHarmonicsNum)) {
                    throw std::runtime_error(
                        "Invalid harmonics number: " +
                        filePath + " -> " +
                        std::to_string(numHarmonics) + "\n" +
                        "0 < numHarmonics <= " +
                        std::to_string(kMaxHarmonicsNum)
                    );
                }

                const bool allowAliasing =
                    oscSettings["allowAliasing"].get<bool>();

//////////////////////////////////////////////////////////////////
                const auto& adsrSettings =
                    item["adsrSettings"];

                const auto& attack =
                    adsrSettings["attack"];

                const double attackSeconds =
                    attack["seconds"].get<double>();
                if (!isValidVal(attackSeconds)) {
                    throw std::runtime_error(
                        "Invalid seconds: " +
                        filePath + " -> " +
                        std::to_string(attackSeconds) + "\n" +
                        "0.0 <= attackSeconds <= 1.0f"
                    );
                }

                const float attackTargetGain =
                    attack["targetGain"].get<float>();
                if (!isValidVal(attackTargetGain)) {
                    throw std::runtime_error(
                        "Invalid gain: " +
                        filePath + " -> " +
                        std::to_string(attackTargetGain) + "\n" +
                        "0.0f <= attackTargetGain <= 1.0f"
                    );
                }

                const auto& decay =
                    adsrSettings["decay"];

                const double decaySeconds =
                    decay["seconds"].get<double>();
                if (!isValidVal(decaySeconds)) {
                    throw std::runtime_error(
                        "Invalid seconds: " +
                        filePath + " -> " +
                        std::to_string(decaySeconds) + "\n" +
                        "0.0 <= decaySeconds <= 1.0f"
                    );
                }

                const float decayTargetGain =
                    decay["targetGain"].get<float>();
                if (!isValidVal(decayTargetGain)) {
                    throw std::runtime_error(
                        "Invalid gain: " +
                        filePath + " -> " +
                        std::to_string(decayTargetGain) + "\n" +
                        "0.0f <= decayTargetGain <= 1.0f"
                    );
                }

                const double sustainSeconds =
                    adsrSettings["sustainSeconds"].get<double>();
                if (!isValidVal(sustainSeconds)) {
                    throw std::runtime_error(
                        "Invalid seconds: " +
                        filePath + " -> " +
                        std::to_string(sustainSeconds) + "\n" +
                        "0.0f <= sustainSeconds <= 1.0f"
                    );
                }

                const float releaseSeconds =
                    adsrSettings["releaseSeconds"].get<float>();
                if (!isValidVal(releaseSeconds)) {
                    throw std::runtime_error(
                        "Invalid seconds: " +
                        filePath + " -> " +
                        std::to_string(releaseSeconds) + "\n" +
                        "0.0 <= releaseSeconds <= 1.0f"
                    );
                }

                const std::string curve = // std::string -> EnvelopeCurve
                    adsrSettings["curve"].get<std::string>();
                if (curve != "None"         &&
                    curve != "Linear"       &&
                    curve != "Exponential") {
                    throw std::runtime_error(
                        "Invalid envelope curve" +
                        filePath + " -> " +
                        curve + "\n" +
                        "valid envelope curves: " +
                        R"("Sine", "Triangle", "Square", "Sawtooth")"
                    );
                }

//////////////////////////////////////////////////////////////////
                const float defaultPan =
                    item["defaultPan"].get<float>();
                if (!isValidVal(defaultPan)) {
                    throw std::runtime_error(
                        "Invalid pan value"
                    );
                }

//////////////////////////////////////////////////////////////////
                const float defaultLevel =
                    item["defaultLevel"].get<float>();
                if (!isValidVal(defaultLevel)) {
                    throw std::runtime_error(
                        "Invalid level"
                    );
                }

//////////////////////////////////////////////////////////////////
                Instrument instrument;
                instrument.id = item["id"].get<std::string>();
                instrument.numVoices =
                    static_cast<std::size_t>(numVoices);
                instrument.oscSettings = {
                    .waveType = setWaveType(waveType),
                    .numHarmonics = static_cast<std::uint8_t>(numHarmonics),
                    .allowAliasing = allowAliasing
                };
                instrument.adsrSettings = {
                    .attack = {
                        .seconds = attackSeconds,
                        .targetGain = attackTargetGain
                    },
                    .decay = {
                        .seconds = decaySeconds,
                        .targetGain = decayTargetGain
                    },
                    .sustainSeconds = sustainSeconds,
                    .releaseSeconds = releaseSeconds,
                    .curve = setCurve(curve)
                };

                instruments.push_back(std::move(instrument));
            }
        }
        catch (const nlohmann::json::exception& e) {
            throw std::runtime_error(
                "Parsing error: " + filePath + "\n" + e.what()
            );
        }
    }
    return instruments;
}

} // namespace

const std::vector<Instrument> instruments = loadInstruments();

} // namespace score_audio_studio::dsp
