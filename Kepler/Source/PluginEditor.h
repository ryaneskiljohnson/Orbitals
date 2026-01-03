/*
  ==============================================================================

    Kepler - MIDI Orbit Generator
    MIDI FX Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../_Shared/Authentication/HubAuthComponent.h"

//==============================================================================
class KeplerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    KeplerAudioProcessorEditor (KeplerAudioProcessor&);
    ~KeplerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    KeplerAudioProcessor& audioProcessor;
    std::unique_ptr<juce::WebBrowserComponent> webView;
    
    void loadWebUI();
    void loadHTMLFile (const juce::File& htmlFile);
    void handleJavaScriptMessage (const juce::var& message);

       // Authentication component
    NNAudio::Authentication::HubAuthComponent m_auth_component;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeplerAudioProcessorEditor)
};
