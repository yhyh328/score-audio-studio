function(add_dsp_test target source)
    add_executable(${target} ${source})

    target_link_libraries(
        ${target}
        PRIVATE score_audio_studio::dsp_core
    )

    add_test(
        NAME ${target}
        COMMAND ${target}
    )
endfunction()

add_dsp_test(library_smoke_test LibrarySmokeTest.cpp)
add_dsp_test(audio_engine_test AudioEngineTest.cpp)
add_dsp_test(oscillator_test OscillatorTest.cpp)
