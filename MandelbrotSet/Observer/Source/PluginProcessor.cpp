/*
  ==============================================================================

    Observer - Wavefunction Collapse Dynamic EQ
    Audio FX Plugin Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::String ObserverAudioProcessor::PARAM_THRESHOLD = "threshold";
const juce::String ObserverAudioProcessor::PARAM_FREQUENCY = "frequency";
const juce::String ObserverAudioProcessor::PARAM_RATIO = "ratio";
const juce::String ObserverAudioProcessor::PARAM_ATTACK = "attack";
const juce::String ObserverAudioProcessor::PARAM_RELEASE = "release";
const juce::String ObserverAudioProcessor::PARAM_BYPASS = "bypass";

//==============================================================================
ObserverAudioProcessor::ObserverAudioProcessor()
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
      parameters (*this, nullptr, juce::Identifier ("Observer"), createParameterLayout())
{
    notePositions.fill(0.0f);
}

ObserverAudioProcessor::~ObserverAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ObserverAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_THRESHOLD, 1),
        "Measurement",
        juce::NormalisableRange<float> (-60.0f, 0.0f, 0.1f),
        -20.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + " dB"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_FREQUENCY, 2),
        "Precision",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f),
        1000.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return value < 1000.0f ? juce::String (value, 0) + " Hz" : juce::String (value / 1000.0f, 1) + "k Hz"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_RATIO, 3),
        "Collapse Ratio",
        juce::NormalisableRange<float> (1.0f, 10.0f, 0.1f),
        2.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + ":1"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_ATTACK, 4),
        "Observation Time",
        juce::NormalisableRange<float> (0.1f, 100.0f, 0.1f),
        10.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value, 1) + " ms"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID (PARAM_RELEASE, 5),
        "Uncertainty",
        juce::NormalisableRange<float> (10.0f, 1000.0f, 1.0f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String ((int)value) + " ms"; }
    ));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID (PARAM_BYPASS, 6),
        "Bypass",
        false
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
std::vector<int> ObserverAudioProcessor::getScaleNotes(int root, int scaleType)
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

int ObserverAudioProcessor::findNearestScaleNote(int note, const std::vector<int>& scaleNotes)
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
const juce::String ObserverAudioProcessor::getName() const { return JucePlugin_Name; }
bool ObserverAudioProcessor::acceptsMidi() const { return true; }
bool ObserverAudioProcessor::producesMidi() const { return true; }
bool ObserverAudioProcessor::isMidiEffect() const { return true; }
double ObserverAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int ObserverAudioProcessor::getNumPrograms() { return 1; }
int ObserverAudioProcessor::getCurrentProgram() { return 0; }
void ObserverAudioProcessor::setCurrentProgram (int) {}
const juce::String ObserverAudioProcessor::getProgramName (int) { return {}; }
void ObserverAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void ObserverAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    notePositions.fill(0.0f);
}

void ObserverAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ObserverAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ObserverAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();

    // Calculate input level for UI - with bounds checking
    float inLevel = 0.0f;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    if (numSamples > 0 && numChannels > 0) {
        for (int channel = 0; channel < totalNumInputChannels && channel < numChannels; ++channel) {
            float channelLevel = buffer.getRMSLevel(channel, 0, numSamples);
            inLevel = std::max(inLevel, channelLevel);
        }
    }
    inputLevel.store(juce::Decibels::gainToDecibels(inLevel, -100.0f));

    bool bypass = *parameters.getRawParameterValue(PARAM_BYPASS);
    if (bypass) {
        outputLevel.store(inputLevel.load());
        return;
    }

    // TODO: Implement dynamic EQ DSP algorithm
    // Parameters available:
    // - PARAM_THRESHOLD (measurement)
    // - PARAM_FREQUENCY (precision)
    // - PARAM_RATIO (collapse ratio)
    // - PARAM_ATTACK (observation time)
    // - PARAM_RELEASE (uncertainty)
    
    float threshold = *parameters.getRawParameterValue(PARAM_THRESHOLD);
    float frequency = *parameters.getRawParameterValue(PARAM_FREQUENCY);
    float ratio = *parameters.getRawParameterValue(PARAM_RATIO);
    float attack = *parameters.getRawParameterValue(PARAM_ATTACK);
    float release = *parameters.getRawParameterValue(PARAM_RELEASE);

    // TODO: Implement dynamic EQ DSP algorithm
    // Process audio buffer with frequency-dependent compression/expansion
    // - threshold: detection threshold in dB (-60 to 0 dB)
    // - frequency: center frequency for the band (20-20000 Hz)
    // - ratio: compression/expansion ratio (1:1 to 10:1)
    // - attack: attack time in ms (0.1-100 ms)
    // - release: release time in ms (10-1000 ms)
    
    juce::ignoreUnused(threshold, frequency, ratio, attack, release);
    
    // Calculate output level for UI - with bounds checking
    float outLevel = 0.0f;
    if (numSamples > 0 && numChannels > 0) {
        for (int channel = 0; channel < totalNumInputChannels && channel < numChannels; ++channel) {
            float channelLevel = buffer.getRMSLevel(channel, 0, numSamples);
            outLevel = std::max(outLevel, channelLevel);
        }
    }
    outputLevel.store(juce::Decibels::gainToDecibels(outLevel, -100.0f));
    
    // For now, pass audio through unchanged
    // MIDI processing removed - this is an Audio FX plugin
}

//==============================================================================
bool ObserverAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ObserverAudioProcessor::createEditor()
{
    return new ObserverAudioProcessorEditor (*this);
}

//==============================================================================
void ObserverAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void ObserverAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ObserverAudioProcessor();
}
