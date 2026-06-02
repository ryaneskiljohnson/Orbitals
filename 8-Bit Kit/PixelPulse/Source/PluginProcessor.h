/*
  ==============================================================================

    PixelPulse - Retro Rhythm Game
    Video Game Music Generator Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>

//==============================================================================
/**
    PixelPulse Audio FX Processor
    Rhythm game that generates music based on player interaction
*/
class PixelPulseAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PixelPulseAudioProcessor();
    ~PixelPulseAudioProcessor() override;

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
    static const juce::String PARAM_TEMPO;
    static const juce::String PARAM_SCALE;
    static const juce::String PARAM_DIFFICULTY;
    static const juce::String PARAM_VOLUME;
    static const juce::String PARAM_BYPASS;

    // Get parameters
    juce::AudioProcessorValueTreeState parameters;
    
    // Audio level tracking for UI
    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };
    
    // Game state (for MIDI generation)
    std::atomic<bool> gameRunning { false };

    /** @brief Queue a note-on or note-off from the UI thread. @param noteNumber MIDI note 0–127. @param velocity Note-on velocity; 0 sends note-off. */
    void queueMidiNote (int noteNumber, int velocity) noexcept;

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    double sampleRate = 44100.0;
    
    // Game timing
    double gameTime = 0.0;
    double beatInterval = 0.0;
    bool previousBypass = false;

    struct PendingMidiEvent
    {
        int note = 0;
        int velocity = 0;
    };

    static constexpr int maxPendingMidiEvents = 512;
    juce::AbstractFifo pendingMidiFifo { maxPendingMidiEvents };
    std::array<PendingMidiEvent, maxPendingMidiEvents> pendingMidiBuffer {};

    /** @brief Emit queued MIDI events into the host buffer. @param midiMessages Output MIDI buffer for this block. */
    void flushPendingMidi (juce::MidiBuffer& midiMessages) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PixelPulseAudioProcessor)
};
