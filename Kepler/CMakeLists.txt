cmake_minimum_required(VERSION 3.22)

project(Synth VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
add_subdirectory(JUCE)

# Create the executable
juce_add_gui_app(Synth
    COMPANY_NAME "YourCompany"
    PRODUCT_NAME "Synth"
    VERSION ${PROJECT_VERSION}
    DOCUMENT_EXTENSIONS ""
    COMPANY_WEBSITE "https://yoursite.com"
    COMPANY_EMAIL "your@email.com"
    COMPANY_COPYRIGHT "Copyright (c) 2024"
    BUNDLE_ID "com.yourcompany.synth"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.synth"
    MACOSX_BUNDLE_BUNDLE_NAME "Synth"
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
    MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_MIN_SYS_VERSION "10.15"
)

# Add source files
target_sources(Synth PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/MainComponent.h
)

# Add JUCE modules
target_link_libraries(Synth PRIVATE
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

# Copy resources (if you have a Resources folder)
# target_copy_resources(Synth)
