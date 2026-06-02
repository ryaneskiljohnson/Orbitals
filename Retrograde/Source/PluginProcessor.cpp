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

bool RetrogradeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RetrogradeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RetrogradeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
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

    const int mode = (int) *parameters.getRawParameterValue (PARAM_MODE);
    const int scope = (int) *parameters.getRawParameterValue (PARAM_SCOPE);
    const float symmetry = *parameters.getRawParameterValue (PARAM_SYMMETRY);
    const int echoCount = juce::jlimit (0, 8, (int) *parameters.getRawParameterValue (PARAM_ECHO));
    const int targetSize = juce::jmax (1, scope * 4);
    const int numSamples = buffer.getNumSamples();

    const float blend = 0.5f - 0.5f * std::cos (symmetry * juce::MathConstants<float>::pi / 180.0f);
    const int echoSpacing = echoCount > 0 ? juce::jmax (1, numSamples / (echoCount + 1)) : 0;

    juce::MidiBuffer processedMidi;

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();

        if (! message.isNoteOn() && ! message.isNoteOff())
        {
            processedMidi.addEvent (message, metadata.samplePosition);
            continue;
        }

        noteBuffer.push_back ({ message, metadata.samplePosition, 0.0 });
    }

    auto emitWithEcho = [&processedMidi, echoCount, echoSpacing, numSamples] (const std::vector<NoteInfo>& events)
    {
        for (int echo = 0; echo <= echoCount; ++echo)
        {
            const int sampleOffset = echo * echoSpacing;

            for (const auto& event : events)
            {
                const int samplePosition = juce::jlimit (0, numSamples - 1, event.samplePosition + sampleOffset);
                processedMidi.addEvent (event.message, samplePosition);
            }
        }
    };

    while ((int) noteBuffer.size() >= targetSize)
    {
        std::vector<NoteInfo> window (noteBuffer.begin(), noteBuffer.begin() + targetSize);
        noteBuffer.erase (noteBuffer.begin(), noteBuffer.begin() + targetSize);

        std::vector<NoteInfo> transformed = window;

        if (mode == 0)
        {
            std::vector<int> velocities;
            for (const auto& event : window)
                if (event.message.isNoteOn())
                    velocities.push_back (event.message.getVelocity());

            std::vector<int> reversedVelocities = velocities;
            std::reverse (reversedVelocities.begin(), reversedVelocities.end());

            int velocityIndex = 0;
            for (auto& event : transformed)
            {
                if (event.message.isNoteOn() && velocityIndex < (int) velocities.size())
                {
                    const int blendedVelocity = juce::jlimit (
                        1, 127,
                        (int) std::lround (velocities[(size_t) velocityIndex] * (1.0f - blend)
                                           + reversedVelocities[(size_t) velocityIndex] * blend));
                    event.message = juce::MidiMessage::noteOn (event.message.getChannel(),
                                                               event.message.getNoteNumber(),
                                                               (juce::uint8) blendedVelocity);
                    ++velocityIndex;
                }
            }
        }
        else if (mode == 1)
        {
            std::vector<NoteInfo> reversed = window;
            std::reverse (reversed.begin(), reversed.end());

            for (size_t i = 0; i < transformed.size(); ++i)
            {
                const int blendedPosition = (int) std::lround (window[i].samplePosition * (1.0f - blend)
                                                               + reversed[i].samplePosition * blend);
                transformed[i].samplePosition = juce::jlimit (0, numSamples - 1, blendedPosition);
            }
        }
        else
        {
            for (size_t i = 0; i < transformed.size(); ++i)
            {
                const float sourceIndex = (float) i * (1.0f - blend)
                                        + (float) (window.size() - 1 - i) * blend;
                const size_t pickIndex = (size_t) juce::jlimit (0, (int) window.size() - 1, (int) std::lround (sourceIndex));
                transformed[i] = window[pickIndex];
                transformed[i].samplePosition = window[i].samplePosition;
            }
        }

        emitWithEcho (transformed);
    }

    midiMessages.swapWith (processedMidi);
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
