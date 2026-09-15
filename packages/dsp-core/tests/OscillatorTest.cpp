#include <iostream>
#include <cassert>

#include "score_audio_studio/dsp/Oscillator.hpp"

int main()
{
    using namespace score_audio_studio::dsp;
    
    constexpr double sampleRate = 44100.0;

    Oscillator osc;

    osc.prepare(sampleRate); // Sine wave
    osc.setFrequency(72); // C5 is converted to 523.25... Hz
    
    std::cout << "Rendering samples..." << std::endl;
    for (std::uint32_t i = 0; i < 64; ++i) { // kMaxHarmsNum is 64
        double sample = osc.renderSample();
        std::cout << "Frame " << i + 1 << ": " << sample << std::endl;
        assert(
            std::isfinite(sample) && "Sample value is not finite"
        );
        assert(
            -1.0 <= sample && sample <= 1.0 && "Sample value is out of range [-1.0, 1.0]"
        );
    }
    std::cout << std::endl;
}
