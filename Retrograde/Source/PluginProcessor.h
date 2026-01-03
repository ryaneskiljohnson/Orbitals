/*
  ==============================================================================

    Retrograde - MIDI Motion Reversal
    MIDI FX Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <deque>

//==============================================================================
/**
    Retrograde MIDI FX Processor
    Reverses velocity contours, timing drift, or note order musically
*/
class RetrogradeAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    RetrogradeAudioProcessor();
    ~RetrogradeAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter IDs
    static const juce::String PARAM_MODE;
    static const juce::String PARAM_SCOPE;
    static const juce::String PARAM_SYMMETRY;
    static const juce::String PARAM_ECHO;
    static const juce::String PARAM_BYPASS;

    // Get parameters
    juce::AudioProcessorValueTreeState parameters;

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Note buffer for phrase reversal
    struct NoteInfo
    {
        juce::MidiMessage message;
        int samplePosition;
        double timestamp;
    };
    std::deque<NoteInfo> noteBuffer;
    int bufferSize = 8; // Number of notes to buffer

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RetrogradeAudioProcessor)
};
