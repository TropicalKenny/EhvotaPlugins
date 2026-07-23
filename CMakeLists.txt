cmake_minimum_required(VERSION 3.22)

project(
    EhvotasBundle
    VERSION 0.1.0
    DESCRIPTION "Ehvota's Bundle Windows VST3 and utility suite"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(EHVOTA_BUILD_UI_PREVIEW "Build the Windows JUCE UI Preview" ${WIN32})
option(EHVOTA_BUILD_TESTS "Build deterministic non-audio tests" ON)

add_subdirectory(shared/timing)
add_subdirectory(shared/viewmodels)

if(EHVOTA_BUILD_UI_PREVIEW)
    if(NOT WIN32)
        message(FATAL_ERROR "Ehvota UI Preview is a Windows-only product target.")
    endif()

    set(EHVOTA_JUCE_SOURCE_DIR "" CACHE PATH "Path to an accepted, local JUCE source checkout")
    if(NOT EXISTS "${EHVOTA_JUCE_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "EHVOTA_JUCE_SOURCE_DIR must point to an accepted local JUCE checkout. "
            "Set JUCE_ROOT and use the Windows preset, or pass "
            "-DEHVOTA_JUCE_SOURCE_DIR=C:/path/to/JUCE. The project does not "
            "download JUCE or accept its licence automatically."
        )
    endif()

    add_subdirectory("${EHVOTA_JUCE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_deps/juce")
    add_subdirectory(shared/ui)
    add_subdirectory(apps/ui-preview)
endif()

if(EHVOTA_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests/unit)
endif()
