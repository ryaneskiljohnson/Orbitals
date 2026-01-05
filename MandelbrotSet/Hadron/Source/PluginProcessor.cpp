/*
  ==============================================================================

    Hadron - Particle Collision Saturation
    Audio FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String HadronAudioProcessor::PARAM_DRIVE = "drive";
const juce::String HadronAudioProcessor::PARAM_TONE = "tone";
const juce::String HadronAudioProcessor::PARAM_BIAS = "bias";
const juce::String HadronAudioProcessor::PARAM_MIX = "mix";
const juce::String HadronAudioProcessor::PARAM_OUTPUT = "output";
const juce::String HadronAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
HadronAudioProcessor::HadronAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Hadron"), createParameterLayout())
{
}

HadronAudioProcessor::~HadronAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout HadronAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_DRIVE, 1),
        "Collision Energy",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_TONE, 2),
        "Particle Mass",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_BIAS, 3),
        "Acceleration",
        juce::NormalisableRange<float> (-100.0f, 100.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return (value >= 0 ? "+" : "") + juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_MIX, 4),
        "Luminosity",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_OUTPUT, 5),
        "Beam Focus",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return (value >= 0 ? "+" : "") + juce::String (value, 1) + " dB"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 6),
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String HadronAudioProcessor::getName() const { return JucePlugin_Name; }
bool HadronAudioProcessor::acceptsMidi() const { return true; }
bool HadronAudioProcessor::producesMidi() const { return true; }
bool HadronAudioProcessor::isMidiEffect() const { return true; }
double HadronAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int HadronAudioProcessor::getNumPrograms() { return 1; }
int HadronAudioProcessor::getCurrentProgram() { return 0; }
void HadronAudioProcessor::setCurrentProgram (int) {}
const juce::String HadronAudioProcessor::getProgramName (int) { return {}; }
void HadronAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void HadronAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void HadronAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HadronAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void HadronAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();

    // Calculate input level for UI - with bounds checking
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

    // TODO: Implement saturation DSP algorithm
    // Parameters available:
    // - PARAM_DRIVE (collision energy)
    // - PARAM_TONE (particle mass)
    // - PARAM_BIAS (acceleration)
    // - PARAM_MIX (luminosity)
    // - PARAM_OUTPUT (beam focus)
    
    float drive = *parameters.getRawParameterValue(PARAM_DRIVE) / 100.0f;
    float tone = *parameters.getRawParameterValue(PARAM_TONE) / 100.0f;
    float bias = *parameters.getRawParameterValue(PARAM_BIAS) / 100.0f;
    float mix = *parameters.getRawParameterValue(PARAM_MIX) / 100.0f;
    float output = *parameters.getRawParameterValue(PARAM_OUTPUT);

    // TODO: Implement saturation DSP algorithm
    // Process audio buffer with saturation/distortion
    // - drive: amount of saturation (0-100%)
    // - tone: tone control (0-100%)
    // - bias: DC bias for asymmetric distortion (-100% to +100%)
    // - mix: wet/dry mix (0-100%)
    // - output: output gain in dB (-12 to +12 dB)
    
    juce::ignoreUnused(drive, tone, bias, mix, output);
    
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
bool HadronAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* HadronAudioProcessor::createEditor()
{
    return new HadronAudioProcessorEditor (*this);
}

//==============================================================================
void HadronAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void HadronAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HadronAudioProcessor();
}
