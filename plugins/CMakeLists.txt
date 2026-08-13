function(ehvota_add_plugin TARGET_NAME PRODUCT_NAME PLUGIN_CODE SOURCE_DIR)
    juce_add_plugin(${TARGET_NAME}
        COMPANY_NAME "Ehvota"
        BUNDLE_ID "com.ehvota.bundle.${TARGET_NAME}"
        PLUGIN_MANUFACTURER_CODE Ehvt
        PLUGIN_CODE ${PLUGIN_CODE}
        FORMATS VST3
        PRODUCT_NAME "${PRODUCT_NAME}"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        COPY_PLUGIN_AFTER_BUILD FALSE
    )

    target_sources(${TARGET_NAME}
        PRIVATE
            ${SOURCE_DIR}/PluginProcessor.cpp
            ${SOURCE_DIR}/PluginProcessor.h
            ${SOURCE_DIR}/PluginEditor.cpp
            ${SOURCE_DIR}/PluginEditor.h
    )

    target_include_directories(${TARGET_NAME}
        PRIVATE
            "${PROJECT_SOURCE_DIR}"
    )

    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
    target_compile_definitions(${TARGET_NAME}
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
    )

    target_link_libraries(${TARGET_NAME}
        PRIVATE
            juce::juce_audio_utils
            juce::juce_dsp
            EhvotaTiming
            EhvotaCommunication
            EhvotaSafety
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )
endfunction()

ehvota_add_plugin(EhvotaRefHub "Ehvota RefHub" RfHb "${CMAKE_CURRENT_SOURCE_DIR}/refhub")
ehvota_add_plugin(EhvotaRefTap "Ehvota RefTap" RfTp "${CMAKE_CURRENT_SOURCE_DIR}/reftap")
ehvota_add_plugin(EhvotaRefEQ "Ehvota RefEQ" RfEq "${CMAKE_CURRENT_SOURCE_DIR}/refeq")
ehvota_add_plugin(EhvotaRefComp "Ehvota RefComp" RfCp "${CMAKE_CURRENT_SOURCE_DIR}/refcomp")
ehvota_add_plugin(EhvotaRefTime "Ehvota RefTime" RfTm "${CMAKE_CURRENT_SOURCE_DIR}/reftime")
