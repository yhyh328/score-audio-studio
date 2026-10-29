cmake_minimum_required(VERSION 3.20)

project(score_audio_studio_dsp_core LANGUAGES CXX)

include(CTest)

add_library(score_audio_studio_dsp_core STATIC
    src/LibrarySmokeTest.cpp
    #src/AudioEngine.cpp
    src/Oscillator.cpp
    src/AdsrEnvelope.cpp
    src/Voice.cpp
    src/VoiceManager.cpp
    src/Instruments.cpp
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

target_compile_definitions(score_audio_studio_dsp_core
    PRIVATE PROJECT_CONFIG_DIR="${CMAKE_CURRENT_SOURCE_DIR}/config"
)

find_package(nlohmann_json REQUIRED)

target_link_libraries(score_audio_studio_dsp_core
    PRIVATE nlohmann_json::nlohmann_json
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(score_audio_studio_dsp_core
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wshadow
    )
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(score_audio_studio_dsp_core
            PRIVATE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
                -fno-sanitize-recover=all
        )
    endif()
endif()

if(
    BUILD_TESTING
    AND
    CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR
)
    add_subdirectory(tests)
endif()
