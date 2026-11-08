#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "WavRenderer.hpp"
#include "score_audio_studio/dsp/AudioEngine.hpp"

namespace {

using AudioEngine = score_audio_studio::dsp::AudioEngine;

std::vector<std::uint8_t> runPlaybackCompiler()
{
    const std::filesystem::path scriptPath =
        std::filesystem::path(__FILE__).parent_path()
        / "../../../packages/dsp-core/tests/emitPlaybackBuffer.mjs";
    const std::string command =
        "node \"" + scriptPath.lexically_normal().string() + "\"";

#ifdef _WIN32
    FILE* process = _popen(command.c_str(), "rb");
#else
    FILE* process = popen(command.c_str(), "r");
#endif

    if (process == nullptr) {
        throw std::runtime_error("Failed to get playback buffer.");
    }

    std::vector<std::uint8_t> buffer;
    std::array<std::uint8_t, 256> chunk{};

    while (true) {
        const std::size_t count = std::fread(
            chunk.data(),
            sizeof(chunk[0]),
            chunk.size(),
            process
        );

        buffer.insert(
            buffer.end(),
            chunk.begin(),
            chunk.begin() + static_cast<std::ptrdiff_t>(count)
        );

        if (count < chunk.size()) {
            if (std::ferror(process) != 0) {
#ifdef _WIN32
                _pclose(process);
#else
                pclose(process);
#endif
                throw std::runtime_error(
                    "Failed to read playback buffer."
                );
            }
            break;
        }
    }

#ifdef _WIN32
    const int status = _pclose(process);
#else
    const int status = pclose(process);
#endif

    if (status != 0) {
        throw std::runtime_error("Playback compiler failed.");
    }

    return buffer;
}  // runPlaybackCompiler 

std::filesystem::path wavPath()
{
    // Prevention of the Integer-Overflow from 2038.
    static_assert(
        8 <= sizeof(std::time_t),
        "CSV timestamps require at least 64-bit time_t"
    );

    const std::filesystem::path dirPath =
        "./apps/wav-renderer/wav-evidences";
    std::filesystem::create_directories(dirPath);

    std::time_t now = std::time(nullptr);
    const std::tm* local = std::localtime(&now);
    char timestamp[16]; // "yyyymmdd_hhmmss"
    if (
        local == nullptr
        ||
        std::strftime(
            timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", local
        ) == 0
    ) {
        throw std::runtime_error("Cannot create timestamp");
    }

    std::string wavFileName = std::string(timestamp) + ".wav";
    return dirPath / wavFileName;
}  // wavPath

void help(std::ostream& output)
{
    output << "Usage: wav_renderer [--help]\n"
           << "\n"
           << "Options:\n"
           << "  -h, --help  Show this help message.\n";
}  // help

}  // namespace

int main(int argc, char* argv[])
{
    if (argc == 2) {
        const std::string_view arg(argv[1]);
        if (arg == "-h" || arg == "--help") {
            help(std::cout);
            return 0;
        }
        throw std::runtime_error("Invalid option: " + std::string(arg));
    }

    if (argc != 1) {
        throw std::runtime_error("Invalid options.");
    }

    AudioEngine engine;
    WavRenderer renderer;

    constexpr double kSampleRate = 48000.0;
    constexpr std::uint32_t kMaxBlockSize = 1024;
    constexpr float kMasterGain = 0.1f;

    constexpr double kRenderSeconds = 10.0;

    const std::vector<std::uint8_t> buffer = runPlaybackCompiler();

    engine.prepare(
        kSampleRate, kMaxBlockSize, kMasterGain, buffer
    ); 

    const std::filesystem::path path = wavPath();

    const auto totalFrames = static_cast<std::uint32_t>(
        kSampleRate * kRenderSeconds
    );

    renderer.renderWavFile(engine, path, totalFrames);

    std::cout << "Created output: " << path << std::endl;

    return 0;
}
