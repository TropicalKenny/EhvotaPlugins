juce_add_gui_app(EhvotaUIDemo
    PRODUCT_NAME "Ehvota UI Preview"
    COMPANY_NAME "Ehvota"
    BUNDLE_ID "com.ehvota.bundle.uipreview"
    VERSION "${PROJECT_VERSION}"
    MICROPHONE_PERMISSION_ENABLED FALSE
)

juce_generate_juce_header(EhvotaUIDemo)

target_sources(EhvotaUIDemo
    PRIVATE
        ApprovedRefHubPreview.cpp
        ApprovedRefHubPreview.h
        JourneyProgressComponent.cpp
        JourneyProgressComponent.h
        Main.cpp
        PreviewComponent.cpp
        PreviewComponent.h
)

target_link_libraries(EhvotaUIDemo
    PRIVATE
        EhvotaUI
        EhvotaViewModels
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(EhvotaUIDemo
    PRIVATE
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
)
