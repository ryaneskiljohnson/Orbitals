/*
  ==============================================================================

    Lagrange - Timing Stabilizer
    MIDI FX Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class LagrangeAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    LagrangeAudioProcessorEditor (LagrangeAudioProcessor&);
    ~LagrangeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    LagrangeAudioProcessor& audioProcessor;
    std::unique_ptr<juce::WebBrowserComponent> webView;
    
    void loadWebUI();
    void loadHTMLFile (const juce::File& htmlFile);
    void handleJavaScriptMessage (const juce::var& message);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LagrangeAudioProcessorEditor)
};
