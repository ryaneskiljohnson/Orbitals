/*
  ==============================================================================

    Hadron - Timing Stabilizer
    MIDI FX Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../../_Shared/Source/WebView/OrbitalsWebViewHost.h"

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
    
    // MIDI note notification
    void notifyMIDINote(int noteNumber, int velocity);

private:
    HadronAudioProcessor& audioProcessor;
    OrbitalsWebViewHost webViewHost;
    
    // Authentication state
    bool isAuthorized = false;
    
    void handleJavaScriptMessage (const juce::var& message);
    bool checkAuthorization();
    static juce::File getAuthFile();
    static juce::String loadAndDecryptLicenseFile();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HadronAudioProcessorEditor)
};
