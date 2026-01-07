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
bool HadronAudioProcessor::acceptsMidi() const { return false; }
bool HadronAudioProcessor::producesMidi() const { return false; }
bool HadronAudioProcessor::isMidiEffect() const { return false; }
double HadronAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int HadronAudioProcessor::getNumPrograms() { return 1; }
int HadronAudioProcessor::getCurrentProgram() { return 0; }
void HadronAudioProcessor::setCurrentProgram (int) {}
const juce::String HadronAudioProcessor::getProgramName (int) { return {}; }
void HadronAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void HadronAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    
    // Prepare tone filters
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;
    
    toneCoeffsL = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 10000.0f);
    toneCoeffsR = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 10000.0f);
    toneFilterL.prepare(spec);
    toneFilterR.prepare(spec);
    toneFilterL.coefficients = toneCoeffsL;
    toneFilterR.coefficients = toneCoeffsR;
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

    // Get parameters
    float drive = *parameters.getRawParameterValue(PARAM_DRIVE) / 100.0f; // 0-1
    float toneAmount = *parameters.getRawParameterValue(PARAM_TONE) / 100.0f; // 0-1
    float bias = *parameters.getRawParameterValue(PARAM_BIAS) / 100.0f; // -1 to 1
    float mix = *parameters.getRawParameterValue(PARAM_MIX) / 100.0f; // 0-1
    float outputGain = *parameters.getRawParameterValue(PARAM_OUTPUT); // dB
    
    // Convert output gain from dB to linear
    float outputGainLinear = juce::Decibels::decibelsToGain(outputGain);
    
    // Update tone filter (toneAmount: 0% = bright, 100% = dark)
    float toneCutoff = 2000.0f + (toneAmount * 18000.0f); // 2kHz to 20kHz
    toneCoeffsL = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, toneCutoff);
    toneCoeffsR = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, toneCutoff);
    toneFilterL.coefficients = toneCoeffsL;
    toneFilterR.coefficients = toneCoeffsR;
    
    // Process each channel
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto& toneFilter = (channel == 0) ? toneFilterL : toneFilterR;
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float input = channelData[sample];
            float dry = input;
            
            // Apply DC bias for asymmetric distortion
            float biased = input + bias * 0.1f;
            
            // Apply drive (gain before saturation)
            float driven = biased * (1.0f + drive * 9.0f); // 1x to 10x gain
            
            // Soft saturation using tanh
            float saturated = std::tanh(driven);
            
            // Apply tone filter
            saturated = toneFilter.processSample(saturated);
            
            // Mix wet/dry
            float output = dry * (1.0f - mix) + saturated * mix;
            
            // Apply output gain
            output *= outputGainLinear;
            
            // Soft clip to prevent overs
            output = juce::jlimit(-1.0f, 1.0f, output);
            
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
