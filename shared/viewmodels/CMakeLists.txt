add_library(EhvotaViewModels STATIC
    JourneyReplay.cpp
    JourneyReplay.h
    LayoutPolicy.cpp
    LayoutPolicy.h
    PreviewScenario.cpp
    PreviewScenario.h
    ShellPresentation.cpp
    ShellPresentation.h
)

target_include_directories(EhvotaViewModels
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_compile_features(EhvotaViewModels PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(EhvotaViewModels PRIVATE /W4 /permissive-)
else()
    target_compile_options(EhvotaViewModels PRIVATE -Wall -Wextra -Wpedantic)
endif()
