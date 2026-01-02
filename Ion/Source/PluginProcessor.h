/*
  ==============================================================================

    Ion - Harmonic Constraint Engine
    MIDI FX Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <map>

//==============================================================================
/**
    Ion MIDI FX Processor
    Keeps MIDI within a harmonic field using magnetic attraction physics
*/
class IonAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    IonAudioProcessor();
    ~IonAudioProcessor() override;

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
    static const juce::String PARAM_ROOT;
    static const juce::String PARAM_SCALE;
    static const juce::String PARAM_FIELD_STRENGTH;
    static const juce::String PARAM_ATTRACTION_STRENGTH;
    static const juce::String PARAM_ATTRACTION_TYPE;
    static const juce::String PARAM_BYPASS;

    // Get parameters
    juce::AudioProcessorValueTreeState parameters;

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Get scale notes relative to root
    std::vector<int> getScaleNotes(int root, int scaleIndex);
    
    // Find nearest scale note
    int findNearestScaleNote(int inputNote, const std::vector<int>& scaleNotes);
    
    // State tracking
    std::array<float, 128> notePositions; // For PULL mode gradual attraction
    std::map<int, int> noteTargets; // Maps original note to target note for note-off
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IonAudioProcessor)
};
