/*
  ==============================================================================

    BufferedSynthVoice.cpp
    Created: 2026
    Author:  Cymasphere Team

  ==============================================================================
    
    @fileoverview Implementation of BufferedSynthVoice - Robust synthesizer voice.
    @module BufferedSynthVoice
    
    This implementation provides pristine audio quality through:
    - PolyBLEP band-limited oscillators
    - Smooth envelope curves with minimum times
    - Phase continuity management
    - Parameter smoothing for click-free changes
*/

#include "BufferedSynthVoice.h"

//==============================================================================
// Constructor
//==============================================================================

BufferedSynthVoice::BufferedSynthVoice()
{
    // Set safe default envelope parameters (prevents clicks)
    m_envelopeParams.attack = 0.01f;   // 10ms minimum attack
    m_envelopeParams.decay = 0.1f;     // 100ms decay
    m_envelopeParams.sustain = 0.8f;   // 80% sustain level
    m_envelopeParams.release = 0.1f;   // 100ms release
    
    m_envelope.setParameters(m_envelopeParams);
}

//==============================================================================
// SynthesiserVoice Interface
//==============================================================================

bool BufferedSynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<BufferedSynthSound*>(sound) != nullptr;
}

void BufferedSynthVoice::startNote(int midiNoteNumber, float velocity,
                                   juce::SynthesiserSound* /*sound*/,
                                   int /*currentPitchWheelPosition*/)
{
    // Store velocity for amplitude scaling
    m_noteVelocity = velocity;
    
    // Calculate target frequency from MIDI note
    const float targetFrequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    
    // Check if we should maintain phase continuity or reset
    // Only reset phase if:
    // 1. This is a fresh voice (not currently playing)
    // 2. Or the frequency change is very large (more than an octave)
    const bool shouldResetPhase = !m_envelope.isActive() || 
                                  (m_currentFrequency > 0.0 && 
                                   std::abs(targetFrequency - m_currentFrequency) / m_currentFrequency > 1.0);
    
    if (shouldResetPhase)
    {
        // Reset phase to zero (start of waveform cycle)
        // This is safe because the envelope attack will smooth the onset
        m_phase = 0.0;
        m_triangleIntegrator = 0.0f;
    }
    else
    {
        // Maintain current phase position for smooth frequency transitions
        // This prevents clicks when playing legato or retriggering quickly
    }
    
    // Update frequency using smoothed value for click-free transitions
    m_currentFrequency = targetFrequency;
    m_smoothedFrequency.setTargetValue(static_cast<float>(targetFrequency));
    
    // Calculate initial phase increment
    if (getSampleRate() > 0.0)
    {
        m_phaseIncrement = targetFrequency / getSampleRate();
    }
    
    // Trigger envelope attack phase
    m_envelope.noteOn();
}

void BufferedSynthVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        // Trigger envelope release phase for smooth fade-out
        m_envelope.noteOff();
    }
    else
    {
        // Immediate stop (rare, but envelope will still provide minimal fade)
        m_envelope.reset();
        clearCurrentNote();
    }
}

void BufferedSynthVoice::pitchWheelMoved(int /*newPitchWheelValue*/)
{
    // TODO: Implement pitch bend
    // Would adjust m_smoothedFrequency based on pitch wheel range
}

void BufferedSynthVoice::controllerMoved(int /*controllerNumber*/, int /*newControllerValue*/)
{
    // TODO: Implement MIDI CC handling
    // Could control filter, volume, or other parameters
}

void BufferedSynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                        int startSample, int numSamples)
{
    // Early exit if envelope has finished
    if (!m_envelope.isActive())
    {
        clearCurrentNote();
        return;
    }
    
    const int numChannels = outputBuffer.getNumChannels();
    
    // Render each sample
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        // Get current frequency from smoothed value (prevents clicks on frequency changes)
        const float currentFreq = m_smoothedFrequency.getNextValue();
        
        // Update phase increment based on smoothed frequency
        if (getSampleRate() > 0.0)
        {
            m_phaseIncrement = currentFreq / getSampleRate();
        }
        
        // Generate oscillator sample
        const float oscSample = generateOscillatorSample(m_phase, m_phaseIncrement);
        
        // Advance phase and wrap to [0, 1)
        m_phase += m_phaseIncrement;
        if (m_phase >= 1.0)
        {
            m_phase -= 1.0;
        }
        
        // Get envelope value
        const float envelopeValue = m_envelope.getNextSample();
        
        // Apply velocity, volume, and envelope
        const float volume = m_smoothedVolume.getNextValue();
        float sample = oscSample * envelopeValue * m_noteVelocity * volume * m_masterVolume;
        
        // Write to output buffer (mono-to-stereo or stereo)
        const int bufferIndex = startSample + sampleIndex;
        
        if (numChannels == 1)
        {
            // Mono output
            outputBuffer.addSample(0, bufferIndex, sample);
        }
        else if (numChannels >= 2)
        {
            // Stereo output (same signal to both channels)
            outputBuffer.addSample(0, bufferIndex, sample);
            outputBuffer.addSample(1, bufferIndex, sample);
        }
    }
}

//==============================================================================
// Parameter Setters
//==============================================================================

void BufferedSynthVoice::setWaveform(Waveform waveform)
{
    m_waveform = waveform;
}

void BufferedSynthVoice::setEnvelope(float attack, float decay, float sustain, float release)
{
    // Clamp to safe minimum values to prevent clicks
    m_envelopeParams.attack = juce::jmax(0.001f, attack);
    m_envelopeParams.decay = juce::jmax(0.001f, decay);
    m_envelopeParams.sustain = juce::jlimit(0.0f, 1.0f, sustain);
    m_envelopeParams.release = juce::jmax(0.001f, release);
    
    m_envelope.setParameters(m_envelopeParams);
}

void BufferedSynthVoice::setVolume(float volume)
{
    m_smoothedVolume.setTargetValue(juce::jlimit(0.0f, 1.0f, volume));
}

void BufferedSynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    
    // Initialize envelope with sample rate
    m_envelope.setSampleRate(sampleRate);
    m_envelope.setParameters(m_envelopeParams);
    
    // Initialize smoothed values
    // Ramp time of ~10ms to smooth parameter changes
    const float rampLengthSeconds = 0.01f;
    const int rampLengthSamples = static_cast<int>(rampLengthSeconds * sampleRate);
    
    m_smoothedFrequency.reset(sampleRate, rampLengthSeconds);
    m_smoothedFrequency.setCurrentAndTargetValue(440.0f); // A4 default
    
    m_smoothedVolume.reset(sampleRate, rampLengthSeconds);
    m_smoothedVolume.setCurrentAndTargetValue(m_masterVolume);
    
    // Reset oscillator state
    m_phase = 0.0;
    m_phaseIncrement = 0.0;
    m_triangleIntegrator = 0.0f;
}

//==============================================================================
// Oscillator Generation
//==============================================================================

float BufferedSynthVoice::generateOscillatorSample(double phase, double phaseIncrement)
{
    float sample = 0.0f;
    
    switch (m_waveform)
    {
        case Waveform::Sine:
        {
            // Pure sine wave - naturally band-limited
            sample = std::sin(phase * juce::MathConstants<double>::twoPi);
            break;
        }
        
        case Waveform::Sawtooth:
        {
            // Band-limited sawtooth using PolyBLEP
            // Start with naive sawtooth: ramp from -1 to 1
            sample = static_cast<float>(2.0 * phase - 1.0);
            
            // Apply PolyBLEP correction at discontinuities
            sample -= polyBLEP(phase, phaseIncrement);
            
            break;
        }
        
        case Waveform::Square:
        {
            // Band-limited square wave using PolyBLEP
            // Start with naive square: +1 for first half, -1 for second half
            sample = (phase < 0.5) ? 1.0f : -1.0f;
            
            // Apply PolyBLEP at both edges of the square wave
            sample += polyBLEP(phase, phaseIncrement);
            sample -= polyBLEP(std::fmod(phase + 0.5, 1.0), phaseIncrement);
            
            break;
        }
        
        case Waveform::Triangle:
        {
            // Band-limited triangle wave via integration of band-limited square
            // This method ensures the triangle is also band-limited
            
            // Generate band-limited square
            float squareSample = (phase < 0.5) ? 1.0f : -1.0f;
            squareSample += polyBLEP(phase, phaseIncrement);
            squareSample -= polyBLEP(std::fmod(phase + 0.5, 1.0), phaseIncrement);
            
            // Integrate the square wave to get triangle
            // Use a leaky integrator to prevent DC buildup
            const float integrationCoeff = static_cast<float>(4.0 * phaseIncrement);
            m_triangleIntegrator = m_triangleIntegrator * 0.999f + squareSample * integrationCoeff;
            
            // The integrator output is the triangle wave
            sample = m_triangleIntegrator;
            
            break;
        }
    }
    
    return sample;
}

float BufferedSynthVoice::polyBLEP(double t, double dt)
{
    // PolyBLEP (Polynomial Band-Limited Step)
    // This algorithm reduces aliasing at discontinuities in waveforms
    // Reference: "Alias-Free Digital Synthesis of Classic Analog Waveforms"
    //            by Välimäki and Huovilainen (2007)
    
    // Handle discontinuity at phase = 0 (beginning of cycle)
    if (t < dt)
    {
        // We're very close to the start of the cycle
        t = t / dt;
        return static_cast<float>(t + t - t * t - 1.0);
    }
    // Handle discontinuity at phase = 1 (end of cycle)
    else if (t > 1.0 - dt)
    {
        // We're very close to the end of the cycle
        t = (t - 1.0) / dt;
        return static_cast<float>(t * t + t + t + 1.0);
    }
    
    // No discontinuity nearby, return 0
    return 0.0f;
}
