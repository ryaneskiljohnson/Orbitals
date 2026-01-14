cmake_minimum_required(VERSION 3.22)

project(ChipTune VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
add_subdirectory(JUCE)

# Create the plugin
juce_add_plugin(ChipTune
    COMPANY_NAME "NNAudio"
    PRODUCT_NAME "ChipTune"
    VERSION ${PROJECT_VERSION}
    PLUGIN_MANUFACTURER_CODE "NNau"
    PLUGIN_CODE "Chip"
    FORMATS VST3 AU Standalone
    IS_MIDI_EFFECT FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COMPANY_WEBSITE "https://nnaud.io"
    COMPANY_EMAIL "info@nnaudio.com"
    BUNDLE_ID "com.nnaudio.chiptune"
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# Add source files
target_sources(ChipTune PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

# Add UI resources as binary data
juce_add_binary_data(ChipTuneResources SOURCES
    UI/index.html
    UI/styles.css
    UI/app.js
    ../_Shared/UI/mandelbrot-design-system.css
    ../_Shared/UI/mandelbrot-animations.js
    ../_Shared/UI/mandelbrot-particles.js
    ../_Shared/UI/mandelbrot-components.js
    ../_Shared/Assets/backgrounds/chiptune.png
)

# Link resources to plugin
target_link_libraries(ChipTune PRIVATE
    ChipTuneResources
)

# Add JUCE modules
target_link_libraries(ChipTune PRIVATE
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
target_compile_definitions(ChipTune PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
