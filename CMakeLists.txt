cmake_minimum_required(VERSION 3.20)

project(music_elf VERSION 0.1.0 LANGUAGES CXX)

add_library(music_elf_core STATIC
    src/accompaniment_generator.cpp
    src/audio_io.cpp
    src/audio_renderer.cpp
    src/c_api.cpp
    src/core_pipeline.cpp
    src/dynamics_analyzer.cpp
    src/harmony_analyzer.cpp
    src/lyric_aligner.cpp
    src/midi_catalog.cpp
    src/midi_writer.cpp
    src/model_interfaces.cpp
    src/musicxml_writer.cpp
    src/note_segmenter.cpp
    src/pitch_detector.cpp
    src/rhythm_analyzer.cpp
)

target_include_directories(music_elf_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(music_elf_core PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(music_elf_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(music_elf_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(music_elf_cli
    tools/music_elf_cli.cpp
)
target_link_libraries(music_elf_cli PRIVATE music_elf_core)
target_compile_features(music_elf_cli PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(music_elf_cli PRIVATE /W4 /permissive-)
else()
    target_compile_options(music_elf_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)

function(add_music_elf_test target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE music_elf_core)
    target_compile_features(${target} PRIVATE cxx_std_17)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME ${target} COMMAND ${target})
endfunction()

set(MUSIC_ELF_ACAPELLA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/acapella/fixtures")
set(MUSIC_ELF_ACAPELLA_OUT "${CMAKE_CURRENT_BINARY_DIR}/acapella_outputs")
file(MAKE_DIRECTORY "${MUSIC_ELF_ACAPELLA_OUT}")

if(BUILD_TESTING)
    add_music_elf_test(pitch_detector_tests tests/pitch_detector_tests.cpp)
    add_music_elf_test(note_segmenter_tests tests/note_segmenter_tests.cpp)
    add_music_elf_test(rhythm_dynamics_tests tests/rhythm_dynamics_tests.cpp)
    add_music_elf_test(harmony_tests tests/harmony_tests.cpp)
    add_music_elf_test(arrangement_midi_tests tests/arrangement_midi_tests.cpp)
    add_music_elf_test(audio_renderer_tests tests/audio_renderer_tests.cpp)
    add_music_elf_test(lyrics_musicxml_tests tests/lyrics_musicxml_tests.cpp)
    add_music_elf_test(audio_io_tests tests/audio_io_tests.cpp)
    add_music_elf_test(pipeline_integration_tests tests/pipeline_integration_tests.cpp)
    add_music_elf_test(c_api_tests tests/c_api_tests.cpp)
    add_music_elf_test(model_interfaces_tests tests/model_interfaces_tests.cpp)
    add_music_elf_test(midi_catalog_tests tests/midi_catalog_tests.cpp)
    add_music_elf_test(export_snapshot_tests tests/export_snapshot_tests.cpp)

    add_executable(cli_end_to_end_tests tests/cli_end_to_end_tests.cpp)
    target_link_libraries(cli_end_to_end_tests PRIVATE music_elf_core)
    target_compile_features(cli_end_to_end_tests PRIVATE cxx_std_17)
    if(MSVC)
        target_compile_options(cli_end_to_end_tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(cli_end_to_end_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME cli_end_to_end_tests COMMAND cli_end_to_end_tests $<TARGET_FILE:music_elf_cli>)

    add_test(NAME music_elf_cli_help COMMAND music_elf_cli --help)

    add_music_elf_test(acapella_real_audio_tests tests/acapella_real_audio_tests.cpp)
    target_compile_definitions(acapella_real_audio_tests PRIVATE
        MUSIC_ELF_ACAPELLA_DIR="${MUSIC_ELF_ACAPELLA_DIR}"
        MUSIC_ELF_ACAPELLA_OUT="${MUSIC_ELF_ACAPELLA_OUT}")

    add_executable(acapella_cli_tests tests/acapella_cli_tests.cpp)
    target_link_libraries(acapella_cli_tests PRIVATE music_elf_core)
    target_compile_features(acapella_cli_tests PRIVATE cxx_std_17)
    if(MSVC)
        target_compile_options(acapella_cli_tests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(acapella_cli_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    target_compile_definitions(acapella_cli_tests PRIVATE
        MUSIC_ELF_ACAPELLA_DIR="${MUSIC_ELF_ACAPELLA_DIR}"
        MUSIC_ELF_ACAPELLA_OUT="${MUSIC_ELF_ACAPELLA_OUT}")
    add_test(NAME acapella_cli_tests
             COMMAND acapella_cli_tests $<TARGET_FILE:music_elf_cli>)
endif()
