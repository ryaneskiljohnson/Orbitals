/*
  ==============================================================================

    Tidal - Rhythmic Push & Pull
    MIDI FX Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>

//==============================================================================
/**
    Tidal MIDI FX Processor
    Applies tempo-synced timing modulation using wave patterns
*/
class TidalAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TidalAudioProcessor();
    ~TidalAudioProcessor() override;

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
    static const juce::String PARAM_AMPLITUDE;
    static const juce::String PARAM_PHASE;
    static const juce::String PARAM_RATE;
    static const juce::String PARAM_SHAPE;
    static const juce::String PARAM_BYPASS;

    // Get parameters
    juce::AudioProcessorValueTreeState parameters;

private:
    //==============================================================================
    // Parameter layout
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Wave timing modulation
    double phase = 0.0;
    double sampleRate = 44100.0;
    
    // Helper to get wave value at given phase
    double getWaveValue(double phaseIn, int shapeIndex);
    
    // Helper to get rate multiplier
    double getRateMultiplier(int rateIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TidalAudioProcessor)
};
