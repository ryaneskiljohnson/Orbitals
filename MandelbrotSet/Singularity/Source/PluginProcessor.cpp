/*
  ==============================================================================

    Singularity - Black Hole Compressor
    Audio FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// Parameter IDs
const juce::String SingularityAudioProcessor::PARAM_THRESHOLD = "threshold";
const juce::String SingularityAudioProcessor::PARAM_RATIO = "ratio";
const juce::String SingularityAudioProcessor::PARAM_ATTACK = "attack";
const juce::String SingularityAudioProcessor::PARAM_RELEASE = "release";
const juce::String SingularityAudioProcessor::PARAM_MAKEUP = "makeup";
const juce::String SingularityAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
SingularityAudioProcessor::SingularityAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Singularity"), createParameterLayout())
{
}

SingularityAudioProcessor::~SingularityAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout SingularityAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Threshold: -60 to 0 dB
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_THRESHOLD, 1),
        "Event Horizon",
        juce::NormalisableRange<float> (-60.0f, 0.0f, 0.1f),
        -20.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + " dB"; }
    ));

    // Ratio: 1:1 to 20:1
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_RATIO, 2),
        "Gravity",
        juce::NormalisableRange<float> (1.0f, 20.0f, 0.1f),
        4.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + ":1"; }
    ));

    // Attack: 0.1 to 100 ms
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_ATTACK, 3),
        "Collapse",
        juce::NormalisableRange<float> (0.1f, 100.0f, 0.1f),
        10.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + " ms"; }
    ));

    // Release: 10 to 1000 ms
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_RELEASE, 4),
        "Hawking Radiation",
        juce::NormalisableRange<float> (10.0f, 1000.0f, 1.0f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + " ms"; }
    ));

    // Makeup Gain: -12 to +24 dB
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_MAKEUP, 5),
        "Mass",
        juce::NormalisableRange<float> (-12.0f, 24.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { 
            juce::String sign = value >= 0 ? "+" : "";
            return sign + juce::String (value, 1) + " dB"; 
        }
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 6),
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String SingularityAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SingularityAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SingularityAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SingularityAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SingularityAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SingularityAudioProcessor::getNumPrograms()
{
    return 1;
}

int SingularityAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SingularityAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SingularityAudioProcessor::getProgramName (int index)
{
    return {};
}

void SingularityAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SingularityAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
}

void SingularityAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SingularityAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Standard stereo only
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
  #endif
}
#endif

void SingularityAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Calculate input level for UI (RMS) - with bounds checking
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
        currentGainReduction.store(0.0f);
        return;
    }

    // TODO: Implement compressor DSP algorithm
    // Parameters available:
    // - PARAM_THRESHOLD (event horizon)
    // - PARAM_RATIO (gravity)
    // - PARAM_ATTACK (collapse)
    // - PARAM_RELEASE (hawking radiation)
    // - PARAM_MAKEUP (mass)
    
    float threshold = *parameters.getRawParameterValue(PARAM_THRESHOLD);
    float ratio = *parameters.getRawParameterValue(PARAM_RATIO);
    float attack = *parameters.getRawParameterValue(PARAM_ATTACK);
    float release = *parameters.getRawParameterValue(PARAM_RELEASE);
    float makeup = *parameters.getRawParameterValue(PARAM_MAKEUP);

    // TODO: Implement compressor DSP algorithm
    // Process audio buffer with compression
    // - threshold: compression threshold in dB (-60 to 0 dB)
    // - ratio: compression ratio (1:1 to 20:1)
    // - attack: attack time in ms (0.1-100 ms)
    // - release: release time in ms (10-1000 ms)
    // - makeup: makeup gain in dB (-12 to +24 dB)
    
    juce::ignoreUnused(threshold, ratio, attack, release, makeup);
    
    // Calculate output level for UI - with bounds checking
    float outLevel = 0.0f;
    if (numSamples > 0 && numChannels > 0) {
        for (int channel = 0; channel < totalNumInputChannels && channel < numChannels; ++channel) {
            float channelLevel = buffer.getRMSLevel(channel, 0, numSamples);
            outLevel = std::max(outLevel, channelLevel);
        }
    }
    outputLevel.store(juce::Decibels::gainToDecibels(outLevel, -100.0f));
    
    // Store gain reduction (placeholder - will be calculated by actual DSP)
    currentGainReduction.store(0.0f);
    
    // For now, pass audio through unchanged
    // MIDI processing removed - this is an Audio FX plugin
}

//==============================================================================
bool SingularityAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SingularityAudioProcessor::createEditor()
{
    return new SingularityAudioProcessorEditor (*this);
}

//==============================================================================
void SingularityAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SingularityAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SingularityAudioProcessor();
}
