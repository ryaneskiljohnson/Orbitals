/*
  ==============================================================================

    BufferedSynthesiser.h
    Created: 2026
    Author:  Cymasphere Team

  ==============================================================================
    
    @fileoverview BufferedSynthesiser - Robust real-time synthesiser.
    @module BufferedSynthesiser
    
    This synthesiser extends JUCE's Synthesiser class with:
    - Band-limited oscillators for alias-free sound
    - Proper ADSR envelopes with minimum times to prevent clicks
    - Smoothed parameter changes for click-free operation
    - 32-voice polyphony with voice management
    
    Uses JUCE's standard real-time rendering approach for zero-latency playback.
*/

#pragma once

#include <JuceHeader.h>
#include "BufferedSynthVoice.h"

//==============================================================================
/**
 * @brief Robust real-time synthesiser with band-limited oscillators.
 * 
 * This class extends juce::Synthesiser to provide high-quality, click-free
 * synthesis. Key features:
 * 
 * - **Band-Limited Oscillators**: PolyBLEP for alias-free sawtooth/square
 * - **Smooth Envelopes**: ADSR with minimum attack/release to prevent clicks
 * - **Parameter Smoothing**: All changes interpolated for click-free operation
 * - **Voice Pool**: Manages 32 polyphonic voices
 * 
 * @note All operations are real-time safe
 * @note Zero latency - notes play immediately
 */
class BufferedSynthesiser : public juce::Synthesiser
{
public:
    //==============================================================================
    /**
     * @brief Constructs the synthesiser with default settings.
     */
    BufferedSynthesiser();
    
    /**
     * @brief Destructor.
     */
    ~BufferedSynthesiser() override;
    
    //==============================================================================
    /**
     * @brief Prepares the synthesiser for playback.
     * 
     * This method:
     * - Sets the sample rate for the synthesiser
     * - Initializes all voices
     * - Reports zero latency (real-time synthesis)
     * 
     * @param sampleRate Sample rate in Hz (e.g., 44100, 48000)
     * @param samplesPerBlock Expected samples per block
     * @param latencyCompensation Output parameter - always 0 for real-time synth
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock, int& latencyCompensation);
    
    /**
     * @brief Renders the next block of audio.
     * 
     * Uses JUCE's standard Synthesiser rendering which handles:
     * - MIDI event processing with sample-accurate timing
     * - Voice allocation and management
     * - Voice stealing when needed
     * 
     * @param outputBuffer Buffer to write audio to
     * @param midiMessages MIDI events to process (note on/off, etc.)
     * @param startSample Starting sample in output buffer
     * @param numSamples Number of samples to render
     */
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                        const juce::MidiBuffer& midiMessages,
                        int startSample,
                        int numSamples);
    
    //==============================================================================
    /**
     * @brief Sets the waveform for all voices.
     * @param waveform The waveform type to use
     */
    void setWaveform(BufferedSynthVoice::Waveform waveform);
    
    /**
     * @brief Sets the ADSR envelope for all voices.
     * 
     * @param attack Attack time in seconds (min 0.001s)
     * @param decay Decay time in seconds (min 0.001s)
     * @param sustain Sustain level (0.0-1.0)
     * @param release Release time in seconds (min 0.001s)
     */
    void setEnvelope(float attack, float decay, float sustain, float release);
    
    /**
     * @brief Sets the master volume for all voices.
     * @param volume Volume level (0.0-1.0)
     */
    void setVolume(float volume);
    
    /**
     * @brief Gets the latency in samples (always 0 for real-time synth).
     * @returns 0 - no latency
     */
    int getLatencySamples() const { return 0; }

private:
    //==============================================================================
    // Voice parameter state
    BufferedSynthVoice::Waveform m_currentWaveform{BufferedSynthVoice::Waveform::Sine};
    float m_attack{0.01f};
    float m_decay{0.1f};
    float m_sustain{0.8f};
    float m_release{0.1f};
    float m_volume{0.7f};
    
    /// Current sample rate
    double m_sampleRate{44100.0};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BufferedSynthesiser)
};
