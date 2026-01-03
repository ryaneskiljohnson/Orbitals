/*
  ==============================================================================

    Kepler - MIDI Orbit Generator
    MIDI FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// Parameter IDs
const juce::String KeplerAudioProcessor::PARAM_ORBIT_COUNT = "orbitCount";
const juce::String KeplerAudioProcessor::PARAM_ECCENTRICITY = "eccentricity";
const juce::String KeplerAudioProcessor::PARAM_ROTATION_SPEED = "rotationSpeed";
const juce::String KeplerAudioProcessor::PARAM_STABILITY_X = "stabilityX";
const juce::String KeplerAudioProcessor::PARAM_STABILITY_Y = "stabilityY";
const juce::String KeplerAudioProcessor::PARAM_PERIOD = "period";
const juce::String KeplerAudioProcessor::PARAM_DIRECTION = "direction";
const juce::String KeplerAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
KeplerAudioProcessor::KeplerAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Kepler"), createParameterLayout())
{
}

KeplerAudioProcessor::~KeplerAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout KeplerAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID (PARAM_ORBIT_COUNT, 1),
        "Orbit Count",
        1, 8, 3
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_ECCENTRICITY, 2),
        "Eccentricity",
        juce::NormalisableRange<float> (0.0f, 0.95f, 0.01f),
        0.5f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_ROTATION_SPEED, 3),
        "Rotation Speed",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_STABILITY_X, 4),
        "Stability X",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_STABILITY_Y, 5),
        "Stability Y",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f
    ));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID (PARAM_PERIOD, 6),
        "Period",
        juce::StringArray { "1/8", "1/4", "1/2", "1 bar", "2 bars" },
        2
    ));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID (PARAM_DIRECTION, 7),
        "Direction",
        juce::StringArray { "CW", "CCW", "ALT" },
        0
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        PARAM_BYPASS,
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String KeplerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KeplerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool KeplerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool KeplerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double KeplerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KeplerAudioProcessor::getNumPrograms()
{
    return 1;
}

int KeplerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KeplerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String KeplerAudioProcessor::getProgramName (int index)
{
    return {};
}

void KeplerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void KeplerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    phase = 0.0;
}

void KeplerAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KeplerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void KeplerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    
    if (bypass)
    {
        return;
    }

    int orbitCount = (int)*parameters.getRawParameterValue(PARAM_ORBIT_COUNT);
    float eccentricity = *parameters.getRawParameterValue(PARAM_ECCENTRICITY);
    float rotationSpeed = *parameters.getRawParameterValue(PARAM_ROTATION_SPEED) / 100.0f;
    float stabilityX = *parameters.getRawParameterValue(PARAM_STABILITY_X) / 100.0f;
    float stabilityY = *parameters.getRawParameterValue(PARAM_STABILITY_Y) / 100.0f;
    int periodIndex = (int)*parameters.getRawParameterValue(PARAM_PERIOD);
    int direction = (int)*parameters.getRawParameterValue(PARAM_DIRECTION);
    
    // Get tempo info
    auto playHead = getPlayHead();
    double bpm = 120.0;
    
    if (playHead != nullptr)
    {
        auto positionInfo = playHead->getPosition();
        if (positionInfo.hasValue() && positionInfo->getBpm().hasValue())
        {
            bpm = *positionInfo->getBpm();
        }
    }
    
    double beatDuration = 60.0 / bpm;
    
    // Calculate period in beats
    double periodBeats = 0.5; // Default to 1/2
    switch (periodIndex)
    {
        case 0: periodBeats = 0.125; break; // 1/8
        case 1: periodBeats = 0.25; break;  // 1/4
        case 2: periodBeats = 0.5; break;   // 1/2
        case 3: periodBeats = 1.0; break;   // 1 bar
        case 4: periodBeats = 2.0; break;   // 2 bars
    }
    
    // Update phase
    double phaseIncrement = rotationSpeed * (buffer.getNumSamples() / sampleRate) / (periodBeats * beatDuration);
    phase += phaseIncrement;
    phase = std::fmod(phase, juce::MathConstants<double>::twoPi * orbitCount);
    
    juce::MidiBuffer processedMidi;
    
    int noteIndex = 0;
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        int samplePosition = metadata.samplePosition;
        
        if (message.isNoteOn())
        {
            int velocity = message.getVelocity();
            int noteNumber = message.getNoteNumber();
            int channel = message.getChannel();
            
            // Assign note to an orbit
            int orbitIndex = noteIndex % orbitCount;
            double orbitPhase = phase + (orbitIndex * juce::MathConstants<double>::twoPi / orbitCount);
            
            // Apply direction
            if (direction == 1) // CCW
                orbitPhase = -orbitPhase;
            else if (direction == 2) // ALT
                orbitPhase *= (orbitIndex % 2 == 0) ? 1.0 : -1.0;
            
            // Calculate position on ellipse
            double angle = orbitPhase;
            double radius = 1.0 / (1.0 + eccentricity * std::cos(angle));
            
            // Convert orbital position to timing offset
            // Position on orbit affects timing (Kepler's 2nd law - area sweep)
            double timingOffset = (radius - 0.5) * (1.0f - stabilityX) * 0.1; // Max 100ms
            int offsetSamples = (int)(timingOffset * sampleRate);
            
            // Convert orbital position to velocity variation
            float velocityMultiplier = 0.7f + radius * 0.6f; // 0.7 to 1.3
            velocityMultiplier *= (1.0f + (1.0f - stabilityY) * 0.3f);
            
            int newSamplePosition = juce::jlimit(0, buffer.getNumSamples() - 1, samplePosition + offsetSamples);
            int modifiedVelocity = juce::jlimit(1, 127, (int)(velocity * velocityMultiplier));
            
            auto modifiedMessage = juce::MidiMessage::noteOn(channel, noteNumber, (juce::uint8)modifiedVelocity);
            processedMidi.addEvent(modifiedMessage, newSamplePosition);
            
            noteIndex++;
        }
        else
        {
            processedMidi.addEvent(message, samplePosition);
        }
    }
    
    midiMessages.swapWith(processedMidi);
}

//==============================================================================
bool KeplerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* KeplerAudioProcessor::createEditor()
{
    return new KeplerAudioProcessorEditor (*this);
}

//==============================================================================
void KeplerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void KeplerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KeplerAudioProcessor();
}
