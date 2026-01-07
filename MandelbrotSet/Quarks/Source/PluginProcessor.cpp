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
    return 0.0; // Gate has no tail
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
    // Accept mono or stereo input
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // Output must be stereo
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

    // Get parameters
    float thresholdDb = *parameters.getRawParameterValue(PARAM_THRESHOLD);
    float ratio = *parameters.getRawParameterValue(PARAM_RATIO);
    float attackMs = *parameters.getRawParameterValue(PARAM_ATTACK);
    float releaseMs = *parameters.getRawParameterValue(PARAM_RELEASE);
    float rangeDb = *parameters.getRawParameterValue(PARAM_RANGE);
    
    // Convert threshold to linear
    float thresholdLinear = juce::Decibels::decibelsToGain(thresholdDb);
    
    // Calculate attack and release coefficients
    float attackCoeff = std::exp(-1.0f / (attackMs * 0.001f * sampleRate));
    float releaseCoeff = std::exp(-1.0f / (releaseMs * 0.001f * sampleRate));
    
    // Process each channel
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float input = channelData[sample];
            
            // Calculate input level
            float inputLevel = std::abs(input);
            
            // Envelope follower (attack/release)
            if (inputLevel > envelope)
                envelope = inputLevel + (envelope - inputLevel) * attackCoeff;
            else
                envelope = inputLevel + (envelope - inputLevel) * releaseCoeff;
            
            // Calculate gain reduction (gate/expander logic)
            if (envelope < thresholdLinear)
            {
                // Signal is below threshold - apply expansion
                float belowThresholdDb = juce::Decibels::gainToDecibels(envelope / thresholdLinear);
                float expandedDb = belowThresholdDb * ratio;
                gainReduction = belowThresholdDb - expandedDb;
                
                // Limit gain reduction to range
                gainReduction = juce::jlimit(-rangeDb, 0.0f, gainReduction);
            }
            else
            {
                // Signal is above threshold - no reduction
                gainReduction = 0.0f;
            }
            
            // Apply gain reduction
            float gainReductionLinear = juce::Decibels::decibelsToGain(-gainReduction);
            float output = input * gainReductionLinear;
            
            // Soft clip
            output = juce::jlimit(-1.0f, 1.0f, output);
            
            channelData[sample] = output;
        }
    }
    
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
