cmake_minimum_required(VERSION 3.22)

project(
    EhvotasBundle
    VERSION 0.1.0
    DESCRIPTION "Ehvota's Bundle Windows VST3 and utility suite"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
endif()

option(EHVOTA_BUILD_UI_PREVIEW "Build the Windows JUCE UI Preview" ${WIN32})
option(EHVOTA_BUILD_PLUGINS "Build the Windows VST3 product targets" ${WIN32})
option(EHVOTA_BUILD_TESTS "Build deterministic non-audio tests" ON)

add_subdirectory(shared/timing)
add_subdirectory(shared/viewmodels)
add_subdirectory(shared/communication)
add_subdirectory(shared/audio)
add_subdirectory(shared/analysis)
add_subdirectory(shared/safety)
add_subdirectory(shared/state)

if(EHVOTA_BUILD_UI_PREVIEW OR EHVOTA_BUILD_PLUGINS)
    if(NOT WIN32)
        message(FATAL_ERROR "Ehvota JUCE product targets are Windows-only.")
    endif()

    set(_ehvota_default_juce_source "${PROJECT_SOURCE_DIR}/third_party/JUCE")
    if(DEFINED ENV{JUCE_ROOT} AND NOT "$ENV{JUCE_ROOT}" STREQUAL "")
        set(_ehvota_default_juce_source "$ENV{JUCE_ROOT}")
    endif()
    set(EHVOTA_JUCE_SOURCE_DIR
        "${_ehvota_default_juce_source}"
        CACHE PATH
        "Path to an accepted, local JUCE source checkout"
    )
    if(NOT EXISTS "${EHVOTA_JUCE_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "EHVOTA_JUCE_SOURCE_DIR must point to an accepted local JUCE checkout. "
            "Initialize the pinned submodule, set JUCE_ROOT, or pass "
            "-DEHVOTA_JUCE_SOURCE_DIR=C:/path/to/JUCE. CMake does not "
            "download JUCE or accept its licence automatically."
        )
    endif()

    add_subdirectory("${EHVOTA_JUCE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_deps/juce")
endif()

if(EHVOTA_BUILD_UI_PREVIEW)
    add_subdirectory(shared/ui)
    add_subdirectory(apps/ui-preview)
endif()

if(EHVOTA_BUILD_PLUGINS)
    add_subdirectory(plugins)
endif()

if(EHVOTA_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests/unit)
endif()
