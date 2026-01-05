/*
  ==============================================================================

    Mandelbrot - Fractal Phaser
    Audio FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String MandelbrotAudioProcessor::PARAM_RATE = "rate";
const juce::String MandelbrotAudioProcessor::PARAM_DEPTH = "depth";
const juce::String MandelbrotAudioProcessor::PARAM_FEEDBACK = "feedback";
const juce::String MandelbrotAudioProcessor::PARAM_STAGES = "stages";
const juce::String MandelbrotAudioProcessor::PARAM_MIX = "mix";
const juce::String MandelbrotAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
MandelbrotAudioProcessor::MandelbrotAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Mandelbrot"), createParameterLayout())
{
}

MandelbrotAudioProcessor::~MandelbrotAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout MandelbrotAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_RATE, 1),
        "Zoom",
        juce::NormalisableRange<float> (0.1f, 10.0f, 0.1f),
        0.5f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + " Hz"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_DEPTH, 2),
        "Boundary",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_FEEDBACK, 3),
        "Self-Similarity",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        30.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID (PARAM_STAGES, 4),
        "Iterations",
        2, 12, 4
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_MIX, 5),
        "Dimension",
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
const juce::String MandelbrotAudioProcessor::getName() const { return JucePlugin_Name; }
bool MandelbrotAudioProcessor::acceptsMidi() const { return true; }
bool MandelbrotAudioProcessor::producesMidi() const { return true; }
bool MandelbrotAudioProcessor::isMidiEffect() const { return true; }
double MandelbrotAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int MandelbrotAudioProcessor::getNumPrograms() { return 1; }
int MandelbrotAudioProcessor::getCurrentProgram() { return 0; }
void MandelbrotAudioProcessor::setCurrentProgram (int) {}
const juce::String MandelbrotAudioProcessor::getProgramName (int) { return {}; }
void MandelbrotAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void MandelbrotAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    noteBuffer.clear();
}

void MandelbrotAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MandelbrotAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MandelbrotAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    float rate = *parameters.getRawParameterValue(PARAM_RATE);
    float depth = *parameters.getRawParameterValue(PARAM_DEPTH) / 100.0f;
    float feedback = *parameters.getRawParameterValue(PARAM_FEEDBACK) / 100.0f;
    int stages = (int)*parameters.getRawParameterValue(PARAM_STAGES);
    float mix = *parameters.getRawParameterValue(PARAM_MIX) / 100.0f;

    // TODO: Implement phaser DSP algorithm
    // Process audio buffer with phaser effect
    // - rate: LFO rate in Hz
    // - depth: modulation depth (0-100%)
    // - feedback: feedback amount (0-100%)
    // - stages: number of allpass stages (2-12)
    // - mix: wet/dry mix (0-100%)
    
    juce::ignoreUnused(rate, depth, feedback, stages, mix);
    
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
bool MandelbrotAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* MandelbrotAudioProcessor::createEditor()
{
    return new MandelbrotAudioProcessorEditor (*this);
}

//==============================================================================
void MandelbrotAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MandelbrotAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MandelbrotAudioProcessor();
}
