/*
  ==============================================================================

    Lagrange - Timing Stabilizer
    MIDI FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String LagrangeAudioProcessor::PARAM_STABILITY = "stability";
const juce::String LagrangeAudioProcessor::PARAM_MASS = "mass";
const juce::String LagrangeAudioProcessor::PARAM_DRIFT_MIN = "driftMin";
const juce::String LagrangeAudioProcessor::PARAM_DRIFT_MAX = "driftMax";
const juce::String LagrangeAudioProcessor::PARAM_CHAOS_X = "chaosX";
const juce::String LagrangeAudioProcessor::PARAM_CHAOS_Y = "chaosY";
const juce::String LagrangeAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
LagrangeAudioProcessor::LagrangeAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Lagrange"), createParameterLayout())
{
}

LagrangeAudioProcessor::~LagrangeAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout LagrangeAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_STABILITY, 1),
        "Stability",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_MASS, 2),
        "Mass",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_DRIFT_MIN, 3),
        "Drift Min",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 20.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_DRIFT_MAX, 4),
        "Drift Max",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 80.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_CHAOS_X, 5),
        "Chaos X",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_CHAOS_Y, 6),
        "Chaos Y",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 7),
        "Bypass", false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String LagrangeAudioProcessor::getName() const { return JucePlugin_Name; }

bool LagrangeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LagrangeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LagrangeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}
double LagrangeAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int LagrangeAudioProcessor::getNumPrograms() { return 1; }
int LagrangeAudioProcessor::getCurrentProgram() { return 0; }
void LagrangeAudioProcessor::setCurrentProgram (int) {}
const juce::String LagrangeAudioProcessor::getProgramName (int) { return {}; }
void LagrangeAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void LagrangeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
}

void LagrangeAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LagrangeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void LagrangeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    if (bypass)
        return;

    float stability = *parameters.getRawParameterValue(PARAM_STABILITY) / 100.0f;
    float mass = *parameters.getRawParameterValue(PARAM_MASS) / 100.0f;
    float driftMin = *parameters.getRawParameterValue(PARAM_DRIFT_MIN);
    float driftMax = *parameters.getRawParameterValue(PARAM_DRIFT_MAX);
    float chaosX = *parameters.getRawParameterValue(PARAM_CHAOS_X) / 100.0f;
    float chaosY = *parameters.getRawParameterValue(PARAM_CHAOS_Y) / 100.0f;

    double blockStartTimeSec = 0.0;
    double beatsPerSecond = 120.0 / 60.0;

    if (auto* playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition(); positionInfo.hasValue())
        {
            if (positionInfo->getTimeInSeconds().hasValue())
                blockStartTimeSec = *positionInfo->getTimeInSeconds();
            else if (positionInfo->getPpqPosition().hasValue() && positionInfo->getBpm().hasValue())
                blockStartTimeSec = (*positionInfo->getPpqPosition() / *positionInfo->getBpm()) * 60.0;

            if (positionInfo->getBpm().hasValue())
                beatsPerSecond = *positionInfo->getBpm() / 60.0;
        }
    }

    const double sixteenthsPerSecond = beatsPerSecond * 4.0;
    juce::MidiBuffer processedMidi;

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        int samplePosition = metadata.samplePosition;
        
        if (message.isNoteOn() || message.isNoteOff())
        {
            const double currentTimeSec = blockStartTimeSec + (samplePosition / sampleRate);
            const double nearestGrid = std::round (currentTimeSec * sixteenthsPerSecond) / sixteenthsPerSecond;
            
            double deviation = currentTimeSec - nearestGrid;
            double pullStrength = stability * mass;
            double correctedTimeSec = currentTimeSec - (deviation * pullStrength);
            
            double driftMinMs = (driftMin / 100.0) * 50.0;
            double driftMaxMs = (driftMax / 100.0) * 50.0;
            double driftRangeMs = driftMaxMs - driftMinMs;
            double maxDeviationMs = driftRangeMs / 2.0;
            
            double deviationMs = (correctedTimeSec - nearestGrid) * 1000.0;
            deviationMs = juce::jlimit (-maxDeviationMs, maxDeviationMs, deviationMs);
            correctedTimeSec = nearestGrid + (deviationMs / 1000.0);
            
            double timingChaos = (random.nextFloat() - 0.5f) * chaosX * 0.01;
            correctedTimeSec += timingChaos;
            
            int correctedSample = (int) std::lround ((correctedTimeSec - blockStartTimeSec) * sampleRate);
            correctedSample = juce::jlimit (0, buffer.getNumSamples() - 1, correctedSample);
            
            if (message.isNoteOn())
            {
                float velocityChaos = (random.nextFloat() - 0.5f) * chaosY * 0.3f;
                float velocity = message.getFloatVelocity();
                velocity = juce::jlimit(0.0f, 1.0f, velocity + velocityChaos);
                
                auto newMessage = juce::MidiMessage::noteOn(message.getChannel(), 
                                                            message.getNoteNumber(), 
                                                            velocity);
                processedMidi.addEvent(newMessage, correctedSample);
            }
            else
            {
                processedMidi.addEvent(message, correctedSample);
            }
        }
        else
        {
            processedMidi.addEvent(message, samplePosition);
        }
    }

    midiMessages.swapWith(processedMidi);
}

//==============================================================================
bool LagrangeAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* LagrangeAudioProcessor::createEditor()
{
    return new LagrangeAudioProcessorEditor (*this);
}

//==============================================================================
void LagrangeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void LagrangeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LagrangeAudioProcessor();
}
