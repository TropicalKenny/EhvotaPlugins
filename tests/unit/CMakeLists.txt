add_executable(EhvotaUnitTests
    PreviewScenarioTests.cpp
)

target_link_libraries(EhvotaUnitTests
    PRIVATE
        EhvotaTiming
        EhvotaViewModels
)

target_compile_features(EhvotaUnitTests PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(EhvotaUnitTests PRIVATE /W4 /permissive-)
else()
    target_compile_options(EhvotaUnitTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME EhvotaUnitTests COMMAND EhvotaUnitTests)

add_executable(EhvotaAbletonBridgeStateTests
    AbletonBridgeStateTests.cpp
)

target_link_libraries(EhvotaAbletonBridgeStateTests
    PRIVATE
        EhvotaViewModels
)

target_compile_features(EhvotaAbletonBridgeStateTests PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(EhvotaAbletonBridgeStateTests PRIVATE /W4 /permissive-)
else()
    target_compile_options(EhvotaAbletonBridgeStateTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME EhvotaAbletonBridgeStateTests COMMAND EhvotaAbletonBridgeStateTests)

add_executable(EhvotaLevelMatchStateTests
    LevelMatchStateTests.cpp
)

target_link_libraries(EhvotaLevelMatchStateTests
    PRIVATE
        EhvotaViewModels
)

target_compile_features(EhvotaLevelMatchStateTests PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(EhvotaLevelMatchStateTests PRIVATE /W4 /permissive-)
else()
    target_compile_options(EhvotaLevelMatchStateTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME EhvotaLevelMatchStateTests COMMAND EhvotaLevelMatchStateTests)

add_executable(EhvotaRefHubNavigationTests
    RefHubNavigationTests.cpp
)

target_link_libraries(EhvotaRefHubNavigationTests
    PRIVATE
        EhvotaViewModels
)

target_compile_features(EhvotaRefHubNavigationTests PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(EhvotaRefHubNavigationTests PRIVATE /W4 /permissive-)
else()
    target_compile_options(EhvotaRefHubNavigationTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME EhvotaRefHubNavigationTests COMMAND EhvotaRefHubNavigationTests)

add_executable(EhvotaBridgeProtocolTests
    BridgeProtocolTests.cpp
)

target_link_libraries(EhvotaBridgeProtocolTests
    PRIVATE
        EhvotaCommunication
)

target_compile_features(EhvotaBridgeProtocolTests PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(EhvotaBridgeProtocolTests PRIVATE /W4 /permissive-)
else()
    target_compile_options(EhvotaBridgeProtocolTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME EhvotaBridgeProtocolTests COMMAND EhvotaBridgeProtocolTests)
