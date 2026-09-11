#include <span>

#include "score_audio_studio/dsp/LibrarySmokeTest.hpp"

int main()
{
    const int values[] = { 1, 2, 3 };
    const std::span<const int> view{values};

    if (view.size() != 3) {
        return 1;
    }
    
    if (score_audio_studio::dsp::smokeTestValue() != 1) {
        return 2;
    }

    return 0;
}