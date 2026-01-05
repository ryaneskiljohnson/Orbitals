/*
  ==============================================================================

    Hadron - Particle Collision Saturation
    Audio FX Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class HadronAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    HadronAudioProcessorEditor (HadronAudioProcessor&);
    ~HadronAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void sendMeteringData();

private:
    HadronAudioProcessor& audioProcessor;
    std::unique_ptr<juce::WebBrowserComponent> webView;
    
    // Authentication state
    bool isAuthorized = false;
    
    void loadWebUI();
    void loadAuthScreen();
    void loadHTMLFile (const juce::File& htmlFile);
    void handleJavaScriptMessage (const juce::var& message);
    bool checkAuthorization();
    static juce::File getAuthFile();
    static juce::String loadAndDecryptLicenseFile();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HadronAudioProcessorEditor)
};
