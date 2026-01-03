/*
  ==============================================================================

    Tidal - Rhythmic Push & Pull
    MIDI FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// Parameter IDs
const juce::String TidalAudioProcessor::PARAM_AMPLITUDE = "amplitude";
const juce::String TidalAudioProcessor::PARAM_PHASE = "phase";
const juce::String TidalAudioProcessor::PARAM_RATE = "rate";
const juce::String TidalAudioProcessor::PARAM_SHAPE = "shape";
const juce::String TidalAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
TidalAudioProcessor::TidalAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Tidal"), createParameterLayout())
{
}

TidalAudioProcessor::~TidalAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout TidalAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_AMPLITUDE, 1),
        "Amplitude",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + "%"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_PHASE, 2),
        "Phase",
        juce::NormalisableRange<float> (0.0f, 360.0f, 1.0f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + "°"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID (PARAM_RATE, 3),
        "Rate",
        juce::StringArray { "1/4", "1/8", "1/16", "1/32" },
        0
    ));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID (PARAM_SHAPE, 4),
        "Shape",
        juce::StringArray { "SINE", "TRI", "SAW", "SQR" },
        0
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 5),
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String TidalAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TidalAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TidalAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TidalAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TidalAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TidalAudioProcessor::getNumPrograms()
{
    return 1;
}

int TidalAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TidalAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TidalAudioProcessor::getProgramName (int index)
{
    return {};
}

void TidalAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TidalAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    phase = 0.0;
}

void TidalAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TidalAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Standard stereo only
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
  #endif
}
#endif

double TidalAudioProcessor::getRateMultiplier(int rateIndex)
{
    switch (rateIndex)
    {
        case 0: return 0.25;  // 1/4 - slowest
        case 1: return 0.5;   // 1/8
        case 2: return 1.0;   // 1/16
        case 3: return 2.0;   // 1/32 - fastest
        default: return 0.5;
    }
}

double TidalAudioProcessor::getWaveValue(double phaseIn, int shapeIndex)
{
    double normalizedPhase = std::fmod(phaseIn, juce::MathConstants<double>::twoPi);
    
    switch (shapeIndex)
    {
        case 0: // SINE
            return std::sin(normalizedPhase);
            
        case 1: // TRIANGLE
        {
            double t = normalizedPhase / juce::MathConstants<double>::twoPi;
            return (t < 0.5) ? (t * 4.0 - 1.0) : (3.0 - t * 4.0);
        }
        
        case 2: // SAW
        {
            double t = normalizedPhase / juce::MathConstants<double>::twoPi;
            return t * 2.0 - 1.0;
        }
        
        case 3: // SQUARE
        {
            double t = normalizedPhase / juce::MathConstants<double>::twoPi;
            return (t < 0.5) ? -1.0 : 1.0;
        }
        
        default:
            return std::sin(normalizedPhase);
    }
}

void TidalAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Get parameters
    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    
    if (bypass)
    {
        // Pass through unchanged
        return;
    }

    float amplitude = *parameters.getRawParameterValue(PARAM_AMPLITUDE);
    float phaseOffset = *parameters.getRawParameterValue(PARAM_PHASE);
    int rateIndex = (int)*parameters.getRawParameterValue(PARAM_RATE);
    int shapeIndex = (int)*parameters.getRawParameterValue(PARAM_SHAPE);

    // Get timing info
    auto playHead = getPlayHead();
    if (playHead == nullptr)
    {
        // No playhead info, pass through
        return;
    }

    auto positionInfo = playHead->getPosition();
    if (!positionInfo.hasValue() || !positionInfo->getBpm().hasValue())
    {
        // No tempo info, pass through
        return;
    }

    double bpm = *positionInfo->getBpm();
    double beatsPerSample = bpm / (60.0 * sampleRate);
    
    // Calculate rate in beats based on note division
    double rateInBeats = getRateMultiplier(rateIndex);
    
    // Output buffer for modified MIDI
    juce::MidiBuffer processedMidi;
    
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        int samplePosition = metadata.samplePosition;
        
        if (message.isNoteOn() || message.isNoteOff())
        {
            // Calculate current phase for this sample
            double currentPhase = phase + (beatsPerSample * samplePosition * juce::MathConstants<double>::twoPi / rateInBeats);
            currentPhase += (phaseOffset / 360.0) * juce::MathConstants<double>::twoPi;
            
            // Get wave value (-1 to 1)
            double waveValue = getWaveValue(currentPhase, shapeIndex);
            
            // Calculate timing offset in samples based on amplitude
            // Max offset is ~50ms at 100% amplitude
            double maxOffsetMs = 50.0;
            double offsetMs = waveValue * (amplitude / 100.0) * maxOffsetMs;
            int offsetSamples = (int)(offsetMs * sampleRate / 1000.0);
            
            // Apply offset
            int newSamplePosition = samplePosition + offsetSamples;
            newSamplePosition = juce::jlimit(0, buffer.getNumSamples() - 1, newSamplePosition);
            
            processedMidi.addEvent(message, newSamplePosition);
        }
        else
        {
            // Pass through non-note messages unchanged
            processedMidi.addEvent(message, samplePosition);
        }
    }
    
    // Update phase for next block
    phase += beatsPerSample * buffer.getNumSamples() * juce::MathConstants<double>::twoPi / rateInBeats;
    phase = std::fmod(phase, juce::MathConstants<double>::twoPi * 1000.0); // Wrap to prevent overflow
    
    // Replace input MIDI with processed MIDI
    midiMessages.swapWith(processedMidi);
}

//==============================================================================
bool TidalAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* TidalAudioProcessor::createEditor()
{
    return new TidalAudioProcessorEditor (*this);
}

//==============================================================================
void TidalAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void TidalAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
    return new TidalAudioProcessor();
}
