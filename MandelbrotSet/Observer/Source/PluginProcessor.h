/*
  ==============================================================================

    Observer - Wavefunction Collapse Dynamic EQ
    Audio FX Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <map>

//==============================================================================
/**
    Observer Audio FX Processor
    Dynamic EQ with quantum physics metaphors
*/
class ObserverAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ObserverAudioProcessor();
    ~ObserverAudioProcessor() override;

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
    static const juce::String PARAM_FREQUENCY;
    static const juce::String PARAM_RATIO;
    static const juce::String PARAM_ATTACK;
    static const juce::String PARAM_RELEASE;
    static const juce::String PARAM_BYPASS;

    // Get parameters
    juce::AudioProcessorValueTreeState parameters;
    
    // Audio level tracking for UI
    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Dynamic EQ DSP
    juce::dsp::IIR::Filter<float> bandFilterL;
    juce::dsp::IIR::Filter<float> bandFilterR;
    juce::dsp::IIR::Coefficients<float>::Ptr bandCoeffsL;
    juce::dsp::IIR::Coefficients<float>::Ptr bandCoeffsR;
    double sampleRate = 44100.0;
    float envelope = 0.0f;
    float currentGain = 0.0f;
    static constexpr float qValue = 2.0f; // Bandwidth

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ObserverAudioProcessor)
};
