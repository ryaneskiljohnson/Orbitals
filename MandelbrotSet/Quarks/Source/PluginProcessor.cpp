/*
  ==============================================================================

    Quarks - Color Charge Gate/Expander
    Audio FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// Parameter IDs
const juce::String QuarksAudioProcessor::PARAM_THRESHOLD = "threshold";
const juce::String QuarksAudioProcessor::PARAM_RATIO = "ratio";
const juce::String QuarksAudioProcessor::PARAM_ATTACK = "attack";
const juce::String QuarksAudioProcessor::PARAM_RELEASE = "release";
const juce::String QuarksAudioProcessor::PARAM_RANGE = "range";
const juce::String QuarksAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
QuarksAudioProcessor::QuarksAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Quarks"), createParameterLayout())
{
}

QuarksAudioProcessor::~QuarksAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout QuarksAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_THRESHOLD, 1),
        "Strong Force",
        juce::NormalisableRange<float> (-80.0f, 0.0f, 0.1f),
        -40.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + " dB"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_RATIO, 2),
        "Color Charge",
        juce::NormalisableRange<float> (1.0f, 20.0f, 0.1f),
        2.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + ":1"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_ATTACK, 3),
        "Binding",
        juce::NormalisableRange<float> (0.1f, 50.0f, 0.1f),
        5.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + " ms"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_RELEASE, 4),
        "Decay",
        juce::NormalisableRange<float> (10.0f, 500.0f, 1.0f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + " ms"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_RANGE, 5),
        "Confinement",
        juce::NormalisableRange<float> (0.0f, 80.0f, 0.1f),
        60.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + " dB"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 6),
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String QuarksAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool QuarksAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool QuarksAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool QuarksAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double QuarksAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int QuarksAudioProcessor::getNumPrograms()
{
    return 1;
}

int QuarksAudioProcessor::getCurrentProgram()
{
    return 0;
}

void QuarksAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String QuarksAudioProcessor::getProgramName (int index)
{
    return {};
}

void QuarksAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void QuarksAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    phase = 0.0;
}

void QuarksAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool QuarksAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void QuarksAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    
    if (bypass)
    {
        outputLevel.store(inputLevel.load());
        return;
    }

    // TODO: Implement gate/expander DSP algorithm
    // Parameters available:
    // - PARAM_THRESHOLD (strong force)
    // - PARAM_RATIO (color charge)
    // - PARAM_ATTACK (binding)
    // - PARAM_RELEASE (decay)
    // - PARAM_RANGE (confinement)
    
    float threshold = *parameters.getRawParameterValue(PARAM_THRESHOLD);
    float ratio = *parameters.getRawParameterValue(PARAM_RATIO);
    float attack = *parameters.getRawParameterValue(PARAM_ATTACK);
    float release = *parameters.getRawParameterValue(PARAM_RELEASE);
    float range = *parameters.getRawParameterValue(PARAM_RANGE);
    
    // TODO: Implement gate/expander DSP algorithm
    // Process audio buffer with gate/expander effect
    // - threshold: gate threshold in dB (-80 to 0 dB)
    // - ratio: expansion ratio (1:1 to 20:1)
    // - attack: attack time in ms (0.1-50 ms)
    // - release: release time in ms (10-500 ms)
    // - range: maximum gain reduction in dB (0-80 dB)
    
    juce::ignoreUnused(threshold, ratio, attack, release, range);
    
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
bool QuarksAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* QuarksAudioProcessor::createEditor()
{
    return new QuarksAudioProcessorEditor (*this);
}

//==============================================================================
void QuarksAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void QuarksAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new QuarksAudioProcessor();
}
