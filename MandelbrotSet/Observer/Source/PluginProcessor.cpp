/*
  ==============================================================================

    Observer - Wavefunction Collapse Dynamic EQ
    Audio FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String ObserverAudioProcessor::PARAM_THRESHOLD = "threshold";
const juce::String ObserverAudioProcessor::PARAM_FREQUENCY = "frequency";
const juce::String ObserverAudioProcessor::PARAM_RATIO = "ratio";
const juce::String ObserverAudioProcessor::PARAM_ATTACK = "attack";
const juce::String ObserverAudioProcessor::PARAM_RELEASE = "release";
const juce::String ObserverAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
ObserverAudioProcessor::ObserverAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Observer"), createParameterLayout())
{
}

ObserverAudioProcessor::~ObserverAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ObserverAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_THRESHOLD, 1),
        "Measurement",
        juce::NormalisableRange<float> (-60.0f, 0.0f, 0.1f),
        -20.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + " dB"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_FREQUENCY, 2),
        "Precision",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f),
        1000.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return value < 1000.0f ? juce::String (value, 0) + " Hz" : juce::String (value / 1000.0f, 1) + "k Hz"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_RATIO, 3),
        "Collapse Ratio",
        juce::NormalisableRange<float> (1.0f, 10.0f, 0.1f),
        2.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + ":1"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_ATTACK, 4),
        "Observation Time",
        juce::NormalisableRange<float> (0.1f, 100.0f, 0.1f),
        10.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + " ms"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_RELEASE, 5),
        "Uncertainty",
        juce::NormalisableRange<float> (10.0f, 1000.0f, 1.0f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + " ms"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 6),
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String ObserverAudioProcessor::getName() const { return JucePlugin_Name; }
bool ObserverAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ObserverAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ObserverAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
double ObserverAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int ObserverAudioProcessor::getNumPrograms() { return 1; }
int ObserverAudioProcessor::getCurrentProgram() { return 0; }
void ObserverAudioProcessor::setCurrentProgram (int) {}
const juce::String ObserverAudioProcessor::getProgramName (int) { return {}; }
void ObserverAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void ObserverAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    
    // Prepare band filters
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;
    
    bandCoeffsL = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, qValue, 1.0f);
    bandCoeffsR = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, qValue, 1.0f);
    bandFilterL.prepare(spec);
    bandFilterR.prepare(spec);
    bandFilterL.coefficients = bandCoeffsL;
    bandFilterR.coefficients = bandCoeffsR;
    
    envelope = 0.0f;
    currentGain = 1.0f;
}

void ObserverAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ObserverAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ObserverAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    float thresholdDb = *parameters.getRawParameterValue(PARAM_THRESHOLD);
    float centerFreq = *parameters.getRawParameterValue(PARAM_FREQUENCY);
    float ratio = *parameters.getRawParameterValue(PARAM_RATIO);
    float attackMs = *parameters.getRawParameterValue(PARAM_ATTACK);
    float releaseMs = *parameters.getRawParameterValue(PARAM_RELEASE);
    
    // Convert threshold to linear
    float thresholdLinear = juce::Decibels::decibelsToGain(thresholdDb);
    
    // Calculate attack and release coefficients
    float attackCoeff = std::exp(-1.0f / (attackMs * 0.001f * sampleRate));
    float releaseCoeff = std::exp(-1.0f / (releaseMs * 0.001f * sampleRate));
    
    // Update band filter
    centerFreq = juce::jlimit(20.0f, 20000.0f, centerFreq);
    bandCoeffsL = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, centerFreq, qValue, 1.0f);
    bandCoeffsR = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, centerFreq, qValue, 1.0f);
    bandFilterL.coefficients = bandCoeffsL;
    bandFilterR.coefficients = bandCoeffsR;
    
    // Process each channel
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto& bandFilter = (channel == 0) ? bandFilterL : bandFilterR;
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float input = channelData[sample];
            
            // Extract band energy using band-pass filter
            float bandEnergy = std::abs(bandFilter.processSample(input));
            
            // Envelope follower
            if (bandEnergy > envelope)
                envelope = bandEnergy + (envelope - bandEnergy) * attackCoeff;
            else
                envelope = bandEnergy + (envelope - bandEnergy) * releaseCoeff;
            
            // Calculate dynamic gain based on threshold
            float targetGain = 1.0f;
            if (envelope > thresholdLinear)
            {
                // Above threshold - apply compression
                float overThresholdDb = juce::Decibels::gainToDecibels(envelope / thresholdLinear);
                float compressedDb = overThresholdDb / ratio;
                targetGain = juce::Decibels::decibelsToGain(compressedDb - overThresholdDb);
            }
            else
            {
                // Below threshold - apply expansion
                float belowThresholdDb = juce::Decibels::gainToDecibels(envelope / thresholdLinear);
                float expandedDb = belowThresholdDb * ratio;
                targetGain = juce::Decibels::decibelsToGain(expandedDb - belowThresholdDb);
            }
            
            // Smooth gain changes
            currentGain = targetGain + (currentGain - targetGain) * 0.1f;
            
            // Apply dynamic gain
            float output = input * currentGain;
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
}

//==============================================================================
bool ObserverAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ObserverAudioProcessor::createEditor()
{
    return new ObserverAudioProcessorEditor (*this);
}

//==============================================================================
void ObserverAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ObserverAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ObserverAudioProcessor();
}
