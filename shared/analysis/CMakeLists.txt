add_library(EhvotaAnalysis STATIC
    BasicMetrics.cpp
    BasicMetrics.h
    SpectrumAnalysis.cpp
    SpectrumAnalysis.h
    Loudness1770.cpp
    Loudness1770.h
    StreamingLoudness1770.cpp
    StreamingLoudness1770.h
    LevelMatch.cpp
    LevelMatch.h
    CapturedSectionAnalysis.cpp
    CapturedSectionAnalysis.h
    ComparisonAnalysis.cpp
    ComparisonAnalysis.h
    CompactAnalysis.cpp
    CompactAnalysis.h
)

target_include_directories(EhvotaAnalysis
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_compile_features(EhvotaAnalysis PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(EhvotaAnalysis PRIVATE /W4 /permissive-)
else()
    target_compile_options(EhvotaAnalysis PRIVATE -Wall -Wextra -Wpedantic)
endif()
