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

bool EclipseAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EclipseAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EclipseAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
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

    float threshold = static_cast<float> (*parameters.getRawParameterValue (PARAM_THRESHOLD));
    float shadowMin = *parameters.getRawParameterValue (PARAM_SHADOW_MIN);
    float shadowMax = *parameters.getRawParameterValue (PARAM_SHADOW_MAX);
    float umbra = *parameters.getRawParameterValue (PARAM_UMBRA) / 100.0f;
    float penumbra = *parameters.getRawParameterValue (PARAM_PENUMBRA) / 100.0f;
    bool eclipseMode = *parameters.getRawParameterValue (PARAM_ECLIPSE_MODE) > 0.5f; // true = REVEAL, false = HIDE

    if (shadowMin > shadowMax)
        std::swap (shadowMin, shadowMax);

    const float zoneHalfWidth = juce::jmax (1.0f, (shadowMax - shadowMin) * 0.5f);
    const float zoneCenter = threshold;
    const float effectiveShadowMin = juce::jlimit (0.0f, 127.0f, zoneCenter - zoneHalfWidth);
    const float effectiveShadowMax = juce::jlimit (0.0f, 127.0f, zoneCenter + zoneHalfWidth);

    juce::MidiBuffer processedMidi;

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        
        if (message.isNoteOn())
        {
            const float velocity = static_cast<float> (message.getVelocity());
            const bool inShadowZone = (velocity >= effectiveShadowMin && velocity <= effectiveShadowMax);
            
            if (eclipseMode) // REVEAL mode
            {
                if (inShadowZone && velocity >= threshold)
                {
                    float fadeAmount = 1.0f;
                    
                    float distanceFromCenter = std::abs (velocity - zoneCenter) / zoneHalfWidth;
                    fadeAmount *= (1.0f - umbra * (1.0f - distanceFromCenter));
                    
                    float edgeFade = 1.0f;
                    const float edgeBand = zoneHalfWidth * 0.2f;
                    if (velocity < effectiveShadowMin + edgeBand)
                    {
                        const float edgePos = (velocity - effectiveShadowMin) / edgeBand;
                        edgeFade = edgePos * penumbra + (1.0f - penumbra);
                    }
                    else if (velocity > effectiveShadowMax - edgeBand)
                    {
                        const float edgePos = (effectiveShadowMax - velocity) / edgeBand;
                        edgeFade = edgePos * penumbra + (1.0f - penumbra);
                    }
                    
                    fadeAmount *= edgeFade;
                    
                    const int newVelocity = juce::jlimit (1, 127, (int) (velocity * fadeAmount));
                    processedMidi.addEvent (juce::MidiMessage::noteOn (message.getChannel(),
                                                                       message.getNoteNumber(),
                                                                       (juce::uint8) newVelocity),
                                            metadata.samplePosition);
                }
            }
            else // HIDE mode
            {
                if (! inShadowZone || velocity < threshold)
                {
                    processedMidi.addEvent (message, metadata.samplePosition);
                }
                else
                {
                    const float fadeAmount = 1.0f - (umbra * 0.8f);
                    
                    if (fadeAmount > 0.1f)
                    {
                        const int newVelocity = juce::jlimit (1, 127, (int) (velocity * fadeAmount));
                        processedMidi.addEvent (juce::MidiMessage::noteOn (message.getChannel(),
                                                                           message.getNoteNumber(),
                                                                           (juce::uint8) newVelocity),
                                                metadata.samplePosition);
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
