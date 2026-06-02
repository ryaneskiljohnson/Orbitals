/*
  ==============================================================================

    Planck - Quantum Scale EQ
    Audio FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String PlanckAudioProcessor::PARAM_LOWFREQ = "lowfreq";
const juce::String PlanckAudioProcessor::PARAM_LOWGAIN = "lowgain";
const juce::String PlanckAudioProcessor::PARAM_MIDFREQ = "midfreq";
const juce::String PlanckAudioProcessor::PARAM_MIDGAIN = "midgain";
const juce::String PlanckAudioProcessor::PARAM_HIGHFREQ = "highfreq";
const juce::String PlanckAudioProcessor::PARAM_HIGHGAIN = "highgain";
const juce::String PlanckAudioProcessor::PARAM_Q = "q";
const juce::String PlanckAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
PlanckAudioProcessor::PlanckAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Planck"), createParameterLayout())
{
}

PlanckAudioProcessor::~PlanckAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PlanckAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_LOWFREQ, 1),
        "Low Scale",
        juce::NormalisableRange<float> (20.0f, 500.0f, 1.0f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 0) + " Hz"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_LOWGAIN, 2),
        "Low Energy",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return (value >= 0 ? "+" : "") + juce::String (value, 1) + " dB"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_MIDFREQ, 3),
        "Mid Scale",
        juce::NormalisableRange<float> (200.0f, 5000.0f, 1.0f),
        1000.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value / 1000.0f, 1) + "k Hz"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_MIDGAIN, 4),
        "Mid Energy",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return (value >= 0 ? "+" : "") + juce::String (value, 1) + " dB"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_HIGHFREQ, 5),
        "High Scale",
        juce::NormalisableRange<float> (2000.0f, 20000.0f, 1.0f),
        10000.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value / 1000.0f, 1) + "k Hz"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_HIGHGAIN, 6),
        "High Energy",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return (value >= 0 ? "+" : "") + juce::String (value, 1) + " dB"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_Q, 7),
        "Resonance",
        juce::NormalisableRange<float> (0.1f, 10.0f, 0.1f),
        1.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1); }
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 8),
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String PlanckAudioProcessor::getName() const { return JucePlugin_Name; }

bool PlanckAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PlanckAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PlanckAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
double PlanckAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int PlanckAudioProcessor::getNumPrograms() { return 1; }
int PlanckAudioProcessor::getCurrentProgram() { return 0; }
void PlanckAudioProcessor::setCurrentProgram (int) {}
const juce::String PlanckAudioProcessor::getProgramName (int) { return {}; }
void PlanckAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void PlanckAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    
    // Prepare 3-band EQ filters
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;
    
    lowCoeffsL = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 200.0f, qValue, 1.0f);
    lowCoeffsR = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 200.0f, qValue, 1.0f);
    midCoeffsL = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, qValue, 1.0f);
    midCoeffsR = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, qValue, 1.0f);
    highCoeffsL = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 5000.0f, qValue, 1.0f);
    highCoeffsR = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 5000.0f, qValue, 1.0f);
    
    lowFilterL.prepare(spec);
    lowFilterR.prepare(spec);
    midFilterL.prepare(spec);
    midFilterR.prepare(spec);
    highFilterL.prepare(spec);
    highFilterR.prepare(spec);
    
    lowFilterL.coefficients = lowCoeffsL;
    lowFilterR.coefficients = lowCoeffsR;
    midFilterL.coefficients = midCoeffsL;
    midFilterR.coefficients = midCoeffsR;
    highFilterL.coefficients = highCoeffsL;
    highFilterR.coefficients = highCoeffsR;
}

void PlanckAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PlanckAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void PlanckAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    float lowFreq = *parameters.getRawParameterValue(PARAM_LOWFREQ);
    float lowGain = *parameters.getRawParameterValue(PARAM_LOWGAIN);
    float midFreq = *parameters.getRawParameterValue(PARAM_MIDFREQ);
    float midGain = *parameters.getRawParameterValue(PARAM_MIDGAIN);
    float highFreq = *parameters.getRawParameterValue(PARAM_HIGHFREQ);
    float highGain = *parameters.getRawParameterValue(PARAM_HIGHGAIN);
    float q = *parameters.getRawParameterValue(PARAM_Q);

    // Convert gains from dB to linear
    float lowGainLinear = juce::Decibels::decibelsToGain(lowGain);
    float midGainLinear = juce::Decibels::decibelsToGain(midGain);
    float highGainLinear = juce::Decibels::decibelsToGain(highGain);
    
    // Clamp frequencies
    lowFreq = juce::jlimit(20.0f, 500.0f, lowFreq);
    midFreq = juce::jlimit(200.0f, 5000.0f, midFreq);
    highFreq = juce::jlimit(2000.0f, 20000.0f, highFreq);
    q = juce::jlimit(0.1f, 10.0f, q);
    
    // Update filter coefficients
    lowCoeffsL = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, lowFreq, q, lowGainLinear);
    lowCoeffsR = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, lowFreq, q, lowGainLinear);
    midCoeffsL = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, midFreq, q, midGainLinear);
    midCoeffsR = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, midFreq, q, midGainLinear);
    highCoeffsL = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, highFreq, q, highGainLinear);
    highCoeffsR = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, highFreq, q, highGainLinear);
    
    lowFilterL.coefficients = lowCoeffsL;
    lowFilterR.coefficients = lowCoeffsR;
    midFilterL.coefficients = midCoeffsL;
    midFilterR.coefficients = midCoeffsR;
    highFilterL.coefficients = highCoeffsL;
    highFilterR.coefficients = highCoeffsR;
    
    // Process each channel
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto& lowFilter = (channel == 0) ? lowFilterL : lowFilterR;
        auto& midFilter = (channel == 0) ? midFilterL : midFilterR;
        auto& highFilter = (channel == 0) ? highFilterL : highFilterR;
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float input = channelData[sample];
            
            // Process through 3-band EQ
            float output = lowFilter.processSample(input);
            output = midFilter.processSample(output);
            output = highFilter.processSample(output);
            
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
}

//==============================================================================
bool PlanckAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* PlanckAudioProcessor::createEditor()
{
    return new PlanckAudioProcessorEditor (*this);
}

//==============================================================================
void PlanckAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PlanckAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PlanckAudioProcessor();
}
