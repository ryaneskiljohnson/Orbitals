/*
  ==============================================================================

    Tidal - Rhythmic Push & Pull
    MIDI FX Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../_Shared/Authentication/HubAuthComponent.h"

//==============================================================================
/**
    Tidal Plugin Editor - WebView-based UI
*/
class TidalAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TidalAudioProcessorEditor (TidalAudioProcessor&);
    ~TidalAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    TidalAudioProcessor& audioProcessor;
    
    // WebView for CSS-based UI
    std::unique_ptr<juce::WebBrowserComponent> webView;
    
    // Authentication component
    NNAudio::Authentication::HubAuthComponent m_auth_component;
    
    // Helper methods
    void loadWebUI();
    void loadHTMLFile (const juce::File& htmlFile);
    void handleJavaScriptMessage (const juce::var& message);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TidalAudioProcessorEditor)
};
