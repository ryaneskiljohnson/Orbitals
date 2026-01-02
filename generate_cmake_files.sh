#!/bin/bash

# Generate CMakeLists.txt for each plugin

create_cmake() {
    local plugin=$1
    local plugin_lower=$(echo "$plugin" | tr '[:upper:]' '[:lower:]')
    local plugin_code=$2
    
    cat > "$plugin/CMakeLists.txt" << EOF
cmake_minimum_required(VERSION 3.22)

project($plugin VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin($plugin
    COMPANY_NAME "NNAudio"
    PRODUCT_NAME "$plugin"
    VERSION \${PROJECT_VERSION}
    PLUGIN_MANUFACTURER_CODE "NNau"
    PLUGIN_CODE "$plugin_code"
    FORMATS VST3 AU
    IS_MIDI_EFFECT TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    COMPANY_WEBSITE "https://nnaud.io"
    COMPANY_EMAIL "info@nnaudio.com"
    BUNDLE_ID "com.nnaudio.$plugin_lower"
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources($plugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_link_libraries($plugin PRIVATE
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

target_compile_definitions($plugin PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
EOF
    
    echo "✓ Created CMakeLists.txt for $plugin"
}

# Create CMakeLists for each plugin
create_cmake "Apogee" "Apog"
create_cmake "Lagrange" "Lagr"
create_cmake "Eclipse" "Ecli"
create_cmake "Ion" "Ionx"
create_cmake "Perihelion" "Peri"
create_cmake "Retrograde" "Retr"
create_cmake "Kepler" "Kepl"

echo "Done generating CMakeLists.txt files"
