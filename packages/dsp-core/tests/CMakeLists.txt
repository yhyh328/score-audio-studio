function(add_dsp_test target source)
    add_executable(${target} ${source})

    target_link_libraries(
        ${target}
        PRIVATE score_audio_studio::dsp_core
    )
    if(
        CMAKE_BUILD_TYPE STREQUAL "Debug"
        AND
        CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"
    )
    target_compile_options(${target}
        PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
            -fno-sanitize-recover=all
    )
    target_link_options(${target}
        PRIVATE
            -fsanitize=address,undefined
    )
    endif()

    add_test(
        NAME ${target}
        COMMAND ${target}
    )
endfunction()

add_dsp_test(library_smoke_test LibrarySmokeTest.cpp)
add_dsp_test(audio_engine_test AudioEngineTest.cpp)
add_dsp_test(oscillator_test OscillatorTest.cpp)
add_dsp_test(adsr_envelope_test AdsrEnvelopeTest.cpp)
