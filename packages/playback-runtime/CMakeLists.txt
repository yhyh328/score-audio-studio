cmake_minimum_required(VERSION 3.20)

project(score_audio_studio_playback_runtime LANGUAGES CXX)

include(CTest)

#if(NOT TARGET score_audio_studio::dsp_core)
#    add_subdirectory(
#        "${CMAKE_CURRENT_LIST_DIR}/../dsp-core"
#        "${CMAKE_CURRENT_BINARY_DIR}/dsp-core"
#    )
#endif()

add_library(score_audio_studio_playback_runtime STATIC
    src/LibrarySmokeTest.cpp
    src/PlaybackRuntime.cpp
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

#target_link_libraries(score_audio_studio_playback_runtime
#    PUBLIC
#        score_audio_studio::dsp_core
#)

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
