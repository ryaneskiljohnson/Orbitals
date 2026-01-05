/*
  ==============================================================================

    Quarks - Color Charge Gate
    Audio FX Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

//==============================================================================
/**
    Quarks Audio FX Processor
    Gate/Expander with quantum physics metaphors
*/
class QuarksAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    QuarksAudioProcessor();
    ~QuarksAudioProcessor() override;

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
    static const juce::String PARAM_THRESHOLD;
    static const juce::String PARAM_RATIO;
    static const juce::String PARAM_ATTACK;
    static const juce::String PARAM_RELEASE;
    static const juce::String PARAM_RANGE;
    static const juce::String PARAM_BYPASS;

    // Get parameters
    juce::AudioProcessorValueTreeState parameters;
    
    // Audio level tracking for UI
    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    double phase = 0.0;
    double sampleRate = 44100.0;
    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuarksAudioProcessor)
};
