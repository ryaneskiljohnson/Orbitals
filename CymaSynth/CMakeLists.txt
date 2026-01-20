cmake_minimum_required(VERSION 3.22)

project(CymaSynth VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(CymaSynth
    COMPANY_NAME "NNAudio"
    PRODUCT_NAME "CymaSynth"
    VERSION ${PROJECT_VERSION}
    PLUGIN_MANUFACTURER_CODE "NNau"
    PLUGIN_CODE "Cyma"
    FORMATS VST3 AU
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    COMPANY_WEBSITE "https://nnaud.io"
    COMPANY_EMAIL "info@nnaudio.com"
    BUNDLE_ID "com.nnaudio.cymasynth"
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(CymaSynth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/BufferedSynthesiser.cpp
    Source/BufferedSynthesiser.h
    Source/BufferedSynthVoice.cpp
    Source/BufferedSynthVoice.h
)

target_link_libraries(CymaSynth PRIVATE
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

target_compile_definitions(CymaSynth PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
