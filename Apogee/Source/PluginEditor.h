/*
  ==============================================================================

    Apogee - Velocity & Energy Booster
    MIDI FX Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../_Shared/Authentication/HubAuthComponent.h"
#include "../../_Shared/Authentication/HubAuthComponent.h"

//==============================================================================
/**
    Apogee Plugin Editor - WebView-based UI
*/
class ApogeeAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ApogeeAudioProcessorEditor (ApogeeAudioProcessor&);
    ~ApogeeAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ApogeeAudioProcessor& audioProcessor;
    
    // WebView for CSS-based UI
    std::unique_ptr<juce::WebBrowserComponent> webView;
    
    // Helper methods
    void loadWebUI();
    void loadHTMLFile (const juce::File& htmlFile);
    void handleJavaScriptMessage (const juce::var& message);

       // Authentication component
    NNAudio::Authentication::HubAuthComponent m_auth_component;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ApogeeAudioProcessorEditor)
};
