#!/usr/bin/env bash
set -euo pipefail

filter_audio_backend_probe_noise()
{
    sed -E \
        -e '/^ALSA lib /d' \
        -e '/^Cannot connect to server socket err = No such file or directory$/d' \
        -e '/^Cannot connect to server request channel$/d' \
        -e '/^jack server is not running or cannot be started$/d' \
        -e '/^JackShmReadWritePtr::~JackShmReadWritePtr - Init not done for -1, skipping unlock$/d'
}

if [[ "${PORTAUDIO_SHOW_BACKEND_PROBE_LOGS:-0}" == "1" ]]; then
    LSAN_OPTIONS="suppressions=$PWD/.lsan-suppressions:print_suppressions=0" \
    exec ./build/portaudio-player/portaudio_player "$@"
fi

LSAN_OPTIONS="suppressions=$PWD/.lsan-suppressions:print_suppressions=0" \
exec ./build/portaudio-player/portaudio_player "$@" \
    2> >(filter_audio_backend_probe_noise >&2)

