/*
  ==============================================================================

    Eclipse - Velocity Masking
    MIDI FX Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class EclipseAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    EclipseAudioProcessorEditor (EclipseAudioProcessor&);
    ~EclipseAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    EclipseAudioProcessor& audioProcessor;
    std::unique_ptr<juce::WebBrowserComponent> webView;
    
    void loadWebUI();
    void loadHTMLFile (const juce::File& htmlFile);
    void handleJavaScriptMessage (const juce::var& message);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EclipseAudioProcessorEditor)
};
