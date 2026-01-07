/*
  ==============================================================================

    Fabric - Spacetime Reverb
    Audio FX Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Fabric Audio FX Processor
    Spacetime reverb with quantum physics metaphors
*/
class FabricAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    FabricAudioProcessor();
    ~FabricAudioProcessor() override;

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
        static const juce::String PARAM_SIZE;
        static const juce::String PARAM_DIFFUSION;
        static const juce::String PARAM_DAMPING;
        static const juce::String PARAM_PREDELAY;
        static const juce::String PARAM_MIX;
        static const juce::String PARAM_WETDRY;
        static const juce::String PARAM_BYPASS;

    // Get parameters
    juce::AudioProcessorValueTreeState parameters;
    
    // Audio level tracking for UI
    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Momentum smoothing per note
    std::array<float, 128> smoothedVelocities;
    
    // Apply curve to velocity
    float applyCurve(float normalizedInput, int curveType);
    
    // Reverb DSP
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> predelayLineL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> predelayLineR;
    double currentSampleRate = 44100.0;
    
    // Parameter smoothing to avoid zipper noise
    juce::SmoothedValue<float> smoothedSize;
    juce::SmoothedValue<float> smoothedDiffusion;
    juce::SmoothedValue<float> smoothedDamping;
    juce::SmoothedValue<float> smoothedPredelay;
    juce::SmoothedValue<float> smoothedMix;
    juce::SmoothedValue<float> smoothedWetDry;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FabricAudioProcessor)
};
