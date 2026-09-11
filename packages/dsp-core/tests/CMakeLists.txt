add_executable(
    library_smoke_test
    LibrarySmokeTest.cpp
)

target_link_libraries(
    library_smoke_test
    PRIVATE score_audio_studio::dsp_core
)

target_compile_features(
    library_smoke_test
    PRIVATE cxx_std_20
)

add_test(
    NAME library_smoke_test
    COMMAND library_smoke_test
)
