add_library(EhvotaCommunication STATIC
    BridgeProtocol.cpp
    BridgeProtocol.h
    RefLinkRegistry.cpp
    RefLinkRegistry.h
)

target_include_directories(EhvotaCommunication
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_compile_features(EhvotaCommunication PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(EhvotaCommunication PRIVATE /W4 /permissive-)
else()
    target_compile_options(EhvotaCommunication PRIVATE -Wall -Wextra -Wpedantic)
endif()
