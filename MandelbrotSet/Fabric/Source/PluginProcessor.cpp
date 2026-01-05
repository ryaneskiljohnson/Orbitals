/*
  ==============================================================================

    Fabric - Spacetime Reverb
    Audio FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// Parameter IDs
const juce::String FabricAudioProcessor::PARAM_SIZE = "size";
const juce::String FabricAudioProcessor::PARAM_DIFFUSION = "diffusion";
const juce::String FabricAudioProcessor::PARAM_DAMPING = "damping";
const juce::String FabricAudioProcessor::PARAM_PREDELAY = "predelay";
const juce::String FabricAudioProcessor::PARAM_MIX = "mix";
const juce::String FabricAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
FabricAudioProcessor::FabricAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Fabric"), createParameterLayout())
{
    smoothedVelocities.fill(64.0f);
}

FabricAudioProcessor::~FabricAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout FabricAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_SIZE, 1),
        "Weave",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_DIFFUSION, 2),
        "Thread Density",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_DAMPING, 3),
        "Fold",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_PREDELAY, 4),
        "Time Dilation",
        juce::NormalisableRange<float> (0.0f, 200.0f, 1.0f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + " ms"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_MIX, 5),
        "Warp",
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
const juce::String FabricAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FabricAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FabricAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FabricAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FabricAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FabricAudioProcessor::getNumPrograms()
{
    return 1;
}

int FabricAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FabricAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FabricAudioProcessor::getProgramName (int index)
{
    return {};
}

void FabricAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FabricAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    smoothedVelocities.fill(64.0f);
}

void FabricAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FabricAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

float FabricAudioProcessor::applyCurve(float normalizedInput, int curveType)
{
    // Clamp input to 0-1
    normalizedInput = juce::jlimit(0.0f, 1.0f, normalizedInput);
    
    switch (curveType)
    {
        case 0: // LINEAR
            return normalizedInput;
            
        case 1: // EXP (Exponential)
            return std::pow(normalizedInput, 2.0f);
            
        case 2: // S-CURVE
            return 0.5f - std::cos(normalizedInput * juce::MathConstants<float>::pi) / 2.0f;
            
        case 3: // LOG (Logarithmic)
            return std::log(1.0f + normalizedInput * 9.0f) / std::log(10.0f);
            
        default:
            return normalizedInput;
    }
}

void FabricAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    // Get parameters
    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    
    if (bypass)
    {
        outputLevel.store(inputLevel.load());
        return;
    }

    // TODO: Implement reverb DSP algorithm
    // Parameters available:
    // - PARAM_SIZE (weave)
    // - PARAM_DIFFUSION (thread density)
    // - PARAM_DAMPING (fold)
    // - PARAM_PREDELAY (time dilation)
    // - PARAM_MIX (warp)
    
    float size = *parameters.getRawParameterValue(PARAM_SIZE) / 100.0f;
    float diffusion = *parameters.getRawParameterValue(PARAM_DIFFUSION) / 100.0f;
    float damping = *parameters.getRawParameterValue(PARAM_DAMPING) / 100.0f;
    float predelay = *parameters.getRawParameterValue(PARAM_PREDELAY);
    float mix = *parameters.getRawParameterValue(PARAM_MIX) / 100.0f;
    
    // TODO: Implement reverb DSP algorithm
    // Process audio buffer with reverb effect
    // - size: room size (0-100%)
    // - diffusion: diffusion amount (0-100%)
    // - damping: high-frequency damping (0-100%)
    // - predelay: pre-delay in ms (0-200 ms)
    // - mix: wet/dry mix (0-100%)
    
    juce::ignoreUnused(size, diffusion, damping, predelay, mix);
    
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
bool FabricAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* FabricAudioProcessor::createEditor()
{
    return new FabricAudioProcessorEditor (*this);
}

//==============================================================================
void FabricAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void FabricAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FabricAudioProcessor();
}
