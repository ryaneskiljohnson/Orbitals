/*
  ==============================================================================

    PixelPulse - Retro Rhythm Game
    Video Game Music Generator Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// Parameter IDs
const juce::String PixelPulseAudioProcessor::PARAM_TEMPO = "tempo";
const juce::String PixelPulseAudioProcessor::PARAM_SCALE = "scale";
const juce::String PixelPulseAudioProcessor::PARAM_DIFFICULTY = "difficulty";
const juce::String PixelPulseAudioProcessor::PARAM_VOLUME = "volume";
const juce::String PixelPulseAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
PixelPulseAudioProcessor::PixelPulseAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("PixelPulse"), createParameterLayout())
{
}

PixelPulseAudioProcessor::~PixelPulseAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PixelPulseAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_TEMPO, 1),
        "Tempo",
        juce::NormalisableRange<float> (60.0f, 200.0f, 1.0f),
        120.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + " BPM"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID (PARAM_SCALE, 2),
        "Scale",
        0, 5, 0,
        juce::String(),
        [](int value, int) {
            const char* scales[] = { "Major", "Minor", "Pentatonic", "Blues", "Dorian", "Mixolydian" };
            return juce::String (scales[value]);
        }
    ));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID (PARAM_DIFFICULTY, 3),
        "Difficulty",
        0, 3, 1,
        juce::String(),
        [](int value, int) {
            const char* diffs[] = { "Easy", "Normal", "Hard", "Expert" };
            return juce::String (diffs[value]);
        }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_VOLUME, 4),
        "Volume",
        juce::NormalisableRange<float> (-60.0f, 12.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + " dB"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 5),
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String PixelPulseAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PixelPulseAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PixelPulseAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PixelPulseAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PixelPulseAudioProcessor::getTailLengthSeconds() const
{
    return 0.0; // Gate has no tail
}

int PixelPulseAudioProcessor::getNumPrograms()
{
    return 1;
}

int PixelPulseAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PixelPulseAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PixelPulseAudioProcessor::getProgramName (int index)
{
    return {};
}

void PixelPulseAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PixelPulseAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    gameTime = 0.0;
    beatInterval = (60.0 / 120.0) * sampleRate; // Default 120 BPM
}

void PixelPulseAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PixelPulseAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Accept mono or stereo input
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // Output must be stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return true;
  #endif
}
#endif

void PixelPulseAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Calculate input level for UI
    float inLevel = 0.0f;
    if (numSamples > 0 && numChannels > 0) {
        for (int channel = 0; channel < totalNumInputChannels && channel < numChannels; ++channel) {
            float channelLevel = buffer.getRMSLevel(channel, 0, numSamples);
            inLevel = std::max(inLevel, channelLevel);
        }
    }
    inputLevel.store(juce::Decibels::gainToDecibels(inLevel, -100.0f));

    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    gameRunning.store(!bypass);
    
    if (bypass)
    {
        outputLevel.store(inputLevel.load());
        return;
    }

    // Get parameters
    float tempo = *parameters.getRawParameterValue(PARAM_TEMPO);
    float volumeDb = *parameters.getRawParameterValue(PARAM_VOLUME);
    float volumeGain = juce::Decibels::decibelsToGain(volumeDb);
    
    // Update beat interval
    beatInterval = (60.0 / tempo) * sampleRate;
    
    // Update game time
    gameTime += numSamples;
    
    // For now, pass audio through with volume control
    // TODO: Generate MIDI notes based on game events
    buffer.applyGain(volumeGain);
    
    // Calculate output level for UI
    float outLevel = 0.0f;
    if (numSamples > 0 && numChannels > 0) {
        for (int channel = 0; channel < totalNumInputChannels && channel < numChannels; ++channel) {
            float channelLevel = buffer.getRMSLevel(channel, 0, numSamples);
            outLevel = std::max(outLevel, channelLevel);
        }
    }
    outputLevel.store(juce::Decibels::gainToDecibels(outLevel, -100.0f));
}

//==============================================================================
bool PixelPulseAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PixelPulseAudioProcessor::createEditor()
{
    return new PixelPulseAudioProcessorEditor (*this);
}

//==============================================================================
void PixelPulseAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PixelPulseAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PixelPulseAudioProcessor();
}
