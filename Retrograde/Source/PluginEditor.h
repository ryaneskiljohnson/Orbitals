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
class RetrogradeAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    RetrogradeAudioProcessorEditor (RetrogradeAudioProcessor&);
    ~RetrogradeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    RetrogradeAudioProcessor& audioProcessor;
    std::unique_ptr<juce::WebBrowserComponent> webView;
    
    void loadWebUI();
    void handleJavaScriptMessage (const juce::var& message);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RetrogradeAudioProcessorEditor)
};
