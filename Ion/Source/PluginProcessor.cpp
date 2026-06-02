/*
  ==============================================================================

    Ion - Harmonic Constraint Engine
    MIDI FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String IonAudioProcessor::PARAM_ROOT = "root";
const juce::String IonAudioProcessor::PARAM_SCALE = "scale";
const juce::String IonAudioProcessor::PARAM_FIELD_STRENGTH = "fieldStrength";
const juce::String IonAudioProcessor::PARAM_ATTRACTION_STRENGTH = "attractionStrength";
const juce::String IonAudioProcessor::PARAM_ATTRACTION_TYPE = "attractionType";
const juce::String IonAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
IonAudioProcessor::IonAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Ion"), createParameterLayout())
{
    notePositions.fill(0.0f);
}

IonAudioProcessor::~IonAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout IonAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID (PARAM_ROOT, 1),
        "Root", 0, 11, 0
    ));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID (PARAM_SCALE, 2),
        "Scale",
        juce::StringArray { "Major", "Minor", "Dorian", "Phrygian", "Lydian", "Mixolydian", "Aeolian", "Locrian", "Chromatic" },
        0
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_FIELD_STRENGTH, 3),
        "Field Strength",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_ATTRACTION_STRENGTH, 4),
        "Attraction Strength",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID (PARAM_ATTRACTION_TYPE, 5),
        "Attraction Type",
        juce::StringArray { "SNAP", "PULL", "GUIDE" },
        0
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 6),
        "Bypass", false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
std::vector<int> IonAudioProcessor::getScaleNotes(int root, int scaleType)
{
    std::vector<std::vector<int>> scales = {
        {0, 2, 4, 5, 7, 9, 11},           // Major
        {0, 2, 3, 5, 7, 8, 10},           // Minor
        {0, 2, 3, 5, 7, 9, 10},           // Dorian
        {0, 1, 3, 5, 7, 8, 10},           // Phrygian
        {0, 2, 4, 6, 7, 9, 11},           // Lydian
        {0, 2, 4, 5, 7, 9, 10},           // Mixolydian
        {0, 2, 3, 5, 7, 8, 10},           // Aeolian
        {0, 1, 3, 5, 6, 8, 10},           // Locrian
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}  // Chromatic
    };

    std::vector<int> scaleNotes;
    auto& intervals = scales[juce::jlimit(0, (int)scales.size() - 1, scaleType)];
    
    for (int octave = -2; octave <= 10; ++octave)
    {
        for (int interval : intervals)
        {
            int note = root + interval + (octave * 12);
            if (note >= 0 && note <= 127)
                scaleNotes.push_back(note);
        }
    }
    
    return scaleNotes;
}

int IonAudioProcessor::findNearestScaleNote(int note, const std::vector<int>& scaleNotes)
{
    int nearest = note;
    int minDist = 128;
    
    for (int scaleNote : scaleNotes)
    {
        int dist = std::abs(note - scaleNote);
        if (dist < minDist)
        {
            minDist = dist;
            nearest = scaleNote;
        }
    }
    
    return nearest;
}

//==============================================================================
const juce::String IonAudioProcessor::getName() const { return JucePlugin_Name; }

bool IonAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool IonAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool IonAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
double IonAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int IonAudioProcessor::getNumPrograms() { return 1; }
int IonAudioProcessor::getCurrentProgram() { return 0; }
void IonAudioProcessor::setCurrentProgram (int) {}
const juce::String IonAudioProcessor::getProgramName (int) { return {}; }
void IonAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void IonAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    notePositions.fill(0.0f);
}

void IonAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool IonAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void IonAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    if (bypass)
        return;

    int root = (int)*parameters.getRawParameterValue(PARAM_ROOT);
    int scaleType = (int)*parameters.getRawParameterValue(PARAM_SCALE);
    float fieldStrength = *parameters.getRawParameterValue(PARAM_FIELD_STRENGTH) / 100.0f;
    float attractionStrength = *parameters.getRawParameterValue(PARAM_ATTRACTION_STRENGTH) / 100.0f;
    int attractionType = (int)*parameters.getRawParameterValue(PARAM_ATTRACTION_TYPE);

    auto scaleNotes = getScaleNotes(root, scaleType);

    juce::MidiBuffer processedMidi;

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        
        if (message.isNoteOn())
        {
            int note = message.getNoteNumber();
            int nearestScaleNote = findNearestScaleNote(note, scaleNotes);
            int distance = std::abs(note - nearestScaleNote);
            
            // Field strength determines range (in semitones)
            int maxFieldRange = (int)(12.0f * fieldStrength); // 0-12 semitones
            
            if (distance <= maxFieldRange || maxFieldRange == 0)
            {
                int outputNote = note;
                
                if (attractionType == 0) // SNAP
                {
                    // Immediate snap to scale note
                    outputNote = nearestScaleNote;
                }
                else if (attractionType == 1) // PULL
                {
                    // Gradual pull toward scale note
                    float pullAmount = attractionStrength;
                    float currentPos = notePositions[note];
                    float targetPos = nearestScaleNote;
                    
                    // Smooth interpolation
                    currentPos += (targetPos - currentPos) * pullAmount * 0.5f;
                    notePositions[note] = currentPos;
                    
                    outputNote = juce::roundToInt(currentPos);
                }
                else if (attractionType == 2) // GUIDE
                {
                    // Subtle nudge toward scale note
                    float nudgeAmount = attractionStrength * 0.3f;
                    float nudge = (nearestScaleNote - note) * nudgeAmount;
                    outputNote = juce::roundToInt(note + nudge);
                }
                
                outputNote = juce::jlimit(0, 127, outputNote);
                
                auto newMessage = juce::MidiMessage::noteOn(message.getChannel(),
                                                            outputNote,
                                                            message.getVelocity());
                processedMidi.addEvent(newMessage, metadata.samplePosition);
            }
            else
            {
                // Outside field range, pass through
                processedMidi.addEvent(message, metadata.samplePosition);
            }
        }
        else
        {
            processedMidi.addEvent(message, metadata.samplePosition);
        }
    }

    midiMessages.swapWith(processedMidi);
}

//==============================================================================
bool IonAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* IonAudioProcessor::createEditor()
{
    return new IonAudioProcessorEditor (*this);
}

//==============================================================================
void IonAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void IonAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IonAudioProcessor();
}
