/*
  ==============================================================================

    Apogee - Velocity & Energy Booster
    MIDI FX Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Apogee MIDI FX Processor
    Expands expressive range upward with trajectory-based velocity boosting
*/
class ApogeeAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ApogeeAudioProcessor();
    ~ApogeeAudioProcessor() override;

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
    static const juce::String PARAM_LIFT;
    static const juce::String PARAM_CURVE;
    static const juce::String PARAM_CEILING;
    static const juce::String PARAM_MOMENTUM;
    static const juce::String PARAM_BYPASS;

    // Get parameters
    juce::AudioProcessorValueTreeState parameters;

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Momentum smoothing per note
    std::array<float, 128> smoothedVelocities;
    
    // Apply curve to velocity
    float applyCurve(float normalizedInput, int curveType);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ApogeeAudioProcessor)
};
