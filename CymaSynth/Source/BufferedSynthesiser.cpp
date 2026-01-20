/*
  ==============================================================================

    BufferedSynthesiser.cpp
    Created: 2026
    Author:  Cymasphere Team

  ==============================================================================
    
    @fileoverview Implementation of BufferedSynthesiser for real-time synthesis.
    @module BufferedSynthesiser
    
    This implementation provides robust audio rendering through:
    - JUCE's standard sample-accurate MIDI processing
    - Band-limited oscillators for alias-free sound
    - Smooth parameter interpolation
    - 32-voice polyphony with voice management
*/

#include "BufferedSynthesiser.h"

//==============================================================================
// Constructor & Destructor
//==============================================================================

BufferedSynthesiser::BufferedSynthesiser()
{
    // Add our sound (applies to all notes/channels)
    addSound(new BufferedSynthSound());
    
    // Add 32 voices for polyphony
    const int numVoices = 32;
    for (int i = 0; i < numVoices; ++i)
    {
        addVoice(new BufferedSynthVoice());
    }
    
    DBG("BufferedSynthesiser: Created with " << numVoices << " voices");
}

BufferedSynthesiser::~BufferedSynthesiser()
{
    // Cleanup is automatic via JUCE - it owns voices and sounds
}

//==============================================================================
// Preparation
//==============================================================================

void BufferedSynthesiser::prepareToPlay(double sampleRate, int samplesPerBlock, 
                                       int& latencyCompensation)
{
    juce::ignoreUnused(samplesPerBlock);
    
    m_sampleRate = sampleRate;
    
    // Real-time synthesizer has zero latency
    latencyCompensation = 0;
    
    // Prepare the base Synthesiser class
    setCurrentPlaybackSampleRate(sampleRate);
    
    // Prepare all voices
    for (int i = 0; i < getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<BufferedSynthVoice*>(getVoice(i)))
        {
            voice->prepareToPlay(sampleRate, samplesPerBlock);
            
            // Apply current parameters to voice
            voice->setWaveform(m_currentWaveform);
            voice->setEnvelope(m_attack, m_decay, m_sustain, m_release);
            voice->setVolume(m_volume);
        }
    }
    
    DBG("BufferedSynthesiser: Prepared at " << sampleRate << " Hz, 0 latency");
}

//==============================================================================
// Rendering
//==============================================================================

void BufferedSynthesiser::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                         const juce::MidiBuffer& midiMessages,
                                         int startSample,
                                         int numSamples)
{
    // Use JUCE's standard Synthesiser rendering which handles:
    // - Sample-accurate MIDI event processing
    // - Voice allocation and voice stealing
    // - Proper timing of note on/off events
    Synthesiser::renderNextBlock(outputBuffer, midiMessages, startSample, numSamples);
}

//==============================================================================
// Parameter Setters
//==============================================================================

void BufferedSynthesiser::setWaveform(BufferedSynthVoice::Waveform waveform)
{
    m_currentWaveform = waveform;
    
    // Apply to all voices
    for (int i = 0; i < getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<BufferedSynthVoice*>(getVoice(i)))
        {
            voice->setWaveform(waveform);
        }
    }
}

void BufferedSynthesiser::setEnvelope(float attack, float decay, float sustain, float release)
{
    // Store parameters
    m_attack = attack;
    m_decay = decay;
    m_sustain = sustain;
    m_release = release;
    
    // Apply to all voices
    for (int i = 0; i < getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<BufferedSynthVoice*>(getVoice(i)))
        {
            voice->setEnvelope(attack, decay, sustain, release);
        }
    }
}

void BufferedSynthesiser::setVolume(float volume)
{
    m_volume = volume;
    
    // Apply to all voices
    for (int i = 0; i < getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<BufferedSynthVoice*>(getVoice(i)))
        {
            voice->setVolume(volume);
        }
    }
}
