/*
  ==============================================================================

    Kepler - MIDI Orbit Generator
    MIDI FX Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

//==============================================================================
/**
    Kepler MIDI FX Processor
    Repositions notes rhythmically using Kepler's orbital mechanics laws
*/
class KeplerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    KeplerAudioProcessor();
    ~KeplerAudioProcessor() override;

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
    static const juce::String PARAM_ORBIT_COUNT;
    static const juce::String PARAM_ECCENTRICITY;
    static const juce::String PARAM_ROTATION_SPEED;
    static const juce::String PARAM_STABILITY_X;
    static const juce::String PARAM_STABILITY_Y;
    static const juce::String PARAM_PERIOD;
    static const juce::String PARAM_DIRECTION;
    static const juce::String PARAM_BYPASS;

    // Get parameters
    juce::AudioProcessorValueTreeState parameters;

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    double phase = 0.0;
    double sampleRate = 44100.0;
    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeplerAudioProcessor)
};
