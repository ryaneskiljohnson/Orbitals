/*
  ==============================================================================

    Singularity - Black Hole Compressor
    Audio FX Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
    Singularity Plugin Editor - WebView-based UI
*/
class SingularityAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    SingularityAudioProcessorEditor (SingularityAudioProcessor&);
    ~SingularityAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void sendMeteringData();

private:
    SingularityAudioProcessor& audioProcessor;
    
    // WebView for CSS-based UI
    std::unique_ptr<juce::WebBrowserComponent> webView;
    
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SingularityAudioProcessorEditor)
};
