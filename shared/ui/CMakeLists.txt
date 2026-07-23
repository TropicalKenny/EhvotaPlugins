add_library(EhvotaUI STATIC
    Components.cpp
    Components.h
    EhvotaLookAndFeel.cpp
    EhvotaLookAndFeel.h
    Theme.cpp
    Theme.h
)

target_include_directories(EhvotaUI
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(EhvotaUI
    PUBLIC
        EhvotaViewModels
        juce::juce_gui_basics
)

target_compile_features(EhvotaUI PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(EhvotaUI PRIVATE /W4 /permissive-)
endif()
