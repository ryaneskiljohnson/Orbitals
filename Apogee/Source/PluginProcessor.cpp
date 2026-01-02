/*
  ==============================================================================

    Apogee - Velocity & Energy Booster
    MIDI FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// Parameter IDs
const juce::String ApogeeAudioProcessor::PARAM_LIFT = "lift";
const juce::String ApogeeAudioProcessor::PARAM_CURVE = "curve";
const juce::String ApogeeAudioProcessor::PARAM_CEILING = "ceiling";
const juce::String ApogeeAudioProcessor::PARAM_MOMENTUM = "momentum";
const juce::String ApogeeAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
ApogeeAudioProcessor::ApogeeAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Apogee"), createParameterLayout())
{
    smoothedVelocities.fill(64.0f);
}

ApogeeAudioProcessor::~ApogeeAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ApogeeAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        PARAM_LIFT,
        "Lift",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        PARAM_CURVE,
        "Curve",
        juce::StringArray { "LINEAR", "EXP", "S", "LOG" },
        1 // Default to EXP
    ));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        PARAM_CEILING,
        "Ceiling",
        1, 127, 127
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        PARAM_MOMENTUM,
        "Momentum",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        PARAM_BYPASS,
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String ApogeeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ApogeeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ApogeeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ApogeeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ApogeeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ApogeeAudioProcessor::getNumPrograms()
{
    return 1;
}

int ApogeeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ApogeeAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ApogeeAudioProcessor::getProgramName (int index)
{
    return {};
}

void ApogeeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ApogeeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    smoothedVelocities.fill(64.0f);
}

void ApogeeAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ApogeeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

float ApogeeAudioProcessor::applyCurve(float normalizedInput, int curveType)
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

void ApogeeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Get parameters
    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    
    if (bypass)
    {
        return;
    }

    float lift = *parameters.getRawParameterValue(PARAM_LIFT) / 100.0f;
    int curveType = (int)*parameters.getRawParameterValue(PARAM_CURVE);
    int ceiling = (int)*parameters.getRawParameterValue(PARAM_CEILING);
    float momentum = *parameters.getRawParameterValue(PARAM_MOMENTUM) / 100.0f;
    
    // Calculate momentum smoothing factor (0 = instant, 1 = very smooth)
    float smoothingFactor = 1.0f - momentum;
    
    juce::MidiBuffer processedMidi;
    
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        int samplePosition = metadata.samplePosition;
        
        if (message.isNoteOn())
        {
            int noteNumber = message.getNoteNumber();
            float inputVelocity = (float)message.getVelocity();
            
            // Normalize velocity (0-1)
            float normalizedVel = inputVelocity / 127.0f;
            
            // Apply curve
            float curvedVel = applyCurve(normalizedVel, curveType);
            
            // Apply lift (boost)
            float boostedVel = curvedVel + (lift * (1.0f - curvedVel));
            
            // Apply momentum smoothing
            float targetVel = boostedVel * 127.0f;
            smoothedVelocities[noteNumber] += (targetVel - smoothedVelocities[noteNumber]) * (1.0f - smoothingFactor);
            
            // Apply ceiling
            int finalVelocity = juce::jlimit(1, ceiling, (int)smoothedVelocities[noteNumber]);
            
            // Create modified message
            auto modifiedMessage = juce::MidiMessage::noteOn(
                message.getChannel(),
                noteNumber,
                (juce::uint8)finalVelocity
            );
            
            processedMidi.addEvent(modifiedMessage, samplePosition);
        }
        else
        {
            // Pass through all other messages unchanged
            processedMidi.addEvent(message, samplePosition);
        }
    }
    
    midiMessages.swapWith(processedMidi);
}

//==============================================================================
bool ApogeeAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* ApogeeAudioProcessor::createEditor()
{
    return new ApogeeAudioProcessorEditor (*this);
}

//==============================================================================
void ApogeeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ApogeeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ApogeeAudioProcessor();
}
