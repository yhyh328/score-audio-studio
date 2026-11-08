cmake_minimum_required(VERSION 3.20)

project(wav_renderer LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../../packages/dsp-core"
    "${CMAKE_CURRENT_BINARY_DIR}/dsp-core"
)

add_executable(wav_renderer
    src/main.cpp
    src/WavRenderer.cpp
)

target_include_directories(wav_renderer
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(wav_renderer
    PUBLIC cxx_std_20
)

target_link_libraries(wav_renderer
    PRIVATE score_audio_studio::dsp_core
)

if(
    CMAKE_BUILD_TYPE STREQUAL "Debug"
    AND
    CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"
)
    target_compile_options(wav_renderer
        PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
            -fno-sanitize-recover=all
    )

    target_link_options(wav_renderer
        PRIVATE
            -fsanitize=address,undefined
    )
endif()
