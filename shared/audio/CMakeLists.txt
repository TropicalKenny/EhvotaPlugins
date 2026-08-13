add_library(EhvotaAudio STATIC
    CaptureBuffer.cpp
    CaptureBuffer.h
    Biquad.cpp
    Biquad.h
    AnalogSaturation.cpp
    AnalogSaturation.h
)

target_include_directories(EhvotaAudio
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_compile_features(EhvotaAudio PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(EhvotaAudio PRIVATE /W4 /permissive-)
else()
    target_compile_options(EhvotaAudio PRIVATE -Wall -Wextra -Wpedantic)
endif()
