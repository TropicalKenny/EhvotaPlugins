add_library(EhvotaTiming STATIC
    RefTimeMath.cpp
    RefTimeMath.h
)

target_include_directories(EhvotaTiming
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_compile_features(EhvotaTiming PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(EhvotaTiming PRIVATE /W4 /permissive-)
else()
    target_compile_options(EhvotaTiming PRIVATE -Wall -Wextra -Wpedantic)
endif()
