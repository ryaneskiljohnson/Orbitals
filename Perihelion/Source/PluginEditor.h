/*
  ==============================================================================

    Perihelion - Velocity Sculptor
    MIDI FX Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class PerihelionAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PerihelionAudioProcessorEditor (PerihelionAudioProcessor&);
    ~PerihelionAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PerihelionAudioProcessor& audioProcessor;
    std::unique_ptr<juce::WebBrowserComponent> webView;
    
    void loadWebUI();
    void handleJavaScriptMessage (const juce::var& message);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerihelionAudioProcessorEditor)
};
