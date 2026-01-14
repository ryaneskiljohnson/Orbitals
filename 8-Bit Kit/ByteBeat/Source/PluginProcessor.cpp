/*
  ==============================================================================

    ByteBeat - Quantum Descent Flanger
    Audio FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String ByteBeatAudioProcessor::PARAM_RATE = "rate";
const juce::String ByteBeatAudioProcessor::PARAM_DEPTH = "depth";
const juce::String ByteBeatAudioProcessor::PARAM_FEEDBACK = "feedback";
const juce::String ByteBeatAudioProcessor::PARAM_STEREO = "stereo";
const juce::String ByteBeatAudioProcessor::PARAM_MIX = "mix";
const juce::String ByteBeatAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
ByteBeatAudioProcessor::ByteBeatAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("ByteBeat"), createParameterLayout())
{
}

ByteBeatAudioProcessor::~ByteBeatAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ByteBeatAudioProcessor::createParameterLayout()
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
const juce::String ByteBeatAudioProcessor::getName() const { return JucePlugin_Name; }
bool ByteBeatAudioProcessor::acceptsMidi() const { return true; }
bool ByteBeatAudioProcessor::producesMidi() const { return true; }
bool ByteBeatAudioProcessor::isMidiEffect() const { return true; }
double ByteBeatAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int ByteBeatAudioProcessor::getNumPrograms() { return 1; }
int ByteBeatAudioProcessor::getCurrentProgram() { return 0; }
void ByteBeatAudioProcessor::setCurrentProgram (int) {}
const juce::String ByteBeatAudioProcessor::getProgramName (int) { return {}; }
void ByteBeatAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void ByteBeatAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    
    // Prepare delay lines for flanger (max 20ms)
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;
    
    delayLineL.prepare(spec);
    delayLineR.prepare(spec);
    delayLineL.setMaximumDelayInSamples(static_cast<int>(sampleRate * maxDelayMs / 1000.0));
    delayLineR.setMaximumDelayInSamples(static_cast<int>(sampleRate * maxDelayMs / 1000.0));
    
    lfoPhaseL = 0.0;
    lfoPhaseR = 0.0;
}

void ByteBeatAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ByteBeatAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ByteBeatAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    // Get parameters
    float rateHz = *parameters.getRawParameterValue(PARAM_RATE);
    float depth = *parameters.getRawParameterValue(PARAM_DEPTH) / 100.0f;
    float feedback = *parameters.getRawParameterValue(PARAM_FEEDBACK) / 100.0f;
    float stereoWidth = *parameters.getRawParameterValue(PARAM_STEREO) / 100.0f;
    float mix = *parameters.getRawParameterValue(PARAM_MIX) / 100.0f;
    
    // Calculate LFO increment
    double lfoIncrement = (rateHz * 2.0 * juce::MathConstants<double>::pi) / sampleRate;
    
    // Base delay time (1-5ms typical for flanger)
    float baseDelayMs = 1.0f + depth * 4.0f;
    float baseDelaySamples = (baseDelayMs / 1000.0f) * static_cast<float>(sampleRate);
    
    // Process each channel
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto& delayLine = (channel == 0) ? delayLineL : delayLineR;
        double& lfoPhase = (channel == 0) ? lfoPhaseL : lfoPhaseR;
        
        // Stereo offset for width
        double phaseOffset = (channel == 1) ? (stereoWidth * juce::MathConstants<double>::pi) : 0.0;
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float input = channelData[sample];
            
            // Calculate LFO value (sine wave)
            double lfoValue = std::sin(lfoPhase + phaseOffset);
            lfoPhase += lfoIncrement;
            if (lfoPhase > 2.0 * juce::MathConstants<double>::pi)
                lfoPhase -= 2.0 * juce::MathConstants<double>::pi;
            
            // Calculate modulated delay time
            float delayModulation = (lfoValue * 0.5f + 0.5f) * depth; // 0 to depth
            float delayTimeSamples = baseDelaySamples + (delayModulation * baseDelaySamples);
            delayTimeSamples = juce::jlimit(0.0f, static_cast<float>(delayLine.getMaximumDelayInSamples() - 1), delayTimeSamples);
            delayLine.setDelay(delayTimeSamples);
            
            // Read delayed signal
            float delayed = delayLine.popSample(0);
            
            // Mix input with delayed signal
            float output = input * (1.0f - mix) + delayed * mix;
            
            // Write to delay line with feedback
            delayLine.pushSample(0, input + delayed * feedback);
            
            channelData[sample] = output;
        }
    }
    
    // Calculate output level for UI
    float outLevel = 0.0f;
    if (numSamples > 0 && numChannels > 0) {
        for (int channel = 0; channel < numChannels; ++channel) {
            float channelLevel = buffer.getRMSLevel(channel, 0, numSamples);
            outLevel = std::max(outLevel, channelLevel);
        }
    }
    outputLevel.store(juce::Decibels::gainToDecibels(outLevel, -100.0f));
    
    // For now, pass audio through unchanged
    // MIDI processing removed - this is an Audio FX plugin
}

//==============================================================================
bool ByteBeatAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ByteBeatAudioProcessor::createEditor()
{
    return new ByteBeatAudioProcessorEditor (*this);
}

//==============================================================================
void ByteBeatAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ByteBeatAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ByteBeatAudioProcessor();
}
