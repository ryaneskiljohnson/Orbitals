/*
  ==============================================================================

    Eclipse - Velocity Masking
    MIDI FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String EclipseAudioProcessor::PARAM_THRESHOLD = "threshold";
const juce::String EclipseAudioProcessor::PARAM_SHADOW_MIN = "shadowMin";
const juce::String EclipseAudioProcessor::PARAM_SHADOW_MAX = "shadowMax";
const juce::String EclipseAudioProcessor::PARAM_UMBRA = "umbra";
const juce::String EclipseAudioProcessor::PARAM_PENUMBRA = "penumbra";
const juce::String EclipseAudioProcessor::PARAM_ECLIPSE_MODE = "eclipseMode";
const juce::String EclipseAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
EclipseAudioProcessor::EclipseAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Eclipse"), createParameterLayout())
{
}

EclipseAudioProcessor::~EclipseAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout EclipseAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID (PARAM_THRESHOLD, 1),
        "Threshold", 0, 127, 64
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_SHADOW_MIN, 2),
        "Shadow Min",
        juce::NormalisableRange<float> (0.0f, 127.0f, 1.0f), 30.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_SHADOW_MAX, 3),
        "Shadow Max",
        juce::NormalisableRange<float> (0.0f, 127.0f, 1.0f), 90.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_UMBRA, 4),
        "Umbra",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_PENUMBRA, 5),
        "Penumbra",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_ECLIPSE_MODE, 6),
        "Eclipse Mode", false
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 7),
        "Bypass", false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String EclipseAudioProcessor::getName() const { return JucePlugin_Name; }
bool EclipseAudioProcessor::acceptsMidi() const { return true; }
bool EclipseAudioProcessor::producesMidi() const { return true; }
bool EclipseAudioProcessor::isMidiEffect() const { return true; }
double EclipseAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int EclipseAudioProcessor::getNumPrograms() { return 1; }
int EclipseAudioProcessor::getCurrentProgram() { return 0; }
void EclipseAudioProcessor::setCurrentProgram (int) {}
const juce::String EclipseAudioProcessor::getProgramName (int) { return {}; }
void EclipseAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void EclipseAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void EclipseAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EclipseAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void EclipseAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    if (bypass)
        return;

    // Threshold parameter (currently not used in processing, reserved for future use)
    juce::ignoreUnused(*parameters.getRawParameterValue(PARAM_THRESHOLD));
    
    float shadowMin = *parameters.getRawParameterValue(PARAM_SHADOW_MIN);
    float shadowMax = *parameters.getRawParameterValue(PARAM_SHADOW_MAX);
    float umbra = *parameters.getRawParameterValue(PARAM_UMBRA) / 100.0f;
    float penumbra = *parameters.getRawParameterValue(PARAM_PENUMBRA) / 100.0f;
    bool eclipseMode = *parameters.getRawParameterValue(PARAM_ECLIPSE_MODE) > 0.5f; // true = REVEAL, false = HIDE

    juce::MidiBuffer processedMidi;

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        
        if (message.isNoteOn())
        {
            int velocity = message.getVelocity();
            
            // Determine if note is in shadow zone
            bool inShadowZone = (velocity >= shadowMin && velocity <= shadowMax);
            
            if (eclipseMode) // REVEAL mode
            {
                // Only pass through notes in shadow zone
                if (inShadowZone)
                {
                    // Calculate fade based on umbra and penumbra
                    float fadeAmount = 1.0f;
                    
                    // Umbra affects darkness at center of zone
                    float zoneCenter = (shadowMin + shadowMax) / 2.0f;
                    float distanceFromCenter = std::abs(velocity - zoneCenter) / ((shadowMax - shadowMin) / 2.0f);
                    fadeAmount *= (1.0f - umbra * (1.0f - distanceFromCenter));
                    
                    // Penumbra affects edge softness
                    float edgeFade = 1.0f;
                    if (velocity < shadowMin + (shadowMax - shadowMin) * 0.2f)
                    {
                        float edgePos = (velocity - shadowMin) / ((shadowMax - shadowMin) * 0.2f);
                        edgeFade = edgePos * penumbra + (1.0f - penumbra);
                    }
                    else if (velocity > shadowMax - (shadowMax - shadowMin) * 0.2f)
                    {
                        float edgePos = (shadowMax - velocity) / ((shadowMax - shadowMin) * 0.2f);
                        edgeFade = edgePos * penumbra + (1.0f - penumbra);
                    }
                    
                    fadeAmount *= edgeFade;
                    
                    int newVelocity = juce::jlimit(1, 127, (int)(velocity * fadeAmount));
                    auto newMessage = juce::MidiMessage::noteOn(message.getChannel(), 
                                                                message.getNoteNumber(), 
                                                                (juce::uint8)newVelocity);
                    processedMidi.addEvent(newMessage, metadata.samplePosition);
                }
                // Notes outside shadow zone are suppressed
            }
            else // HIDE mode
            {
                // Pass through notes outside shadow zone
                if (!inShadowZone)
                {
                    processedMidi.addEvent(message, metadata.samplePosition);
                }
                else
                {
                    // Notes in shadow zone are suppressed or faded
                    float fadeAmount = 1.0f - (umbra * 0.8f); // Umbra reduces volume
                    
                    if (fadeAmount > 0.1f)
                    {
                        int newVelocity = juce::jlimit(1, 127, (int)(velocity * fadeAmount));
                        auto newMessage = juce::MidiMessage::noteOn(message.getChannel(),
                                                                    message.getNoteNumber(),
                                                                    (juce::uint8)newVelocity);
                        processedMidi.addEvent(newMessage, metadata.samplePosition);
                    }
                }
            }
        }
        else
        {
            // Pass through all non-note-on messages
            processedMidi.addEvent(message, metadata.samplePosition);
        }
    }

    midiMessages.swapWith(processedMidi);
}

//==============================================================================
bool EclipseAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* EclipseAudioProcessor::createEditor()
{
    return new EclipseAudioProcessorEditor (*this);
}

//==============================================================================
void EclipseAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void EclipseAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EclipseAudioProcessor();
}
