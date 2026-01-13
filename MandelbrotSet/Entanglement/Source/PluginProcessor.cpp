/*
  ==============================================================================

    Entanglement - Quantum Delay
    Audio FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String EntanglementAudioProcessor::PARAM_TIME = "time";
const juce::String EntanglementAudioProcessor::PARAM_FEEDBACK = "feedback";
const juce::String EntanglementAudioProcessor::PARAM_MIX = "mix";
const juce::String EntanglementAudioProcessor::PARAM_DAMPING = "damping";
const juce::String EntanglementAudioProcessor::PARAM_WETDRY = "wetdry";
const juce::String EntanglementAudioProcessor::PARAM_HIGHPASS = "highpass";
const juce::String EntanglementAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
EntanglementAudioProcessor::EntanglementAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Entanglement"), createParameterLayout())
{
}

EntanglementAudioProcessor::~EntanglementAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout EntanglementAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_TIME, 1),
        "Distance",
        juce::NormalisableRange<float> (10.0f, 2000.0f, 1.0f),
        250.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + " ms"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_FEEDBACK, 2),
        "Coupling",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        30.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_MIX, 3),
        "Coherence",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_DAMPING, 4),
        "Decay",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_WETDRY, 5),
        "Wet/Dry",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_HIGHPASS, 6),
        "Highpass",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f),
        20.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { 
            if (value >= 1000.0f) return juce::String (value / 1000.0f, 1) + " kHz";
            return juce::String ((int)value) + " Hz"; 
        }
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 7),
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String EntanglementAudioProcessor::getName() const { return JucePlugin_Name; }
bool EntanglementAudioProcessor::acceptsMidi() const { return false; }
bool EntanglementAudioProcessor::producesMidi() const { return false; }
bool EntanglementAudioProcessor::isMidiEffect() const { return false; }
double EntanglementAudioProcessor::getTailLengthSeconds() const { return 2.0; } // Max delay time
int EntanglementAudioProcessor::getNumPrograms() { return 1; }
int EntanglementAudioProcessor::getCurrentProgram() { return 0; }
void EntanglementAudioProcessor::setCurrentProgram (int) {}
const juce::String EntanglementAudioProcessor::getProgramName (int) { return {}; }
void EntanglementAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void EntanglementAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    
    // Prepare delay lines (max 2 seconds)
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;
    
    delayLineL.prepare(spec);
    delayLineR.prepare(spec);
    delayLineL.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0));
    delayLineR.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0));
    
    // Prepare damping filters (low-pass for high-frequency damping)
    dampingCoeffsL = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 20000.0f);
    dampingCoeffsR = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 20000.0f);
    dampingFilterL.prepare(spec);
    dampingFilterR.prepare(spec);
    dampingFilterL.coefficients = dampingCoeffsL;
    dampingFilterR.coefficients = dampingCoeffsR;
    
    // Prepare highpass filters
    highpassCoeffsL = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);
    highpassCoeffsR = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);
    highpassFilterL.prepare(spec);
    highpassFilterR.prepare(spec);
    highpassFilterL.coefficients = highpassCoeffsL;
    highpassFilterR.coefficients = highpassCoeffsR;
}

void EntanglementAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EntanglementAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void EntanglementAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    if (bypass) {
        outputLevel.store(inputLevel.load());
        return;
    }

    // Get parameters
    float timeMs = *parameters.getRawParameterValue(PARAM_TIME);
    float feedback = *parameters.getRawParameterValue(PARAM_FEEDBACK) / 100.0f;
    float mix = *parameters.getRawParameterValue(PARAM_MIX) / 100.0f;
    float dampingAmount = *parameters.getRawParameterValue(PARAM_DAMPING) / 100.0f;
    float wetDry = *parameters.getRawParameterValue(PARAM_WETDRY) / 100.0f;
    float highpassFreq = *parameters.getRawParameterValue(PARAM_HIGHPASS);
    
    // Convert delay time to samples
    float delayTimeSamples = (timeMs / 1000.0f) * static_cast<float>(sampleRate);
    delayTimeSamples = juce::jlimit(0.0f, static_cast<float>(delayLineL.getMaximumDelayInSamples() - 1), delayTimeSamples);
    
    // Update delay time if changed
    if (std::abs(delayTimeSamples - lastDelayTimeL) > 0.5f)
    {
        delayLineL.setDelay(delayTimeSamples);
        delayLineR.setDelay(delayTimeSamples);
        lastDelayTimeL = delayTimeSamples;
        lastDelayTimeR = delayTimeSamples;
    }
    
    // Update damping filter cutoff (dampingAmount: 0% = no damping, 100% = heavy damping)
    float dampingCutoff = 20000.0f * (1.0f - dampingAmount * 0.9f); // 2kHz to 20kHz range
    dampingCutoff = juce::jmax(2000.0f, dampingCutoff);
    dampingCoeffsL = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, dampingCutoff);
    dampingCoeffsR = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, dampingCutoff);
    dampingFilterL.coefficients = dampingCoeffsL;
    dampingFilterR.coefficients = dampingCoeffsR;
    
    // Update highpass filter
    highpassCoeffsL = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, highpassFreq);
    highpassCoeffsR = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, highpassFreq);
    highpassFilterL.coefficients = highpassCoeffsL;
    highpassFilterR.coefficients = highpassCoeffsR;
    
    // Process each channel
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto& delayLine = (channel == 0) ? delayLineL : delayLineR;
        auto& dampingFilter = (channel == 0) ? dampingFilterL : dampingFilterR;
        auto& highpassFilter = (channel == 0) ? highpassFilterL : highpassFilterR;
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float input = channelData[sample];
            
            // Apply highpass filter to input
            float filteredInput = highpassFilter.processSample(input);
            
            // Read delayed signal
            float delayed = delayLine.popSample(0);
            
            // Apply damping (low-pass filter)
            delayed = dampingFilter.processSample(delayed);
            
            // Mix delayed signal with input (coherence/mix parameter)
            float wetSignal = filteredInput * (1.0f - mix) + delayed * mix;
            
            // Apply wet/dry mix
            float wetLevel = wetDry;
            float dryLevel = 1.0f - wetDry;
            float output = input * dryLevel + wetSignal * wetLevel;
            
            // Write to delay line with feedback
            delayLine.pushSample(0, filteredInput + delayed * feedback);
            
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
bool EntanglementAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* EntanglementAudioProcessor::createEditor()
{
    return new EntanglementAudioProcessorEditor (*this);
}

//==============================================================================
void EntanglementAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void EntanglementAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EntanglementAudioProcessor();
}
