/*
  ==============================================================================

    Fabric - Spacetime Reverb
    Audio FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <iostream>

// Parameter IDs
const juce::String FabricAudioProcessor::PARAM_SIZE = "size";
const juce::String FabricAudioProcessor::PARAM_DIFFUSION = "diffusion";
const juce::String FabricAudioProcessor::PARAM_DAMPING = "damping";
const juce::String FabricAudioProcessor::PARAM_PREDELAY = "predelay";
const juce::String FabricAudioProcessor::PARAM_MIX = "mix";
const juce::String FabricAudioProcessor::PARAM_WETDRY = "wetdry";
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

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_WETDRY, 6),
        "Wet/Dry",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { 
            if (value == 0.0f) {
                return juce::String ("100% Dry");
            } else if (value == 100.0f) {
                return juce::String ("100% Wet");
            } else if (value == 50.0f) {
                return juce::String ("50/50");
            } else {
                return juce::String ((int)value) + "% Wet";
            }
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
    // Return tail length based on reverb time (max ~3 seconds)
    return 3.0;
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
    
    // Prepare reverb DSP
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    
    reverb.prepare(spec);
    
    // Initialize predelay lines (max 200ms at 192kHz = ~38400 samples)
    int maxDelaySamples = static_cast<int>(sampleRate * 0.2);
    
    // Prepare delay lines with the same spec as reverb
    predelayLineL.prepare(spec);
    predelayLineL.setMaximumDelayInSamples(maxDelaySamples);
    predelayLineL.setDelay(0);
    
    predelayLineR.prepare(spec);
    predelayLineR.setMaximumDelayInSamples(maxDelaySamples);
    predelayLineR.setDelay(0);
    
    // Initialize parameter smoothing (20ms ramp time to avoid zipper noise)
    smoothedSize.reset(sampleRate, 0.02);
    smoothedDiffusion.reset(sampleRate, 0.02);
    smoothedDamping.reset(sampleRate, 0.02);
    smoothedPredelay.reset(sampleRate, 0.02);
    smoothedMix.reset(sampleRate, 0.001); // 1ms for faster response on mix
    smoothedWetDry.reset(sampleRate, 0.001); // 1ms for faster response on wet/dry
    
    // Set initial values
    smoothedSize.setCurrentAndTargetValue(*parameters.getRawParameterValue(PARAM_SIZE) / 100.0f);
    smoothedDiffusion.setCurrentAndTargetValue(*parameters.getRawParameterValue(PARAM_DIFFUSION) / 100.0f);
    smoothedDamping.setCurrentAndTargetValue(*parameters.getRawParameterValue(PARAM_DAMPING) / 100.0f);
    smoothedPredelay.setCurrentAndTargetValue(*parameters.getRawParameterValue(PARAM_PREDELAY));
    smoothedMix.setCurrentAndTargetValue(*parameters.getRawParameterValue(PARAM_MIX) / 100.0f);
    smoothedWetDry.setCurrentAndTargetValue(*parameters.getRawParameterValue(PARAM_WETDRY) / 100.0f);
    
    // Initialize reverb parameters with default values
    reverbParams.roomSize = 0.5f;
    reverbParams.damping = 0.5f;
    reverbParams.width = 1.0f;
    reverbParams.wetLevel = 0.33f;
    reverbParams.dryLevel = 0.4f;
    reverbParams.freezeMode = 0.0f;
    reverb.setParameters(reverbParams);
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
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // === AUDIO PROCESSING ===
    
    auto totalNumInputChannels  = getTotalNumInputChannels();

    // Calculate input level for UI - with bounds checking
    float inLevel = 0.0f;
    
    if (numSamples > 0 && numChannels > 0) {
        for (int channel = 0; channel < totalNumInputChannels && channel < numChannels; ++channel) {
            float channelLevel = buffer.getRMSLevel(channel, 0, numSamples);
            inLevel = std::max(inLevel, channelLevel);
        }
    }
    inputLevel.store(juce::Decibels::gainToDecibels(inLevel, -100.0f));
    
    // Debug: Log input levels to verify mic input
    static int inputCheckCounter = 0;
    if (++inputCheckCounter % 256 == 0) // Every 256 blocks (~3 seconds)
    {
        float maxSample = 0.0f;
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getReadPointer(channel);
            for (int sample = 0; sample < numSamples; ++sample)
            {
                maxSample = std::max(maxSample, std::abs(channelData[sample]));
            }
        }
        std::cerr << "Input: RMS=" << inLevel << " Max=" << maxSample << " | Channels=" << numChannels << std::endl;
    }
    
    // Get parameters
    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    
    if (bypass)
    {
        outputLevel.store(inputLevel.load());
        return;
    }

    // Get parameters and update smoothed values
    float rawSize = *parameters.getRawParameterValue(PARAM_SIZE);
    float rawDiffusion = *parameters.getRawParameterValue(PARAM_DIFFUSION);
    float rawDamping = *parameters.getRawParameterValue(PARAM_DAMPING);
    float rawPredelay = *parameters.getRawParameterValue(PARAM_PREDELAY);
    float rawMix = *parameters.getRawParameterValue(PARAM_MIX);
    float rawWetDry = *parameters.getRawParameterValue(PARAM_WETDRY);
    
    smoothedSize.setTargetValue(rawSize / 100.0f);
    smoothedDiffusion.setTargetValue(rawDiffusion / 100.0f);
    smoothedDamping.setTargetValue(rawDamping / 100.0f);
    smoothedPredelay.setTargetValue(rawPredelay);
    smoothedMix.setTargetValue(rawMix / 100.0f);
    smoothedWetDry.setTargetValue(rawWetDry / 100.0f);
    
    // Get smoothed parameter values (skip to apply over full buffer for instant response)
    float size = smoothedSize.skip(numSamples);
    float diffusion = smoothedDiffusion.skip(numSamples);
    float damping = smoothedDamping.skip(numSamples);
    float predelayMs = smoothedPredelay.skip(numSamples);
    float mix = smoothedMix.skip(numSamples);
    float wetDry = smoothedWetDry.skip(numSamples);
    
    // Advanced reverb parameter mapping:
    // 
    // SIZE (Weave): Controls the room size and decay time
    // - Larger size = longer reverb tail
    // - Exponential scaling for more musical control
    float roomSize = std::pow(size, 1.5f); // Exponential curve for better control
    roomSize = juce::jlimit(0.0f, 1.0f, roomSize);
    
    // DIFFUSION (Thread Density): Controls stereo width and early reflections
    // - Higher diffusion = wider stereo image and denser reflections
    // - Maps to stereo width (0.5 to 1.0 range for musical results)
    float width = 0.5f + (diffusion * 0.5f); // 0.5 = mono, 1.0 = full stereo
    
    // DAMPING (Fold): Controls high-frequency absorption
    // - Higher damping = darker, more absorbed sound (like curtains/carpets)
    // - Lower damping = brighter, more reflective sound (like hard surfaces)
    float dampingAmount = damping;
    
    // Apply reverb parameters
    reverbParams.roomSize = roomSize;
    reverbParams.damping = dampingAmount;
    reverbParams.width = width; // Controlled by diffusion
    reverbParams.freezeMode = 0.0f;
    
    // Mix parameter (Warp) controls overall reverb intensity (0% = no reverb, 100% = full reverb)
    // Wet/dry parameter controls balance between wet and dry signals (0% = all dry, 100% = all wet)
    // 
    // JUCE's reverb handles the mixing internally, so we set wetLevel and dryLevel directly:
    // - wetLevel: amount of reverb signal (0-1)
    // - dryLevel: amount of original signal (0-1)
    // 
    // The reverb will mix: output = input * dryLevel + reverb(input) * wetLevel
    // 
    // Formula:
    // - wetLevel = mix * wetDry (reverb amount scaled by wet/dry balance)
    // - dryLevel = mix * (1.0 - wetDry) + (1.0 - mix) (dry signal from reverb + bypassed signal)
    // 
    // This ensures unity gain: when mix=1.0 and wetDry=0.5, we get 50% wet + 50% dry = 100% signal
    // 
    // Examples:
    // - mix=1.0, wetDry=0.5: wetLevel=0.5, dryLevel=0.5 (50/50 mix, full reverb)
    // - mix=0.5, wetDry=1.0: wetLevel=0.5, dryLevel=0.5 (50% reverb, all wet)
    // - mix=1.0, wetDry=1.0: wetLevel=1.0, dryLevel=0.0 (100% wet, no dry)
    // - mix=1.0, wetDry=0.0: wetLevel=0.0, dryLevel=1.0 (100% dry, no reverb)
    // - mix=0.0, wetDry=0.5: wetLevel=0.0, dryLevel=1.0 (no reverb, all dry)
    float reverbIntensity = mix; // Overall reverb amount (0-1)
    reverbParams.wetLevel = reverbIntensity * wetDry;
    reverbParams.dryLevel = reverbIntensity * (1.0f - wetDry) + (1.0f - reverbIntensity);
    
    // Debug wet/dry calculation
    static int wetDryDebugCounter = 0;
    if (++wetDryDebugCounter % 256 == 0)
    {
        std::cerr << "💧 WET/DRY: Raw=" << rawWetDry << "% → Normalized=" << wetDry 
                  << " | Mix=" << mix << " → WetLvl=" << reverbParams.wetLevel 
                  << " DryLvl=" << reverbParams.dryLevel << std::endl;
    }
    
    // Ensure we always have some signal (safety check)
    if (reverbParams.wetLevel < 0.001f && reverbParams.dryLevel < 0.001f)
    {
        reverbParams.dryLevel = 1.0f; // Fallback to 100% dry if both are too low
    }
    
    // Ensure parameters are valid
    reverbParams.wetLevel = juce::jlimit(0.0f, 1.0f, reverbParams.wetLevel);
    reverbParams.dryLevel = juce::jlimit(0.0f, 1.0f, reverbParams.dryLevel);
    
    // Update reverb parameters - this must be called before processing
    reverb.setParameters(reverbParams);
    
    // Debug: Log parameter changes
    static float lastSize = -1, lastDiffusion = -1, lastDamping = -1, lastMix = -1, lastWetDry = -1;
    bool paramChanged = (lastSize < 0) || // First time
                        std::fabs(rawSize - lastSize) > 0.5f || 
                        std::fabs(rawDiffusion - lastDiffusion) > 0.5f || 
                        std::fabs(rawDamping - lastDamping) > 0.5f || 
                        std::fabs(rawMix - lastMix) > 0.5f ||
                        std::fabs(rawWetDry - lastWetDry) > 0.5f;
    
    if (paramChanged)
    {
        std::cerr << "🎚️ PARAMETER CHANGED: Size=" << rawSize << " Diff=" << rawDiffusion 
                  << " Damp=" << rawDamping << " Mix=" << rawMix << " W/D=" << rawWetDry 
                  << " → WetLvl=" << reverbParams.wetLevel << " DryLvl=" << reverbParams.dryLevel << std::endl;
        lastSize = rawSize;
        lastDiffusion = rawDiffusion;
        lastDamping = rawDamping;
        lastMix = rawMix;
        lastWetDry = rawWetDry;
    }
    
    // PREDELAY (Time Dilation): Adds initial delay before reverb
    // - Creates spatial separation between direct sound and reverb
    // - Useful for vocals and instruments to maintain clarity
    float predelaySamples = static_cast<float>((predelayMs / 1000.0) * currentSampleRate);
    
    // Clamp delay to maximum to prevent out-of-bounds access
    int maxDelaySamples = static_cast<int>(currentSampleRate * 0.2); // 200ms max
    predelaySamples = juce::jlimit(0.0f, static_cast<float>(maxDelaySamples - 1), predelaySamples);
    
    // Only set delay if it's valid and the delay lines are prepared
    if (predelaySamples >= 0.0f && predelaySamples < maxDelaySamples)
    {
        predelayLineL.setDelay(predelaySamples);
        predelayLineR.setDelay(predelaySamples);
    }
    else
    {
        predelayLineL.setDelay(0);
        predelayLineR.setDelay(0);
    }
    
    // Apply predelay (only if delay > 0.5ms to avoid unnecessary processing)
    if (predelayMs > 0.5f && predelaySamples > 0.5f && predelaySamples < maxDelaySamples)
    {
        if (buffer.getNumChannels() >= 1)
        {
            auto* channelDataL = buffer.getWritePointer(0);
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float input = channelDataL[sample];
                predelayLineL.pushSample(0, input);
                channelDataL[sample] = predelayLineL.popSample(0);
            }
        }
        
        if (buffer.getNumChannels() >= 2)
        {
            auto* channelDataR = buffer.getWritePointer(1);
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float input = channelDataR[sample];
                predelayLineR.pushSample(0, input);
                channelDataR[sample] = predelayLineR.popSample(0);
            }
        }
    }
    
    // Process with reverb
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
    
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
