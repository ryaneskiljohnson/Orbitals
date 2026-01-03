/*
  ==============================================================================

    Retrograde - MIDI Motion Reversal
    MIDI FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String RetrogradeAudioProcessor::PARAM_MODE = "mode";
const juce::String RetrogradeAudioProcessor::PARAM_SCOPE = "scope";
const juce::String RetrogradeAudioProcessor::PARAM_SYMMETRY = "symmetry";
const juce::String RetrogradeAudioProcessor::PARAM_ECHO = "echo";
const juce::String RetrogradeAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
RetrogradeAudioProcessor::RetrogradeAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Retrograde"), createParameterLayout())
{
}

RetrogradeAudioProcessor::~RetrogradeAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout RetrogradeAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID (PARAM_MODE, 1),
        "Mode",
        juce::StringArray { "VELOCITY", "TIMING", "PHRASE" }, 0
    ));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID (PARAM_SCOPE, 2),
        "Scope", 1, 4, 2
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_SYMMETRY, 3),
        "Symmetry",
        juce::NormalisableRange<float> (0.0f, 360.0f, 1.0f), 180.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID (PARAM_ECHO, 4),
        "Echo", 0, 8, 0
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 5),
        "Bypass", false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String RetrogradeAudioProcessor::getName() const { return JucePlugin_Name; }
bool RetrogradeAudioProcessor::acceptsMidi() const { return true; }
bool RetrogradeAudioProcessor::producesMidi() const { return true; }
bool RetrogradeAudioProcessor::isMidiEffect() const { return true; }
double RetrogradeAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int RetrogradeAudioProcessor::getNumPrograms() { return 1; }
int RetrogradeAudioProcessor::getCurrentProgram() { return 0; }
void RetrogradeAudioProcessor::setCurrentProgram (int) {}
const juce::String RetrogradeAudioProcessor::getProgramName (int) { return {}; }
void RetrogradeAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void RetrogradeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    noteBuffer.clear();
}

void RetrogradeAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RetrogradeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void RetrogradeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    if (bypass)
        return;

    int mode = (int)*parameters.getRawParameterValue(PARAM_MODE);
    int scope = (int)*parameters.getRawParameterValue(PARAM_SCOPE);
    float symmetry = *parameters.getRawParameterValue(PARAM_SYMMETRY);
    int echoCount = (int)*parameters.getRawParameterValue(PARAM_ECHO);

    // Add incoming notes to buffer
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        if (message.isNoteOn() || message.isNoteOff())
        {
            NoteEvent event;
            event.message = message;
            event.samplePosition = metadata.samplePosition;
            event.timestamp = metadata.samplePosition;
            noteBuffer.push_back(event);
        }
    }

    juce::MidiBuffer processedMidi;

    // When buffer reaches scope, reverse and output
    int targetSize = scope * 4; // Assuming 4 notes per unit of scope
    
    if (noteBuffer.size() >= targetSize)
    {
        std::vector<NoteEvent> toReverse(noteBuffer.begin(), noteBuffer.begin() + targetSize);
        noteBuffer.erase(noteBuffer.begin(), noteBuffer.begin() + targetSize);
        
        if (mode == 0) // VELOCITY
        {
            // Reverse velocity contours
            std::vector<int> velocities;
            for (auto& event : toReverse)
            {
                if (event.message.isNoteOn())
                    velocities.push_back(event.message.getVelocity());
            }
            std::reverse(velocities.begin(), velocities.end());
            
            int velIndex = 0;
            for (auto& event : toReverse)
            {
                if (event.message.isNoteOn() && velIndex < velocities.size())
                {
                    auto newMessage = juce::MidiMessage::noteOn(event.message.getChannel(),
                                                                event.message.getNoteNumber(),
                                                                (juce::uint8)velocities[velIndex++]);
                    processedMidi.addEvent(newMessage, event.samplePosition);
                }
                else
                {
                    processedMidi.addEvent(event.message, event.samplePosition);
                }
            }
        }
        else if (mode == 1) // TIMING
        {
            // Reverse timing
            std::reverse(toReverse.begin(), toReverse.end());
            for (auto& event : toReverse)
            {
                processedMidi.addEvent(event.message, event.samplePosition);
            }
        }
        else if (mode == 2) // PHRASE
        {
            // Reverse note order
            std::reverse(toReverse.begin(), toReverse.end());
            for (auto& event : toReverse)
            {
                processedMidi.addEvent(event.message, event.samplePosition);
            }
        }
    }

    midiMessages.swapWith(processedMidi);
}

//==============================================================================
bool RetrogradeAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* RetrogradeAudioProcessor::createEditor()
{
    return new RetrogradeAudioProcessorEditor (*this);
}

//==============================================================================
void RetrogradeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void RetrogradeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RetrogradeAudioProcessor();
}
