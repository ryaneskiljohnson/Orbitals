/*
  ==============================================================================

    BufferedSynthVoice.h
    Created: 2026
    Author:  Cymasphere Team

  ==============================================================================
    
    @fileoverview BufferedSynthVoice - Robust synthesizer voice with click-free rendering.
    @module BufferedSynthVoice
    
    This voice implementation is designed for extremely robust audio rendering
    with no pops, clicks, or crackling noises. It features:
    - Band-limited oscillators (Sine, Sawtooth, Square, Triangle)
    - Smooth ADSR envelopes with minimum attack/release times
    - Phase continuity between notes
    - Smooth parameter interpolation using juce::SmoothedValue
    - Proper handling of startSample offsets for sample-accurate MIDI
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * @brief Simple sound class for the buffered synthesizer.
 * 
 * This sound responds to all MIDI notes and channels, making it suitable
 * for a general-purpose synthesizer.
 */
class BufferedSynthSound : public juce::SynthesiserSound
{
public:
    BufferedSynthSound() {}
    
    /**
     * @brief Determines if this sound should play for a given MIDI note.
     * @param midiNoteNumber The MIDI note number (0-127)
     * @returns Always true - this sound responds to all notes
     */
    bool appliesToNote(int midiNoteNumber) override
    {
        juce::ignoreUnused(midiNoteNumber);
        return true;
    }
    
    /**
     * @brief Determines if this sound should play for a given MIDI channel.
     * @param midiChannel The MIDI channel (1-16)
     * @returns Always true - this sound responds to all channels
     */
    bool appliesToChannel(int midiChannel) override
    {
        juce::ignoreUnused(midiChannel);
        return true;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BufferedSynthSound)
};

//==============================================================================
/**
 * @brief Robust synthesizer voice with band-limited oscillators and smooth envelopes.
 * 
 * This voice is designed to produce pristine audio without clicks, pops, or
 * crackling. Key features:
 * - Band-limited waveforms using PolyBLEP algorithm for saw/square
 * - ADSR envelope with configurable parameters (min 1ms attack/release)
 * - Smooth parameter changes using juce::SmoothedValue
 * - Phase continuity maintenance between notes
 * - Proper startSample handling for sample-accurate MIDI
 * 
 * @note All audio processing is lock-free and real-time safe
 */
class BufferedSynthVoice : public juce::SynthesiserVoice
{
public:
    //==============================================================================
    /**
     * @brief Oscillator waveform types.
     * 
     * All waveforms are band-limited to prevent aliasing artifacts.
     */
    enum class Waveform
    {
        Sine,      ///< Pure sine wave (naturally band-limited)
        Sawtooth,  ///< Sawtooth wave (PolyBLEP band-limited)
        Square,    ///< Square wave (PolyBLEP band-limited)
        Triangle   ///< Triangle wave (integrated from band-limited square)
    };
    
    //==============================================================================
    BufferedSynthVoice();
    
    //==============================================================================
    // SynthesiserVoice interface
    
    /**
     * @brief Checks if this voice can play the given sound.
     * @param sound The sound to check
     * @returns True if sound is a BufferedSynthSound
     */
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    
    /**
     * @brief Starts playing a note.
     * 
     * This method initializes the oscillator, triggers the envelope, and
     * ensures smooth phase transitions to prevent clicks.
     * 
     * @param midiNoteNumber The MIDI note number (0-127)
     * @param velocity Note velocity (0.0-1.0)
     * @param sound The sound object (unused)
     * @param currentPitchWheelPosition Current pitch wheel position (unused)
     * 
     * @note Phase is only reset at zero crossings or during envelope attack
     * @note Frequency changes are smoothed over multiple samples
     */
    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound* sound,
                   int currentPitchWheelPosition) override;
    
    /**
     * @brief Stops playing a note.
     * 
     * Triggers the envelope release phase for a smooth fade-out.
     * 
     * @param velocity Release velocity (0.0-1.0, unused)
     * @param allowTailOff If true, allows envelope to complete release phase
     */
    void stopNote(float velocity, bool allowTailOff) override;
    
    /**
     * @brief Handles pitch wheel changes (currently not implemented).
     * @param newPitchWheelValue The new pitch wheel value
     */
    void pitchWheelMoved(int newPitchWheelValue) override;
    
    /**
     * @brief Handles MIDI controller changes (currently not implemented).
     * @param controllerNumber The controller number
     * @param newControllerValue The new controller value
     */
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    
    /**
     * @brief Renders the next block of audio samples.
     * 
     * This is the main audio rendering function. It processes the oscillator,
     * envelope, and applies smooth parameter changes to prevent discontinuities.
     * 
     * @param outputBuffer The buffer to write audio to (stereo)
     * @param startSample Starting sample index within the buffer
     * @param numSamples Number of samples to render
     * 
     * @note Respects startSample for sample-accurate MIDI timing
     * @note Uses smoothed values for all parameter changes
     * @note Applies envelope at the end to ensure clean fades
     */
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                        int startSample, int numSamples) override;
    
    //==============================================================================
    // Parameter setters
    
    /**
     * @brief Sets the oscillator waveform type.
     * @param waveform The waveform to use
     */
    void setWaveform(Waveform waveform);
    
    /**
     * @brief Sets the ADSR envelope parameters.
     * 
     * @param attack Attack time in seconds (minimum 0.001s)
     * @param decay Decay time in seconds (minimum 0.001s)
     * @param sustain Sustain level (0.0-1.0)
     * @param release Release time in seconds (minimum 0.001s)
     * 
     * @note All times are clamped to safe minimum values to prevent clicks
     */
    void setEnvelope(float attack, float decay, float sustain, float release);
    
    /**
     * @brief Sets the master volume level.
     * @param volume Volume level (0.0-1.0)
     */
    void setVolume(float volume);
    
    /**
     * @brief Prepares the voice for playback at the given sample rate.
     * 
     * @param sampleRate Sample rate in Hz
     * @param samplesPerBlock Expected samples per block (for smoothing)
     * 
     * @note Initializes all smoothed values and resets state
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock);

private:
    //==============================================================================
    // Oscillator generation
    
    /**
     * @brief Generates a single sample from the oscillator.
     * 
     * @param phase Current oscillator phase (0.0-1.0)
     * @param phaseIncrement Phase increment per sample
     * @returns Generated audio sample (-1.0 to 1.0)
     * 
     * @note Uses PolyBLEP for band-limited sawtooth and square waves
     */
    float generateOscillatorSample(double phase, double phaseIncrement);
    
    /**
     * @brief PolyBLEP residual calculation for band-limited waveforms.
     * 
     * This function calculates the polynomial band-limited step function
     * used to reduce aliasing in non-sine waveforms.
     * 
     * @param t Phase position (0.0-1.0)
     * @param dt Phase increment per sample
     * @returns PolyBLEP correction value
     * 
     * @note See "Alias-Free Digital Synthesis of Classic Analog Waveforms"
     *       by Välimäki and Huovilainen (2007)
     */
    float polyBLEP(double t, double dt);
    
    //==============================================================================
    // Member variables
    
    // Oscillator state
    Waveform m_waveform{Waveform::Sine};
    double m_phase{0.0};
    double m_phaseIncrement{0.0};
    double m_currentFrequency{0.0};
    
    // Smoothed parameters (for click-free parameter changes)
    juce::SmoothedValue<float> m_smoothedFrequency;
    juce::SmoothedValue<float> m_smoothedVolume;
    
    // Envelope
    juce::ADSR m_envelope;
    juce::ADSR::Parameters m_envelopeParams;
    
    // Voice state
    float m_noteVelocity{0.0f};
    float m_masterVolume{0.7f};
    
    // Triangle wave state (for integrated band-limited triangle)
    float m_triangleIntegrator{0.0f};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BufferedSynthVoice)
};
