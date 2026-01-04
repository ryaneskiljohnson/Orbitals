/*
  ==============================================================================

    Zenith - Velocity Expander
    MIDI FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String ZenithAudioProcessor::PARAM_EXPANSION = "expansion";
const juce::String ZenithAudioProcessor::PARAM_THRESHOLD = "threshold";
const juce::String ZenithAudioProcessor::PARAM_CEILING = "ceiling";
const juce::String ZenithAudioProcessor::PARAM_CURVE = "curve";
const juce::String ZenithAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
ZenithAudioProcessor::ZenithAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Zenith"), createParameterLayout())
{
}

ZenithAudioProcessor::~ZenithAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ZenithAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_EXPANSION, 1),
        "Expansion",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_THRESHOLD, 2),
        "Threshold",
        juce::NormalisableRange<float> (0.0f, 127.0f, 1.0f), 64.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_CEILING, 3),
        "Ceiling",
        juce::NormalisableRange<float> (0.0f, 127.0f, 1.0f), 127.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_CURVE, 4),
        "Curve",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 5),
        "Bypass", false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String ZenithAudioProcessor::getName() const { return JucePlugin_Name; }
bool ZenithAudioProcessor::acceptsMidi() const { return true; }
bool ZenithAudioProcessor::producesMidi() const { return true; }
bool ZenithAudioProcessor::isMidiEffect() const { return true; }
double ZenithAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int ZenithAudioProcessor::getNumPrograms() { return 1; }
int ZenithAudioProcessor::getCurrentProgram() { return 0; }
void ZenithAudioProcessor::setCurrentProgram (int) {}
const juce::String ZenithAudioProcessor::getProgramName (int) { return {}; }
void ZenithAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void ZenithAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
}

void ZenithAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ZenithAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ZenithAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    if (bypass)
        return;

    float expansion = *parameters.getRawParameterValue(PARAM_EXPANSION) / 100.0f;
    float threshold = *parameters.getRawParameterValue(PARAM_THRESHOLD);
    float ceiling = *parameters.getRawParameterValue(PARAM_CEILING);
    float curve = *parameters.getRawParameterValue(PARAM_CURVE) / 100.0f;

    juce::MidiBuffer processedMidi;

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        int samplePosition = metadata.samplePosition;
        
        if (message.isNoteOn())
        {
            int originalVelocity = message.getVelocity();
            int newVelocity = originalVelocity;
            
            // Only expand velocities above threshold
            if (originalVelocity >= threshold)
            {
                // Calculate expansion amount based on distance from threshold
                float normalizedVelocity = (originalVelocity - threshold) / (127.0f - threshold);
                
                // Apply curve (0.5 = linear, <0.5 = softer, >0.5 = harder)
                float curveFactor = 0.5f + (curve - 0.5f);
                float curved = std::pow(normalizedVelocity, 2.0f - curveFactor * 2.0f);
                
                // Expand towards ceiling
                float expandedVelocity = threshold + (curved * (ceiling - threshold));
                
                // Blend based on expansion amount
                newVelocity = (int)(originalVelocity * (1.0f - expansion) + expandedVelocity * expansion);
                
                // Clamp to valid range
                newVelocity = juce::jlimit(1, (int)ceiling, newVelocity);
            }
            
            // Notify UI of MIDI note
            if (auto* editor = dynamic_cast<ZenithAudioProcessorEditor*>(getActiveEditor()))
            {
                editor->notifyMIDINote(message.getNoteNumber(), newVelocity);
            }
            
            auto newMessage = juce::MidiMessage::noteOn(message.getChannel(), 
                                                        message.getNoteNumber(), 
                                                        (juce::uint8)newVelocity);
            processedMidi.addEvent(newMessage, samplePosition);
        }
        else
        {
            processedMidi.addEvent(message, samplePosition);
        }
    }

    midiMessages.swapWith(processedMidi);
}

//==============================================================================
bool ZenithAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ZenithAudioProcessor::createEditor()
{
    return new ZenithAudioProcessorEditor (*this);
}

//==============================================================================
void ZenithAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ZenithAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZenithAudioProcessor();
}
