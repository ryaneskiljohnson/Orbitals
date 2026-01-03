/*
  ==============================================================================

    Retrograde - MIDI Motion Reversal
    MIDI FX Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class RetrogradeAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    RetrogradeAudioProcessorEditor (RetrogradeAudioProcessor&);
    ~RetrogradeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    RetrogradeAudioProcessor& audioProcessor;
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RetrogradeAudioProcessorEditor)
};
