cmake_minimum_required(VERSION 3.20)

project(score_audio_studio_dsp_core LANGUAGES CXX)

include(CTest)

add_library(score_audio_studio_dsp_core STATIC
    src/LibrarySmokeTest.cpp
    src/AudioEngine.cpp
)

add_library(score_audio_studio::dsp_core
    ALIAS score_audio_studio_dsp_core
)

target_compile_features(score_audio_studio_dsp_core
    PUBLIC cxx_std_20
)

target_include_directories(score_audio_studio_dsp_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(score_audio_studio_dsp_core
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
