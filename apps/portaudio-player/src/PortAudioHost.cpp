#include <cstdint>
#include <portaudio.h>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>

#include "PortAudioHost.hpp"

PortAudioHost::PortAudioHost()
{
    const PaError err = Pa_Initialize();
    if (err != paNoError) {
        throw std::runtime_error(
            "Failed to initialize PortAudio: "
            +
            std::string(Pa_GetErrorText(err))
        );
    }
    PaDeviceIndex outputDevice = Pa_GetDefaultOutputDevice();
    if (outputDevice == paNoDevice) {
        throw std::runtime_error(
            "No default output device found: "
            +
            std::string(Pa_GetErrorText(err))
        );
    }
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(outputDevice);
    if (deviceInfo == nullptr) {
        throw std::runtime_error(
            "Failed to get device info: "
            +
            std::string(Pa_GetErrorText(err))
        );
    }
    std::cout << "Using output device: " << deviceInfo->name << std::endl;
    std::cout << "Max output channels: " << deviceInfo->maxOutputChannels << std::endl;

    memset(&outputParams_, 0, sizeof(outputParams_));
    outputParams_.device = outputDevice;
    outputParams_.channelCount = numChannels_;
    outputParams_.sampleFormat = paFloat32 | paNonInterleaved;
    outputParams_.suggestedLatency =
        Pa_GetDeviceInfo(outputParams_.device)->defaultLowOutputLatency;
    outputParams_.hostApiSpecificStreamInfo = nullptr;

    portaudioInitialized_ = true;
}

PortAudioHost::~PortAudioHost() noexcept
{
    if (stream_ != nullptr) {
        if (Pa_IsStreamActive(stream_) == 1) {
            Pa_StopStream(stream_);
        }
        Pa_CloseStream(stream_);
        stream_ = nullptr;
    }
    if (portaudioInitialized_) {
        Pa_Terminate();
    }
}

void PortAudioHost::prepare(
    AudioEngine& engine,
    const double sampleRate,
    const std::uint32_t maxBlockSize,
    const float masterGain,
    const std::vector<std::uint8_t> buffer
)
{
    engine.prepare(
        sampleRate,
        maxBlockSize,
        masterGain,
        buffer
    );
    engine_ = &engine;
    if (engine_ == nullptr) {
        throw std::runtime_error(
            "Audio engine not prepared."
        );
    }
}  // PortAudioHost::prepare

void PortAudioHost::play()
{
    PaError err;
    if (stream_ != nullptr) {
        if (!paused_) {
            throw std::runtime_error(
                "Stream is already playing."
            );
        }
        err = Pa_StartStream(stream_);
        if (err != paNoError) {
            throw std::runtime_error(
                "Failed to resume stream: "
                +
                std::string(Pa_GetErrorText(err))
            );
        }
        paused_ = false;
        return;
    }
    err = Pa_OpenStream(
        &stream_, nullptr, &outputParams_,
        engine_->getSampleRate(), frameCount_,
        paNoFlag, &streamCallback, this
    );
    if (err != paNoError) {
        throw std::runtime_error(
            "Failed to open stream: "
            +
            std::string(Pa_GetErrorText(err))
        );
    }
    err = Pa_StartStream(stream_);
    if (err != paNoError) {
        Pa_CloseStream(stream_);
        stream_ = nullptr;
        Pa_Terminate();
        throw std::runtime_error(
            "Failed to start stream: "
            +
            std::string(Pa_GetErrorText(err))
        );
    }
}  // PortAudioHost::play

int PortAudioHost::streamCallback(
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
)
{
    // no use
    (void) input;
    (void) timeInfo;
    (void) statusFlags;

    PortAudioHost* host = static_cast<PortAudioHost*>(userData);

    float** channels = static_cast<float**>(output);

    float* left  = channels[0];
    float* right = channels[1];

    const std::uint32_t uint32FrameCount =
        static_cast<std::uint32_t>(frameCount);

    host->engine_->process(
        left,
        right,
        uint32FrameCount,
        host->blockStartFrame_
    );

    host->blockStartFrame_ += uint32FrameCount;

    return paContinue;
}  // PortAudioHost::paCallback

void PortAudioHost::pause()
{
    if (stream_ == nullptr || paused_) {
        return;
    }
    const PaError err = Pa_StopStream(stream_);
    if (err != paNoError) {
        throw std::runtime_error(
            "Failed to puase stream: "
            +
            std::string(Pa_GetErrorText(err))
        );
    }
    paused_ = true;
}  // PortAudioHost::pause

void PortAudioHost::stop()
{
    PaError err;

    if (stream_ != nullptr) {
        err = Pa_IsStreamActive(stream_);
        if (err == 1) {
            err = Pa_StopStream(stream_);
            if (err != paNoError) {
                throw std::runtime_error(
                    "Failed to stop stream"
                    +
                    std::string(Pa_GetErrorText(err))
                );
            }
        }
        err = Pa_CloseStream(stream_);
        stream_ = nullptr;
        if (err != paNoError) {
            throw std::runtime_error(
                "Failed to close stream: "
                +
                std::string(Pa_GetErrorText(err))
            );
        }
    }

    paused_ = false;
    blockStartFrame_ = 0;

    if (engine_ != nullptr) {
        engine_ -> rewind();
    }
}  // PortAudioHost::stop
