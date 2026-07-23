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
