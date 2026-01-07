/*
  ==============================================================================

    Entanglement - Quantum Delay
    Audio FX Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <deque>

//==============================================================================
/**
    Entanglement Audio FX Processor
    Delay with quantum physics metaphors
*/
class EntanglementAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    EntanglementAudioProcessor();
    ~EntanglementAudioProcessor() override;

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
    static const juce::String PARAM_TIME;
    static const juce::String PARAM_FEEDBACK;
    static const juce::String PARAM_MIX;
    static const juce::String PARAM_DAMPING;
    static const juce::String PARAM_BYPASS;

    // Get parameters
    juce::AudioProcessorValueTreeState parameters;
    
    // Audio level tracking for UI
    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    double sampleRate = 44100.0;
    juce::Random random;
    
    // Delay DSP
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineR;
    juce::dsp::IIR::Filter<float> dampingFilterL;
    juce::dsp::IIR::Filter<float> dampingFilterR;
    juce::dsp::IIR::Coefficients<float>::Ptr dampingCoeffsL;
    juce::dsp::IIR::Coefficients<float>::Ptr dampingCoeffsR;
    float lastDelayTimeL = 0.0f;
    float lastDelayTimeR = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EntanglementAudioProcessor)
};
