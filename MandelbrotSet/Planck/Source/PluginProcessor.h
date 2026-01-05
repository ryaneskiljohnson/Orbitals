/*
  ==============================================================================

    Planck - Quantum Scale EQ
    Audio FX Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class PlanckAudioProcessor  : public juce::AudioProcessor
{
public:
    PlanckAudioProcessor();
    ~PlanckAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static const juce::String PARAM_LOWFREQ;
    static const juce::String PARAM_LOWGAIN;
    static const juce::String PARAM_MIDFREQ;
    static const juce::String PARAM_MIDGAIN;
    static const juce::String PARAM_HIGHFREQ;
    static const juce::String PARAM_HIGHGAIN;
    static const juce::String PARAM_Q;
    static const juce::String PARAM_BYPASS;

    juce::AudioProcessorValueTreeState parameters;
    
    // Audio level tracking for UI
    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlanckAudioProcessor)
};
