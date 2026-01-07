/*
  ==============================================================================

    Mandelbrot - Fractal Phaser
    Audio FX Plugin Processor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <deque>

//==============================================================================
/**
    Mandelbrot Audio FX Processor
    Phaser with fractal mathematics metaphors
*/
class MandelbrotAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MandelbrotAudioProcessor();
    ~MandelbrotAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter IDs
    static const juce::String PARAM_RATE;
    static const juce::String PARAM_DEPTH;
    static const juce::String PARAM_FEEDBACK;
    static const juce::String PARAM_STAGES;
    static const juce::String PARAM_MIX;
    static const juce::String PARAM_BYPASS;

    // Get parameters
    juce::AudioProcessorValueTreeState parameters;
    
    // Audio level tracking for UI
    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Phaser DSP
    static constexpr int maxStages = 12;
    std::array<juce::dsp::IIR::Filter<float>, maxStages> allPassFiltersL;
    std::array<juce::dsp::IIR::Filter<float>, maxStages> allPassFiltersR;
    std::array<juce::dsp::IIR::Coefficients<float>::Ptr, maxStages> allPassCoeffsL;
    std::array<juce::dsp::IIR::Coefficients<float>::Ptr, maxStages> allPassCoeffsR;
    double sampleRate = 44100.0;
    double lfoPhaseL = 0.0;
    double lfoPhaseR = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MandelbrotAudioProcessor)
};
