#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>

#include "score_audio_studio/dsp/Instruments.hpp"

/** [Path to the instrument preset JSON files]
 *  score-audio-studio/packages/dsp-core/config
 */

namespace {

  using namespace score_audio_studio::dsp;

  constexpr std::uint32_t kFrameCount = 512;
  constexpr double kSampleRate = 48000.0;

  constexpr std::array<PlaybackEvent, 10> events{{
    {
      PlaybackEventType::noteOn,
      60,             // midiNoteNumber
      1,              // partId
      1,              // noteId
      1,              // samplePosition
      72.0f / 127.0f  // velocity
    },
    {
      PlaybackEventType::noteOn,
      61,             // midiNoteNumber
      1,              // partId
      2,              // noteId
      2,              // samplePosition
      72.0f / 127.0f  // velocity
    },
    {
      PlaybackEventType::noteOn,
      62,             // midiNoteNumber
      1,              // partId
      3,              // noteId
      3,              // samplePosition
      72.0f / 127.0f  // velocity
    },
    {
      PlaybackEventType::noteOn,
      63,             // midiNoteNumber
      1,              // partId
      4,              // noteId
      4,              // samplePosition
      72.0f / 127.0f  // velocity
    },
    {
      PlaybackEventType::noteOn,
      64,             // midiNoteNumber
      1,              // partId
      5,              // noteId
      5,              // samplePosition
      72.0f / 127.0f  // velocity   
    },
    {
      PlaybackEventType::noteOff,
      60,             // midiNoteNumber
      1,              // partId
      1,              // noteId
      kFrameCount - 1, // samplePosition
      0.0f            // velocity
    },
    {
      PlaybackEventType::noteOff,
      61,             // midiNoteNumber
      1,              // partId
      2,              // noteId
      kFrameCount - 1, // samplePosition
      0.0f            // velocity
    },
    {
      PlaybackEventType::noteOff,
      62,             // midiNoteNumber
      1,              // partId
      3,              // noteId
      kFrameCount - 1, // samplePosition
      0.0f            // velocity
    },
    {
      PlaybackEventType::noteOff,
      63,             // midiNoteNumber
      1,              // partId
      4,              // noteId
      kFrameCount - 1, // samplePosition
      0.0f            // velocity
    },
    {
      PlaybackEventType::noteOff,
      64,             // midiNoteNumber
      1,              // partId
      5,              // noteId
      kFrameCount - 1, // samplePosition
      0.0f            // velocity
    },
  }};

  const std::vector<InstSettings>& instrumentPresets =
    getInstrumentPresets();

  void loadInstrumentSettings() {

    assert(instrumentPresets.size() == 4);

    OscSettings oscSettings;
    AdsrSettings adsrSettings;

    for (const InstSettings& instPreset : instrumentPresets) {

      assert(!instPreset.id.empty());
      assert(0 < instPreset.numVoices);
      assert(
        instPreset.voiceManager.activeVoiceCount() == 0
      );

      oscSettings = instPreset.oscSettings;

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

      adsrSettings = instPreset.adsrSettings;

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
        0.0f <= instPreset.pan
        &&
        instPreset.pan <= 1.0f
      );

      assert(
        0.0f <= instPreset.level
        &&
        instPreset.level <= 1.0f
      );

      assert(instPreset.scratchLeft.size() == 0);
      assert(instPreset.scratchRight.size() == 0);
    }

  }

  void instrumentTest() {
    /** Instrument information
     * id: Viola
     * numVoices: 4
     */

    /**
     * While developing `AudioEngine.cpp`,
     * I just realized that part model in phase 1 `score-domain`
     * should have instrument type.
     *
     * I will use partId temporarily instead of it.
     */
    const std::uint16_t partId = 1; // Temporary part ID
    
    Instrument* inst = new Instrument(
      instrumentPresets[partId],
      kFrameCount,
      kSampleRate
    );
    
    InstSettings& settings = inst->getSettings();

    assert(settings.voiceManager.activeVoiceCount() == 0);
    
    assert(settings.scratchLeft.size()  == kFrameCount);
    assert(settings.scratchRight.size() == kFrameCount);

    std::size_t i;
    std::uint32_t noteId;
    PlaybackEvent event;
    std::uint8_t prvActiveVoiceCount,
                 curActiveVoiceCount;
    std::uint32_t remainingFrames,
                  frameCounts;
    VoiceManager voiceManager = settings.voiceManager;
    for (i = 0; i < events.size(); ++i) {
      event = events[i];
      noteId = event.noteId;
      prvActiveVoiceCount = voiceManager.activeVoiceCount();
      switch (event.type) {
        case PlaybackEventType::noteOff:
          voiceManager.noteOff(noteId);
          curActiveVoiceCount = voiceManager.activeVoiceCount();
          assert(
            prvActiveVoiceCount == curActiveVoiceCount
          );
          remainingFrames = static_cast<std::uint32_t>(
            std::round(
              kSampleRate
              *
              settings.adsrSettings.releaseSeconds
            )
          );
          while(0 < remainingFrames) {
            frameCounts = std::min(
              remainingFrames, kFrameCount
            );
            voiceManager.render(
              settings.scratchLeft.data(),
              settings.scratchRight.data(),
              0,
              frameCounts
            );
            remainingFrames -= frameCounts;
          }
          curActiveVoiceCount = voiceManager.activeVoiceCount();
          if (event.noteId == 1) { // already removed from active voices
            assert(
              prvActiveVoiceCount == curActiveVoiceCount
            );
          }
          else {
            assert(
              prvActiveVoiceCount - 1 == curActiveVoiceCount
            );
          }
          break;
        case PlaybackEventType::noteOn:
          voiceManager.noteOn(
            noteId,
            event.midiNoteNumber,
            event.velocity
          );
          curActiveVoiceCount = voiceManager.activeVoiceCount();
          if (i < settings.numVoices) {
            assert(
              curActiveVoiceCount == i + 1
            );
          }
          else {
            assert(
              prvActiveVoiceCount == curActiveVoiceCount
            );
          }
          break;
        default:
          assert(false && "corrupted PlaybackEventType");
      }
      if (i == settings.numVoices - 1) {  // fifth event is also noteOn
        assert(
          voiceManager.activeVoiceCount() == settings.numVoices
        );
      }
    }
    delete inst;
  }

} // namespace

int main() {
  loadInstrumentSettings();
  instrumentTest();
  return 0;
}
