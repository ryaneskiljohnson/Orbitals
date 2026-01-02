cmake_minimum_required(VERSION 3.22)

project(Tidal VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
add_subdirectory(JUCE)

# Create the plugin
juce_add_plugin(Tidal
    COMPANY_NAME "NNAudio"
    PRODUCT_NAME "Tidal"
    VERSION ${PROJECT_VERSION}
    PLUGIN_MANUFACTURER_CODE "NNau"
    PLUGIN_CODE "Tidl"
    FORMATS VST3 AU
    IS_MIDI_EFFECT TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    COMPANY_WEBSITE "https://nnaud.io"
    COMPANY_EMAIL "info@nnaudio.com"
    BUNDLE_ID "com.nnaudio.tidal"
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# Add source files
target_sources(Tidal PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

# Add UI resources as binary data
juce_add_binary_data(TidalResources SOURCES
    UI/index.html
    UI/styles.css
    UI/app.js
    ../_Shared/UI/orbitals-design-system.css
    ../_Shared/UI/orbitals-animations.js
    ../_Shared/UI/orbitals-particles.js
    ../_Shared/UI/orbitals-components.js
    ../_Shared/Assets/backgrounds/tidal-background.png
)

# Link resources to plugin
target_link_libraries(Tidal PRIVATE
    TidalResources
)

# Add JUCE modules
target_link_libraries(Tidal PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# Plugin definitions
target_compile_definitions(Tidal PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
