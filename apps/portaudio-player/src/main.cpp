#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "PortAudioHost.hpp"
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
}

void help(std::ostream& output)
{
    output << "Usage: portaudio_player [--help]\n"
           << "\n"
           << "Commands:\n"
           << "  1      play        Start playback.\n"
           << "  2      pause       Pause Playback.\n"
           << "  3      stop        Stop playback.\n"
           << "  4      quit        Stop playback and exit.\n"
           << "\n"
           << "Options:\n"
           << "  -h, --help  Show this help message.\n";
}

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
    PortAudioHost host;

    constexpr double kSampleRate = 48000.0;
    constexpr std::uint32_t kMaxBlockSize = 1024;
    constexpr float kMasterGain = 0.1f;
    const std::vector<std::uint8_t> buffer = runPlaybackCompiler();

    host.prepare(
        engine,
        kSampleRate,
        kMaxBlockSize,
        kMasterGain,
        buffer
    );

    help(std::cout);

    bool isPlaying = false,
         isPaused  = false;
    std::string command;
    while (std::getline(std::cin, command)) {
        if (
            command == "1" || command == "play"
        ) {
            if (!isPlaying || isPaused) {
                host.play();
                isPlaying = true;
                isPaused  = false;
            }
        }
        else if (
            command == "2" || command == "pause"
        ) {
            if (isPlaying && !isPaused) {
                host.pause();
                isPaused = true;
            }
        }
        else if (
            command == "3" || command == "stop"
        ) {
            if (isPlaying) {
                host.stop();
                isPlaying = false;
                isPaused  = false;
            }
        }
        else if (
            command == "4" || command == "quit"
        ) {
            if (isPlaying) {
                host.stop();
            }
            break;
        }
        else {
            std::cout << "Invalid command.\n";
            help(std::cout);
        }
    }

    if (isPlaying || isPaused) {
        host.stop();
    }
    engine.reset();

    return 0;
}
