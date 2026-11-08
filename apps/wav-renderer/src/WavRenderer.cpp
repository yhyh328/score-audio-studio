#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <fstream>
#include <stdexcept>

#include "WavRenderer.hpp"
#include "score_audio_studio/dsp/AudioEngine.hpp"

namespace {

void writeUint16(
    std::ofstream& file,
    std::uint16_t value
)
{
    file.write(
        reinterpret_cast<const char*>(&value),
        sizeof(value)
    );
}

void writeUint32(
    std::ofstream& file,
    std::uint32_t value
)
{
    file.write(
        reinterpret_cast<const char*>(&value),
        sizeof(value)
    );
}

void writeWavHeader(
    std::ofstream& file,
    std::uint32_t sampleRate,
    std::uint32_t numFrames
)
{
    constexpr std::uint16_t numChannels = 2;
    constexpr std::uint16_t bitsPerSample = 16;
    
    const std::uint32_t dataSize =
        numFrames *
        numChannels *
        (bitsPerSample / 8);

    file.write("RIFF", 4);

    writeUint32(
        file,
        36 + dataSize
    );

    file.write("WAVE", 4);

    file.write("fmt ", 4);

    writeUint32(file, 16);
    writeUint16(file, 1);

    writeUint16(
        file,
        numChannels
    );

    writeUint32(
        file,
        sampleRate
    );

    writeUint32(
        file,
        sampleRate *
        numChannels *
        bitsPerSample / 8
    );

    writeUint16(
        file,
        numChannels *
        bitsPerSample / 8
    );

    writeUint16(
        file,
        bitsPerSample
    );

    file.write("data", 4);

    writeUint32(
        file,
        dataSize
    );
}

} // namespace

void WavRenderer::renderWavFile(
    AudioEngine& engine,
    const std::string& path,
    const std::uint32_t totalFrames
)
{
    constexpr std::uint32_t blockSize = 512;

    std::ofstream file(
        path,
        std::ios::binary
    );
    if (!file) {
        throw std::runtime_error("Failed to open wav file.");
    }

    writeWavHeader(
        file,
        static_cast<std::uint32_t>(engine.getSampleRate()),
        totalFrames
    );

    float left[blockSize];
    float right[blockSize];

    std::uint32_t frame = 0;

    while (frame < totalFrames) {

        const std::uint32_t currentBlock =
            std::min(blockSize, totalFrames - frame);

        engine.process(
            left, right, currentBlock, frame
        );

        for (std::size_t i = 0; i < currentBlock; i++) {
            const float l =
                std::clamp(left[i], -1.0f, 1.0f);
            const float r =
                std::clamp(right[i], -1.0f, 1.0f);
            const std::int16_t pcmL =
                static_cast<std::int16_t>(l * 32767.0f);
            const std::int16_t pcmR =
                static_cast<std::int16_t>(r * 32767.0f);
            file.write(
                reinterpret_cast<const char*>(&pcmL),
                sizeof(pcmL)
            );
            file.write(
                reinterpret_cast<const char*>(&pcmR),
                sizeof(pcmR)
            );
        }

        frame += currentBlock;
    }

} // renderWavFile
