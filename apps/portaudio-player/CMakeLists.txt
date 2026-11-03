cmake_minimum_required(VERSION 3.20)

project(portaudio_player LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../../packages/dsp-core"
    "${CMAKE_CURRENT_BINARY_DIR}/dsp-core"
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(PORTAUDIO
    REQUIRED
    IMPORTED_TARGET
    portaudio-2.0
)

add_executable(portaudio_player
    src/main.cpp
    src/PortAudioHost.cpp
)

target_include_directories(portaudio_player
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(portaudio_player
    PUBLIC cxx_std_20
)

target_link_libraries(portaudio_player
    PRIVATE score_audio_studio::dsp_core
            PkgConfig::PORTAUDIO
)

if(
    CMAKE_BUILD_TYPE STREQUAL "Debug"
    AND
    CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"
)
    target_compile_options(portaudio_player
        PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
            -fno-sanitize-recover=all
    )

    target_link_options(portaudio_player
        PRIVATE
            -fsanitize=address,undefined
    )
endif()
