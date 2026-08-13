add_library(EhvotaSafety STATIC
    ProposalPolicy.cpp
    ProposalPolicy.h
    AuditionPolicy.cpp
    AuditionPolicy.h
)

target_include_directories(EhvotaSafety
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_compile_features(EhvotaSafety PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(EhvotaSafety PRIVATE /W4 /permissive-)
else()
    target_compile_options(EhvotaSafety PRIVATE -Wall -Wextra -Wpedantic)
endif()
