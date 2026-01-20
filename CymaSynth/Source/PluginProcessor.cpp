/*
  ==============================================================================

    CymaSynth
    Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BufferedSynthesiser.h"

//==============================================================================
CymaSynthAudioProcessor::CymaSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
    :
#endif
      parameters (*this, nullptr, juce::Identifier ("CymaSynth"), createParameterLayout())
{
    // Create BufferedSynthesiser instance
    m_synth = std::make_unique<BufferedSynthesiser>();
    
    // Set default parameters
    m_synth->setWaveform(BufferedSynthVoice::Waveform::Sine);
    m_synth->setEnvelope(0.01f, 0.1f, 0.8f, 0.1f); // A, D, S, R
    m_synth->setVolume(0.7f);
}

CymaSynthAudioProcessor::~CymaSynthAudioProcessor()
{
    // Cleanup is automatic - unique_ptr handles deletion
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CymaSynthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    // No parameters - all DSP and MIDI functionality removed
    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String CymaSynthAudioProcessor::getName() const { return JucePlugin_Name; }
bool CymaSynthAudioProcessor::acceptsMidi() const { return true; }
bool CymaSynthAudioProcessor::producesMidi() const { return false; }
bool CymaSynthAudioProcessor::isMidiEffect() const { return false; }
bool CymaSynthAudioProcessor::isSynth() const { return true; }
double CymaSynthAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int CymaSynthAudioProcessor::getNumPrograms() { return 1; }
int CymaSynthAudioProcessor::getCurrentProgram() { return 0; }
void CymaSynthAudioProcessor::setCurrentProgram (int) {}
const juce::String CymaSynthAudioProcessor::getProgramName (int) { return {}; }
void CymaSynthAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void CymaSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    
    // Prepare BufferedSynthesiser (real-time, zero latency)
    if (m_synth)
    {
        int latencyCompensation = 0;
        m_synth->prepareToPlay(sampleRate, samplesPerBlock, latencyCompensation);
    }
}

void CymaSynthAudioProcessor::releaseResources()
{
    // Release resources is automatic - JUCE handles it
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CymaSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
  #endif
}
#endif

void CymaSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Clear audio buffer
    buffer.clear();
    
    // Process synth if enabled
    if (m_bufferedSynthEnabled && m_synth)
    {
        // Render synth audio
        m_synth->renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    }
    
    // Apply master fader gain
    const float masterGain = m_masterFaderGain.load();
    if (masterGain != 1.0f)
    {
        buffer.applyGain(masterGain);
    }
}

//==============================================================================
bool CymaSynthAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* CymaSynthAudioProcessor::createEditor()
{
    return new CymaSynthAudioProcessorEditor (*this);
}

//==============================================================================
void CymaSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void CymaSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// BufferedSynthesiser Control Methods
//==============================================================================

void CymaSynthAudioProcessor::setBufferedSynthEnabled(bool enabled)
{
    m_bufferedSynthEnabled = enabled;
    DBG("Synth " << (enabled ? "enabled" : "disabled"));
}

bool CymaSynthAudioProcessor::isBufferedSynthEnabled() const
{
    return m_bufferedSynthEnabled;
}

void CymaSynthAudioProcessor::setBufferedSynthWaveform(int waveformIndex)
{
    BufferedSynthVoice::Waveform waveform;
    
    switch (waveformIndex)
    {
        case 0:  waveform = BufferedSynthVoice::Waveform::Sine; break;
        case 1:  waveform = BufferedSynthVoice::Waveform::Sawtooth; break;
        case 2:  waveform = BufferedSynthVoice::Waveform::Square; break;
        case 3:  waveform = BufferedSynthVoice::Waveform::Triangle; break;
        default: waveform = BufferedSynthVoice::Waveform::Sine; break;
    }
    
    if (m_synth)
    {
        m_synth->setWaveform(waveform);
    }
    DBG("Synth waveform set to " << waveformIndex);
}

void CymaSynthAudioProcessor::setBufferedSynthEnvelope(float attack, float decay, float sustain, float release)
{
    if (m_synth)
    {
        m_synth->setEnvelope(attack, decay, sustain, release);
    }
    DBG("Synth envelope: A=" << attack << " D=" << decay 
        << " S=" << sustain << " R=" << release);
}

void CymaSynthAudioProcessor::setBufferedSynthVolume(float volume)
{
    const float clampedVolume = juce::jlimit(0.0f, 1.0f, volume);
    if (m_synth)
    {
        m_synth->setVolume(clampedVolume);
    }
    DBG("Synth volume set to " << clampedVolume);
}

void CymaSynthAudioProcessor::setMasterFaderGain(float gain)
{
    // Set master fader gain (applied to final mix bus)
    m_masterFaderGain.store(gain);
    DBG("Master fader gain set to " << gain);
}

float CymaSynthAudioProcessor::getMasterFaderGain() const
{
    return m_masterFaderGain.load();
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CymaSynthAudioProcessor();
}
