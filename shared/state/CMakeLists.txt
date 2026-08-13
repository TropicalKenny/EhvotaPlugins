add_library(EhvotaState STATIC
    ReferenceModel.cpp
    ReferenceModel.h
    ReferenceFolderRules.cpp
    ReferenceFolderRules.h
    CapturedComparison.cpp
    CapturedComparison.h
)

target_include_directories(EhvotaState
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(EhvotaState PUBLIC EhvotaAnalysis)
target_compile_features(EhvotaState PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(EhvotaState PRIVATE /W4 /permissive-)
else()
    target_compile_options(EhvotaState PRIVATE -Wall -Wextra -Wpedantic)
endif()
