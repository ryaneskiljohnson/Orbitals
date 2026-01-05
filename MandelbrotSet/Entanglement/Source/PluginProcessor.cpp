/*
  ==============================================================================

    Entanglement - Quantum Delay
    Audio FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String EntanglementAudioProcessor::PARAM_TIME = "time";
const juce::String EntanglementAudioProcessor::PARAM_FEEDBACK = "feedback";
const juce::String EntanglementAudioProcessor::PARAM_MIX = "mix";
const juce::String EntanglementAudioProcessor::PARAM_DAMPING = "damping";
const juce::String EntanglementAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
EntanglementAudioProcessor::EntanglementAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Entanglement"), createParameterLayout())
{
}

EntanglementAudioProcessor::~EntanglementAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout EntanglementAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_TIME, 1),
        "Distance",
        juce::NormalisableRange<float> (10.0f, 2000.0f, 1.0f),
        250.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + " ms"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_FEEDBACK, 2),
        "Coupling",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        30.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_MIX, 3),
        "Coherence",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_DAMPING, 4),
        "Decay",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 5),
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String EntanglementAudioProcessor::getName() const { return JucePlugin_Name; }
bool EntanglementAudioProcessor::acceptsMidi() const { return true; }
bool EntanglementAudioProcessor::producesMidi() const { return true; }
bool EntanglementAudioProcessor::isMidiEffect() const { return true; }
double EntanglementAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int EntanglementAudioProcessor::getNumPrograms() { return 1; }
int EntanglementAudioProcessor::getCurrentProgram() { return 0; }
void EntanglementAudioProcessor::setCurrentProgram (int) {}
const juce::String EntanglementAudioProcessor::getProgramName (int) { return {}; }
void EntanglementAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void EntanglementAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
}

void EntanglementAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EntanglementAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void EntanglementAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Calculate input level for UI - with bounds checking
    float inLevel = 0.0f;
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

    // TODO: Implement delay DSP algorithm
    // Parameters available:
    // - PARAM_TIME (delay time)
    // - PARAM_FEEDBACK (coupling)
    // - PARAM_MIX (coherence)
    // - PARAM_DAMPING (decay)
    
    float time = *parameters.getRawParameterValue(PARAM_TIME);
    float feedback = *parameters.getRawParameterValue(PARAM_FEEDBACK) / 100.0f;
    float mix = *parameters.getRawParameterValue(PARAM_MIX) / 100.0f;
    float damping = *parameters.getRawParameterValue(PARAM_DAMPING) / 100.0f;

    // TODO: Implement delay DSP algorithm
    // Process audio buffer with delay effect
    // - time: delay time in ms
    // - feedback: feedback amount (0-100%)
    // - mix: wet/dry mix (0-100%)
    // - damping: high-frequency damping (0-100%)
    
    juce::ignoreUnused(time, feedback, mix, damping);
    
    // Calculate output level for UI - with bounds checking
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
bool EntanglementAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* EntanglementAudioProcessor::createEditor()
{
    return new EntanglementAudioProcessorEditor (*this);
}

//==============================================================================
void EntanglementAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void EntanglementAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EntanglementAudioProcessor();
}
