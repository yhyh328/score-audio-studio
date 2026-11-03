#include <cstdint>
#include <portaudio.h>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>

#include "PortAudioHost.hpp"

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
    if (stream_ != nullptr) {
        throw std::runtime_error(
            "Stream is already open."
        );
    }

    PaError err;
    
    err = Pa_Initialize();
    if (err != paNoError) {
        throw std::runtime_error(
            "Failed to initialize PortAudio: "
            +
            std::string(Pa_GetErrorText(err))
        );
    }

    portaudioInitialized_ = true;
    
    blockStartFrame_ = 0;

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

    PaStreamParameters outputParams;
    memset(&outputParams, 0, sizeof(outputParams));
    outputParams.device = outputDevice;
    outputParams.channelCount = numChannels_;
    outputParams.sampleFormat = paFloat32 | paNonInterleaved;
    outputParams.suggestedLatency =
        Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream(
        &stream_, nullptr, &outputParams, engine_->getSampleRate(),
        frameCount_, paNoFlag, &streamCallback, this
    );
    if (err != paNoError) {
        Pa_Terminate();
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

void PortAudioHost::stop()
{
    PaError activeError, closeError, stopError, terminateError;
    PaError result = paNoError;

    if (stream_ != nullptr) {
        activeError = Pa_IsStreamActive(stream_);
        if (activeError == 1) {
            stopError = Pa_StopStream(stream_);
            if (stopError != paNoError) {
                result = stopError;
            }
        } else if (activeError < 0) {
            result = activeError;
        }
        closeError = Pa_CloseStream(stream_);
    }

    stream_ = nullptr;

    if (result == paNoError && closeError != paNoError) {
        result = closeError;
    }

    if (portaudioInitialized_) {
        terminateError = Pa_Terminate();
        portaudioInitialized_ = false;
        if (result == paNoError && terminateError != paNoError) {
            result = terminateError;
        }
    }

    if (result != paNoError) {
        throw std::runtime_error(
            "Failed to stop audio: "
            +
            std::string(Pa_GetErrorText(result))
        );
    }
}  // PortAudioHost::stop
