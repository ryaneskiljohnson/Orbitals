/*
  ==============================================================================

    PixelPulse - Retro Rhythm Game
    Video Game Music Generator Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../../_Shared/Source/WebView/OrbitalsWebViewHost.h"

//==============================================================================
class PixelPulseAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    PixelPulseAudioProcessorEditor (PixelPulseAudioProcessor&);
    ~PixelPulseAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void sendMeteringData();
    
    // Mouse handling
    void mouseDown(const juce::MouseEvent& e) override;
    
    // Keyboard handling
    bool keyPressed(const juce::KeyPress& key) override;
    
    // MIDI note notification
    void notifyMIDINote(int noteNumber, int velocity);

private:
    PixelPulseAudioProcessor& audioProcessor;
    OrbitalsWebViewHost webViewHost;
    
    // Authentication state
    bool isAuthorized = false;
    
    void handleJavaScriptMessage (const juce::var& message);
    bool checkAuthorization();
    static juce::File getAuthFile();
    static juce::String loadAndDecryptLicenseFile();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PixelPulseAudioProcessorEditor)
};
