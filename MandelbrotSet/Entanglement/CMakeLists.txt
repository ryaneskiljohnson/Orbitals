cmake_minimum_required(VERSION 3.22)

project(Entanglement VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(Entanglement
    COMPANY_NAME "NNAudio"
    PRODUCT_NAME "Entanglement"
    VERSION ${PROJECT_VERSION}
    PLUGIN_MANUFACTURER_CODE "NNau"
    PLUGIN_CODE "Enta"
    FORMATS VST3 AU Standalone
    IS_MIDI_EFFECT FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COMPANY_WEBSITE "https://nnaud.io"
    COMPANY_EMAIL "info@nnaudio.com"
    BUNDLE_ID "com.nnaudio.entanglement"
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(Entanglement PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

juce_add_binary_data(EntanglementResources SOURCES
    UI/index.html
    UI/styles.css
    UI/app.js
    ../_Shared/UI/mandelbrot-design-system.css
    ../_Shared/UI/mandelbrot-animations.js
    ../_Shared/UI/mandelbrot-particles.js
    ../_Shared/UI/mandelbrot-components.js
    ../_Shared/Assets/backgrounds/entanglement.png
)

target_link_libraries(Entanglement PRIVATE
    EntanglementResources
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

target_compile_definitions(Entanglement PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
