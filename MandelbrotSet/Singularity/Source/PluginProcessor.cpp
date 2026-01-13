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
const juce::String SingularityAudioProcessor::PARAM_MIX = "mix";
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

    // Mix: 0 to 100% (wet/dry blend)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_MIX, 6),
        "Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 7),
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
    
    std::cout << "🎵 prepareToPlay called: sampleRate=" << sampleRate 
              << ", samplesPerBlock=" << samplesPerBlock 
              << ", InputChannels=" << getTotalNumInputChannels()
              << ", OutputChannels=" << getTotalNumOutputChannels() << std::endl;
}

void SingularityAudioProcessor::releaseResources()
{
    std::cout << "🛑 releaseResources called - audio processing stopped" << std::endl;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SingularityAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SingularityAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // === AUDIO PROCESSING ===
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    
    // Debug: Confirm processBlock is being called
    static int processBlockCounter = 0;
    if (++processBlockCounter == 1 || processBlockCounter % 100 == 0) // First call + every 100 blocks (~2 seconds)
    {
        std::cout << "✅ processBlock called (#" << processBlockCounter << "): Samples=" << numSamples << ", Channels=" << numChannels << ", InputChannels=" << totalNumInputChannels << std::endl;
    }

    // Calculate input level for UI - with bounds checking
    float inLevel = 0.0f;
    
    if (numSamples > 0 && numChannels > 0) {
        for (int channel = 0; channel < totalNumInputChannels && channel < numChannels; ++channel) {
            float channelLevel = buffer.getRMSLevel(channel, 0, numSamples);
            inLevel = std::max(inLevel, channelLevel);
        }
    }
    
    // Convert to dB with minimum of -100dB
    float inputLevelDb = juce::Decibels::gainToDecibels(inLevel, -100.0f);
    inputLevel.store(inputLevelDb);
    
    // Debug: Log when input level is calculated
    if (processBlockCounter == 1 || processBlockCounter % 100 == 0)
    {
        std::cout << "   Input level: inLevel=" << inLevel << " -> dB=" << inputLevelDb << std::endl;
    }
    
    // Debug: Log input levels to verify audio input (like Fabric)
    static int inputCheckCounter = 0;
    if (++inputCheckCounter == 1 || inputCheckCounter % 100 == 0) // First call + every 100 blocks (~2 seconds)
    {
        float maxSample = 0.0f;
        for (int channel = 0; channel < numChannels; ++channel)
        {
            const float* channelData = buffer.getReadPointer(channel);
            for (int sample = 0; sample < numSamples; ++sample)
            {
                maxSample = std::max(maxSample, std::abs(channelData[sample]));
            }
        }
        std::cerr << "Singularity Input: RMS=" << inLevel << " Max=" << maxSample << " | Channels=" << numChannels << " | InputChannels=" << totalNumInputChannels << std::endl;
        std::cout << "Singularity Input: RMS=" << inLevel << " Max=" << maxSample << " | Channels=" << numChannels << " | InputChannels=" << totalNumInputChannels << std::endl;
    }

    // Get parameters
    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    
    if (bypass)
    {
        outputLevel.store(inputLevel.load());
        currentGainReduction.store(0.0f);
        return;
    }

    // Get parameters
    float thresholdDb = *parameters.getRawParameterValue(PARAM_THRESHOLD);
    float ratio = *parameters.getRawParameterValue(PARAM_RATIO);
    float attackMs = *parameters.getRawParameterValue(PARAM_ATTACK);
    float releaseMs = *parameters.getRawParameterValue(PARAM_RELEASE);
    float makeupDb = *parameters.getRawParameterValue(PARAM_MAKEUP);
    float mixPercent = *parameters.getRawParameterValue(PARAM_MIX);
    
    // Convert threshold to linear
    float thresholdLinear = juce::Decibels::decibelsToGain(thresholdDb);
    
    // Calculate attack and release coefficients
    float attackCoeff = std::exp(-1.0f / (attackMs * 0.001f * sampleRate));
    float releaseCoeff = std::exp(-1.0f / (releaseMs * 0.001f * sampleRate));
    
    // Convert makeup gain from dB to linear
    float makeupGain = juce::Decibels::decibelsToGain(makeupDb);
    
    // Convert mix from 0-100% to wet/dry levels
    float wetLevel = mixPercent / 100.0f;
    float dryLevel = 1.0f - wetLevel;
    
    // Process each channel
    for (int channel = 0; channel < numChannels; ++channel)
    {
        // Read from input buffer first (before writing)
        const float* inputData = buffer.getReadPointer(channel);
        float* outputData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Read input sample
            float input = inputData[sample];
            
            // Calculate input level (RMS-like, using absolute value)
            float inputLevel = std::abs(input);
            
            // Envelope follower (attack/release)
            if (inputLevel > envelope)
                envelope = inputLevel + (envelope - inputLevel) * attackCoeff;
            else
                envelope = inputLevel + (envelope - inputLevel) * releaseCoeff;
            
            // Calculate gain reduction
            if (envelope > thresholdLinear)
            {
                float overThresholdDb = juce::Decibels::gainToDecibels(envelope / thresholdLinear);
                float compressedDb = overThresholdDb / ratio;
                gainReduction = overThresholdDb - compressedDb;
            }
            else
            {
                gainReduction = 0.0f;
            }
            
            // Apply gain reduction
            float gainReductionLinear = juce::Decibels::decibelsToGain(-gainReduction);
            float compressed = input * gainReductionLinear * makeupGain;
            
            // Soft clip compressed signal
            compressed = juce::jlimit(-1.0f, 1.0f, compressed);
            
            // Mix wet (compressed) and dry (input) signals
            float output = (compressed * wetLevel) + (input * dryLevel);
            
            // Final soft clip
            output = juce::jlimit(-1.0f, 1.0f, output);
            
            // Write output sample
            outputData[sample] = output;
        }
    }
    
    // Store gain reduction for UI
    currentGainReduction.store(gainReduction);
    
    // Calculate output level for UI - with bounds checking (match Fabric)
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
