/*
  ==============================================================================

    Perihelion - Velocity Sculptor
    MIDI FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String PerihelionAudioProcessor::PARAM_GRAVITY = "gravity";
const juce::String PerihelionAudioProcessor::PARAM_ORBIT = "orbit";
const juce::String PerihelionAudioProcessor::PARAM_SOLAR_POINT = "solarPoint";
const juce::String PerihelionAudioProcessor::PARAM_BIAS = "bias";
const juce::String PerihelionAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
PerihelionAudioProcessor::PerihelionAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Perihelion"), createParameterLayout())
{
}

PerihelionAudioProcessor::~PerihelionAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PerihelionAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        PARAM_GRAVITY, "Gravity",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        PARAM_ORBIT, "Orbit",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        PARAM_SOLAR_POINT, "Solar Point", 1, 127, 64
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        PARAM_BIAS, "Bias",
        juce::NormalisableRange<float> (-100.0f, 100.0f, 0.1f), 0.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        PARAM_BYPASS, "Bypass", false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String PerihelionAudioProcessor::getName() const { return JucePlugin_Name; }
bool PerihelionAudioProcessor::acceptsMidi() const { return true; }
bool PerihelionAudioProcessor::producesMidi() const { return true; }
bool PerihelionAudioProcessor::isMidiEffect() const { return true; }
double PerihelionAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int PerihelionAudioProcessor::getNumPrograms() { return 1; }
int PerihelionAudioProcessor::getCurrentProgram() { return 0; }
void PerihelionAudioProcessor::setCurrentProgram (int) {}
const juce::String PerihelionAudioProcessor::getProgramName (int) { return {}; }
void PerihelionAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void PerihelionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void PerihelionAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PerihelionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void PerihelionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    if (bypass)
        return;

    float gravity = *parameters.getRawParameterValue(PARAM_GRAVITY) / 100.0f;
    float orbit = *parameters.getRawParameterValue(PARAM_ORBIT) / 100.0f;
    int solarPoint = (int)*parameters.getRawParameterValue(PARAM_SOLAR_POINT);
    float bias = *parameters.getRawParameterValue(PARAM_BIAS) / 100.0f;

    juce::MidiBuffer processedMidi;

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        
        if (message.isNoteOn())
        {
            int velocity = message.getVelocity();
            
            // Calculate distance from solar point
            float distance = (float)(velocity - solarPoint) / 127.0f;
            
            // Apply bias (inner/outer orbit preference)
            float biasedDistance = distance + (bias * 0.3f);
            
            // Calculate orbital eccentricity based on orbit parameter
            float eccentricity = orbit;
            
            // Apply gravitational pull toward solar point
            float pull = gravity * (1.0f - eccentricity);
            float newVelocity = velocity - (biasedDistance * 127.0f * pull);
            
            // Apply orbital shaping (elliptical path)
            float orbitalModulation = std::sin(distance * juce::MathConstants<float>::pi) * eccentricity * 30.0f;
            newVelocity += orbitalModulation;
            
            int finalVelocity = juce::jlimit(1, 127, (int)newVelocity);
            
            auto newMessage = juce::MidiMessage::noteOn(message.getChannel(),
                                                        message.getNoteNumber(),
                                                        (juce::uint8)finalVelocity);
            processedMidi.addEvent(newMessage, metadata.samplePosition);
        }
        else
        {
            processedMidi.addEvent(message, metadata.samplePosition);
        }
    }

    midiMessages.swapWith(processedMidi);
}

//==============================================================================
bool PerihelionAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* PerihelionAudioProcessor::createEditor()
{
    return new PerihelionAudioProcessorEditor (*this);
}

//==============================================================================
void PerihelionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PerihelionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PerihelionAudioProcessor();
}
