/*
  ==============================================================================

    This file contains the basic framework code for a JUCE processor editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Main component using JUCE 8 WebView for CSS-based UI
*/
class MainComponent  : public juce::Component,
                        public juce::AudioIODeviceCallback
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    // WebView for CSS-based UI (WebBrowserComponent is in juce_gui_extra)
    std::unique_ptr<juce::WebBrowserComponent> webView;
    
    // Audio components
    juce::AudioDeviceManager audioDeviceManager;
    bool isPlaying = false;
    double currentFrequency = 440.0;
    double currentPhase = 0.0;
    double phaseDelta = 0.0;
    float currentVolume = 0.5f;
    double sampleRate = 44100.0;
    juce::String currentWaveform = "sine"; // sine, square, sawtooth, triangle
    
    // Audio callback methods
    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                            int numInputChannels,
                                            float* const* outputChannelData,
                                            int numOutputChannels,
                                            int numSamples,
                                            const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart (juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError (const juce::String& errorMessage) override;
    
    // Helper to load HTML content
    void loadWebUI();
    
    // Loading screen
    
    void loadMainUI();
    
    // Helper functions
    void loadHTMLFile (const juce::File& htmlFile);
    void showFallbackUI();
    
    // Handle messages from JavaScript
    void handleJavaScriptMessage (const juce::var& message);
    
    // Send message to JavaScript
    void sendMessageToWebView (const juce::var& message);
    
    // Flag to switch between native and web UI
    bool useWebView = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
