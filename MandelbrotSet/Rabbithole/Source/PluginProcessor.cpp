/*
  ==============================================================================

    Rabbithole - Quantum Descent Flanger
    Audio FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String RabbitholeAudioProcessor::PARAM_RATE = "rate";
const juce::String RabbitholeAudioProcessor::PARAM_DEPTH = "depth";
const juce::String RabbitholeAudioProcessor::PARAM_FEEDBACK = "feedback";
const juce::String RabbitholeAudioProcessor::PARAM_STEREO = "stereo";
const juce::String RabbitholeAudioProcessor::PARAM_MIX = "mix";
const juce::String RabbitholeAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
RabbitholeAudioProcessor::RabbitholeAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Rabbithole"), createParameterLayout())
{
}

RabbitholeAudioProcessor::~RabbitholeAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout RabbitholeAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_RATE, 1),
        "Descent Rate",
        juce::NormalisableRange<float> (0.1f, 10.0f, 0.1f),
        0.5f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + " Hz"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_DEPTH, 2),
        "Tunnel Depth",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_FEEDBACK, 3),
        "Gravity Well",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        30.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_STEREO, 4),
        "Duality",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_MIX, 5),
        "Wonderland",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 6),
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String RabbitholeAudioProcessor::getName() const { return JucePlugin_Name; }
bool RabbitholeAudioProcessor::acceptsMidi() const { return true; }
bool RabbitholeAudioProcessor::producesMidi() const { return true; }
bool RabbitholeAudioProcessor::isMidiEffect() const { return true; }
double RabbitholeAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int RabbitholeAudioProcessor::getNumPrograms() { return 1; }
int RabbitholeAudioProcessor::getCurrentProgram() { return 0; }
void RabbitholeAudioProcessor::setCurrentProgram (int) {}
const juce::String RabbitholeAudioProcessor::getProgramName (int) { return {}; }
void RabbitholeAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void RabbitholeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
}

void RabbitholeAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RabbitholeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void RabbitholeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();

    // Calculate input level for UI (with bounds checking)
    float inLevel = 0.0f;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    if (numSamples > 0 && numChannels > 0) {
        for (int channel = 0; channel < totalNumInputChannels && channel < numChannels; ++channel) {
            float channelLevel = buffer.getRMSLevel(channel, 0, numSamples);
            inLevel = std::max(inLevel, channelLevel);
        }
    }
    inputLevel.store(juce::Decibels::gainToDecibels(inLevel, -100.0f));

    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    if (bypass) {
        outputLevel.store(inputLevel.load());
        return;
    }

    // TODO: Implement flanger DSP algorithm
    // Parameters available:
    // - PARAM_RATE (descent rate)
    // - PARAM_DEPTH (tunnel depth)
    // - PARAM_FEEDBACK (gravity well)
    // - PARAM_STEREO (duality)
    // - PARAM_MIX (wonderland)
    
    float rate = *parameters.getRawParameterValue(PARAM_RATE);
    float depth = *parameters.getRawParameterValue(PARAM_DEPTH) / 100.0f;
    float feedback = *parameters.getRawParameterValue(PARAM_FEEDBACK) / 100.0f;
    float stereo = *parameters.getRawParameterValue(PARAM_STEREO) / 100.0f;
    float mix = *parameters.getRawParameterValue(PARAM_MIX) / 100.0f;

    // TODO: Implement flanger DSP algorithm
    // Process audio buffer with flanger effect
    // - rate: LFO rate in Hz (0.1-10 Hz)
    // - depth: modulation depth (0-100%)
    // - feedback: feedback amount (0-100%)
    // - stereo: stereo width (0-100%)
    // - mix: wet/dry mix (0-100%)
    
    juce::ignoreUnused(rate, depth, feedback, stereo, mix);
    
    // Calculate output level for UI (with bounds checking)
    float outLevel = 0.0f;
    if (numSamples > 0 && numChannels > 0) {
        for (int channel = 0; channel < totalNumInputChannels && channel < numChannels; ++channel) {
            float channelLevel = buffer.getRMSLevel(channel, 0, numSamples);
            outLevel = std::max(outLevel, channelLevel);
        }
    }
    outputLevel.store(juce::Decibels::gainToDecibels(outLevel, -100.0f));
    
    // For now, pass audio through unchanged
    // MIDI processing removed - this is an Audio FX plugin
}

//==============================================================================
bool RabbitholeAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* RabbitholeAudioProcessor::createEditor()
{
    return new RabbitholeAudioProcessorEditor (*this);
}

//==============================================================================
void RabbitholeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void RabbitholeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RabbitholeAudioProcessor();
}
