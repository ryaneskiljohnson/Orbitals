/*
  ==============================================================================

    Perihelion - Velocity Sculptor
    MIDI FX Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class PerihelionAudioProcessor  : public juce::AudioProcessor
{
public:
    PerihelionAudioProcessor();
    ~PerihelionAudioProcessor() override;

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

    static const juce::String PARAM_GRAVITY;
    static const juce::String PARAM_ORBIT;
    static const juce::String PARAM_SOLAR_POINT;
    static const juce::String PARAM_BIAS;
    static const juce::String PARAM_BYPASS;

    juce::AudioProcessorValueTreeState parameters;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PerihelionAudioProcessor)
};
