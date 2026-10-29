#include <cassert>
#include <string>

#include "score_audio_studio/dsp/Instruments.hpp"

/** [Path to the instrument preset JSON files]
 *  score-audio-studio/packages/dsp-core/config
 */

namespace {

  using namespace score_audio_studio::dsp;

  void instrumentTest() {

    assert(instruments.size() == 4);

    OscSettings oscSettings;
    AdsrSettings adsrSettings;

    for (const Instrument& instrument : instruments) {

      assert(!instrument.id.empty());
      assert(0 < instrument.numVoices);
      assert(
        instrument.voiceManager.activeVoiceCount()
        == 0
      );

      oscSettings = instrument.oscSettings;
      
      int iWaveType = static_cast<int>(
        oscSettings.waveType
      );
      int iAllowAliasing = static_cast<int>(
        oscSettings.allowAliasing
      );

      assert(0 <= iWaveType && iWaveType <= 3);
      assert(0 < oscSettings.numHarmonics);
      assert(
        iAllowAliasing == 0
        ||
        iAllowAliasing == 1
      );

      adsrSettings = instrument.adsrSettings;

      assert(0.0 <= adsrSettings.attack.seconds);
      assert(0.0 <= adsrSettings.decay.seconds);
      assert(0.0 <= adsrSettings.sustainSeconds);
      assert(0.0 <= adsrSettings.releaseSeconds);

      assert(0.0f <= adsrSettings.attack.targetGain);
      assert(0.0f <= adsrSettings.decay.targetGain);

      int iCurve = static_cast<int>(
        adsrSettings.curve
      );

      assert(0 <= iCurve && iCurve <= 2);

      assert(
        0.0f <= instrument.pan
        &&
        instrument.pan <= 1.0f
      );

      assert(
        0.0f <= instrument.level
        &&
        instrument.level <= 1.0f
      );

      assert(instrument.scratchLeft.size() == 0);
      assert(instrument.scratchRight.size() == 0);

    }

  }

} // namespace

int main() {
  instrumentTest();
  return 0;
}
