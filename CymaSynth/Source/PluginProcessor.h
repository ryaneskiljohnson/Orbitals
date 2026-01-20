/*
  ==============================================================================

    CymaSynth
    Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BufferedSynthesiser.h"

//==============================================================================
/**
    CymaSynth Plugin Processor
*/
class CymaSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    CymaSynthAudioProcessor();
    ~CymaSynthAudioProcessor() override;

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
    bool isSynth() const;
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
    // BufferedSynthesiser control methods
    void setBufferedSynthEnabled(bool enabled);
    bool isBufferedSynthEnabled() const;
    void setBufferedSynthWaveform(int waveformIndex); // 0=Sine, 1=Saw, 2=Square, 3=Triangle
    void setBufferedSynthEnvelope(float attack, float decay, float sustain, float release);
    void setBufferedSynthVolume(float volume);
    
    /**
     * @brief Sets the master fader gain (applied to final mix bus).
     * @param gain Linear gain (0.0-1.0+)
     */
    void setMasterFaderGain(float gain);
    
    /**
     * @brief Gets the current master fader gain.
     * @return Linear gain value
     */
    float getMasterFaderGain() const;

    //==============================================================================
    // Get parameters
    juce::AudioProcessorValueTreeState parameters;

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    double sampleRate = 44100.0;
    
    // Buffered Synthesiser (robust, click-free audio rendering)
    std::unique_ptr<BufferedSynthesiser> m_synth;
    bool m_bufferedSynthEnabled = true; // Enable by default
    
    // Master fader gain (applied to final mix bus)
    std::atomic<float> m_masterFaderGain{1.0f}; ///< Master fader linear gain (0.0-1.0+)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CymaSynthAudioProcessor)
};
