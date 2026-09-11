add_executable(
    dsp_core_smoke_test
    DspCoreSmokeTest.cpp
)

target_link_libraries(
    dsp_core_smoke_test
    PRIVATE score_audio_studio::dsp_core
)

target_compile_features(
    dsp_core_smoke_test
    PRIVATE cxx_std_20
)

add_test(
    NAME dsp_core_smoke_test
    COMMAND dsp_core_smoke_test
)
