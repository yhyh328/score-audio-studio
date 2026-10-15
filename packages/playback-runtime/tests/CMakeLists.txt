function(add_playback_runtime_test target source)
    add_executable(${target} ${source})

    target_link_libraries(
        ${target}
        PRIVATE score_audio_studio::playback_runtime
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target}
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wconversion
                -Wsign-conversion
                -Wshadow
        )
    endif()

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

add_playback_runtime_test(library_smoke_test LibrarySmokeTest.cpp)
