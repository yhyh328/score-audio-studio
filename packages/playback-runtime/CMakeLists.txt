cmake_minimum_required(VERSION 3.20)

project(score_audio_studio_playback_runtime LANGUAGES CXX)

include(CTest)

add_library(score_audio_studio_playback_runtime STATIC
    src/LibrarySmokeTest.cpp
)

add_library(score_audio_studio::playback_runtime
    ALIAS score_audio_studio_playback_runtime
)

target_compile_features(score_audio_studio_playback_runtime
    PUBLIC cxx_std_20
)

target_include_directories(score_audio_studio_playback_runtime
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

option(
    BUILD_PLAYBACK_RUNTIME_NODE_ADDON
    "Build the Playback Runtime Node addon"
    OFF
)

if(BUILD_PLAYBACK_RUNTIME_NODE_ADDON)
    if(NOT NODE_INCLUDE_DIR)
        message(
            FATAL_ERROR
            "NODE_INCLUDE_DIR is required when building the Node addon"
        )
    endif()

    add_library(
        playback_runtime_shared_buffer_addon
        MODULE
        share-buffer-addon/PlaybackRuntimeAddon.cpp
    )

    target_compile_features(
        playback_runtime_shared_buffer_addon
        PRIVATE cxx_std_20
    )

    target_include_directories(
        playback_runtime_shared_buffer_addon
        SYSTEM PRIVATE
            "${NODE_INCLUDE_DIR}"
    )

    target_include_directories(
        playback_runtime_shared_buffer_addon
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/../dsp-core/include"
    )

    target_compile_definitions(
        playback_runtime_shared_buffer_addon
        PRIVATE
            NODE_GYP_MODULE_NAME=playback_runtime_shared_buffer_addon
    )

    set_target_properties(
        playback_runtime_shared_buffer_addon
        PROPERTIES
            PREFIX ""
            SUFFIX ".node"
    )

    target_compile_options(
        playback_runtime_shared_buffer_addon
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(score_audio_studio_playback_runtime
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wshadow
    )
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(score_audio_studio_playback_runtime
            PRIVATE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
                -fno-sanitize-recover=all
        )
    endif()
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
