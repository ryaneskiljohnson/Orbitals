/*
  ==============================================================================

    Tidal - Rhythmic Push & Pull
    MIDI FX Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// Helper class to handle WebBrowserComponent callbacks (like NNAudioAccess)
class WebBrowserWithCallbacks : public juce::WebBrowserComponent
{
public:
    WebBrowserWithCallbacks(const juce::WebBrowserComponent::Options& options)
        : juce::WebBrowserComponent(options) {}
    
    std::function<void(const juce::String&)> onPageFinishedLoading;
    
    void pageFinishedLoading(const juce::String& url) override
    {
        juce::WebBrowserComponent::pageFinishedLoading(url);
        if (onPageFinishedLoading) {
            onPageFinishedLoading(url);
        }
    }
};

//==============================================================================
/**
    Tidal Plugin Editor - WebView-based UI
*/
class TidalAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    TidalAudioProcessorEditor (TidalAudioProcessor&);
    ~TidalAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    TidalAudioProcessor& audioProcessor;
    
    // WebView for CSS-based UI
    std::unique_ptr<WebBrowserWithCallbacks> webView;
    
    // Authentication state
    bool isAuthorized = false;
    
    // Helper methods
    void loadWebUI();
    void loadAuthScreen();
    void loadHTMLFile (const juce::File& htmlFile);
    void handleJavaScriptMessage (const juce::var& message);
    bool checkAuthorization();
    static juce::File getAuthFile();
    static juce::String loadAndDecryptLicenseFile();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TidalAudioProcessorEditor)
};
