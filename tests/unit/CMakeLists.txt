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

add_executable(EhvotaAbletonBridgeStateTests AbletonBridgeStateTests.cpp)
target_link_libraries(EhvotaAbletonBridgeStateTests PRIVATE EhvotaViewModels)
target_compile_features(EhvotaAbletonBridgeStateTests PRIVATE cxx_std_20)
add_test(NAME EhvotaAbletonBridgeStateTests COMMAND EhvotaAbletonBridgeStateTests)

add_executable(EhvotaLevelMatchStateTests LevelMatchStateTests.cpp)
target_link_libraries(EhvotaLevelMatchStateTests PRIVATE EhvotaViewModels)
target_compile_features(EhvotaLevelMatchStateTests PRIVATE cxx_std_20)
add_test(NAME EhvotaLevelMatchStateTests COMMAND EhvotaLevelMatchStateTests)

add_executable(EhvotaRefHubNavigationTests RefHubNavigationTests.cpp)
target_link_libraries(EhvotaRefHubNavigationTests PRIVATE EhvotaViewModels)
target_compile_features(EhvotaRefHubNavigationTests PRIVATE cxx_std_20)
add_test(NAME EhvotaRefHubNavigationTests COMMAND EhvotaRefHubNavigationTests)

add_executable(EhvotaBridgeProtocolTests BridgeProtocolTests.cpp)
target_link_libraries(EhvotaBridgeProtocolTests PRIVATE EhvotaCommunication)
target_compile_features(EhvotaBridgeProtocolTests PRIVATE cxx_std_20)
add_test(NAME EhvotaBridgeProtocolTests COMMAND EhvotaBridgeProtocolTests)

add_executable(EhvotaProposalPolicyTests ProposalPolicyTests.cpp)
target_link_libraries(EhvotaProposalPolicyTests PRIVATE EhvotaSafety)
target_compile_features(EhvotaProposalPolicyTests PRIVATE cxx_std_20)
add_test(NAME EhvotaProposalPolicyTests COMMAND EhvotaProposalPolicyTests)

add_executable(EhvotaReferenceModelTests ReferenceModelTests.cpp)
target_link_libraries(EhvotaReferenceModelTests PRIVATE EhvotaState)
target_compile_features(EhvotaReferenceModelTests PRIVATE cxx_std_20)
add_test(NAME EhvotaReferenceModelTests COMMAND EhvotaReferenceModelTests)

add_executable(EhvotaAnalysisTests AnalysisTests.cpp)
target_link_libraries(EhvotaAnalysisTests PRIVATE EhvotaAnalysis)
target_compile_features(EhvotaAnalysisTests PRIVATE cxx_std_20)
add_test(NAME EhvotaAnalysisTests COMMAND EhvotaAnalysisTests)

add_executable(EhvotaLoudness1770Tests Loudness1770Tests.cpp)
target_link_libraries(EhvotaLoudness1770Tests PRIVATE EhvotaAnalysis)
target_compile_features(EhvotaLoudness1770Tests PRIVATE cxx_std_20)
add_test(NAME EhvotaLoudness1770Tests COMMAND EhvotaLoudness1770Tests)

add_executable(EhvotaCapturedComparisonTests CapturedComparisonTests.cpp)
target_link_libraries(EhvotaCapturedComparisonTests PRIVATE EhvotaState)
target_compile_features(EhvotaCapturedComparisonTests PRIVATE cxx_std_20)
add_test(NAME EhvotaCapturedComparisonTests COMMAND EhvotaCapturedComparisonTests)

add_executable(EhvotaAuditionPolicyTests AuditionPolicyTests.cpp)
target_link_libraries(EhvotaAuditionPolicyTests PRIVATE EhvotaSafety)
target_compile_features(EhvotaAuditionPolicyTests PRIVATE cxx_std_20)
add_test(NAME EhvotaAuditionPolicyTests COMMAND EhvotaAuditionPolicyTests)

add_executable(EhvotaRefLinkRegistryTests RefLinkRegistryTests.cpp)
target_link_libraries(EhvotaRefLinkRegistryTests PRIVATE EhvotaCommunication)
target_compile_features(EhvotaRefLinkRegistryTests PRIVATE cxx_std_20)
add_test(NAME EhvotaRefLinkRegistryTests COMMAND EhvotaRefLinkRegistryTests)

add_executable(EhvotaReferenceFolderRulesTests ReferenceFolderRulesTests.cpp)
target_link_libraries(EhvotaReferenceFolderRulesTests PRIVATE EhvotaState)
target_compile_features(EhvotaReferenceFolderRulesTests PRIVATE cxx_std_20)
add_test(NAME EhvotaReferenceFolderRulesTests COMMAND EhvotaReferenceFolderRulesTests)

foreach(target
    EhvotaAbletonBridgeStateTests
    EhvotaLevelMatchStateTests
    EhvotaRefHubNavigationTests
    EhvotaBridgeProtocolTests
    EhvotaProposalPolicyTests
    EhvotaReferenceModelTests
    EhvotaAnalysisTests
    EhvotaLoudness1770Tests
    EhvotaCapturedComparisonTests
    EhvotaAuditionPolicyTests
    EhvotaRefLinkRegistryTests
    EhvotaReferenceFolderRulesTests)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
