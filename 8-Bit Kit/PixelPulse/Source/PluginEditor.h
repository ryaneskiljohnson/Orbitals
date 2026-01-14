/*
  ==============================================================================

    PixelPulse - Retro Rhythm Game
    Video Game Music Generator Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// Helper class to handle WebBrowserComponent callbacks (like NNAudioAccess)
class WebBrowserWithCallbacks : public juce::WebBrowserComponent
{
public:
    WebBrowserWithCallbacks(const juce::WebBrowserComponent::Options& options)
        : juce::WebBrowserComponent(options) {}
    
    std::function<void(const juce::String&)> onPageFinishedLoading;
    
    void pageFinishedLoading(const juce::String& url) override
    {
        juce::WebBrowserComponent::pageFinishedLoading(url);
        if (onPageFinishedLoading) {
            onPageFinishedLoading(url);
        }
    }
};

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
    std::unique_ptr<WebBrowserWithCallbacks> webView;
    
    // Authentication state
    bool isAuthorized = false;
    
    void loadWebUI();
    void loadAuthScreen();
    void loadHTMLFile (const juce::File& htmlFile);
    void handleJavaScriptMessage (const juce::var& message);
    bool checkAuthorization();
    static juce::File getAuthFile();
    static juce::String loadAndDecryptLicenseFile();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PixelPulseAudioProcessorEditor)
};
