/*
  ==============================================================================

    Ion - Harmonic Constraint Engine
    MIDI FX Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../../_Shared/Source/OrbitalsPluginEditor.h"

//==============================================================================
IonAudioProcessorEditor::IonAudioProcessorEditor (IonAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (1200, 750);
    setResizable (false, false);

    auto options = juce::WebBrowserComponent::Options{}
        .withNativeIntegrationEnabled (true)
        .withEventListener ("message", [this](const juce::var& message) {
            handleJavaScriptMessage (message);
        });

    webView = std::make_unique<juce::WebBrowserComponent> (options);
    webView->setOpaque (false);
    addAndMakeVisible (webView.get());
    webView->setBounds (getLocalBounds());

    loadWebUI();
}

IonAudioProcessorEditor::~IonAudioProcessorEditor()
{
}

void IonAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12121a));
}

void IonAudioProcessorEditor::resized()
{
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void IonAudioProcessorEditor::loadWebUI()
{
    auto htmlContent = OrbitalsEditorHelpers::loadPluginHTML("Ion");
    if (htmlContent.isNotEmpty())
    {
        webView->goToURL("data:text/html;charset=utf-8," + juce::URL::addEscapeChars(htmlContent, true));
    }
}

void IonAudioProcessorEditor::loadHTMLFile (const juce::File& htmlFile)
{
    // Not used - using helper instead
}

void IonAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
{
    if (!message.isObject())
        return;

    auto obj = message.getDynamicObject();
    if (obj == nullptr)
        return;

    auto type = obj->getProperty("type").toString();
    
    if (type == "parameterChange")
    {
        auto param = obj->getProperty("parameter").toString();
        auto value = obj->getProperty("value");

        if (param == "root")
        {
            auto* p = audioProcessor.parameters.getParameter(IonAudioProcessor::PARAM_ROOT);
            if (p != nullptr)
                p->setValueNotifyingHost((int)value / 11.0f);
        }
        else if (param == "scale")
        {
            juce::String scaleStr = value.toString();
            int scaleIndex = 0;
            const char* scales[] = {"major", "minor", "dorian", "phrygian", "lydian", "mixolydian", "aeolian", "locrian", "chromatic"};
            for (int i = 0; i < 9; i++)
            {
                if (scaleStr == scales[i])
                {
                    scaleIndex = i;
                    break;
                }
            }
            
            auto* p = audioProcessor.parameters.getParameter(IonAudioProcessor::PARAM_SCALE);
            if (p != nullptr)
                p->setValueNotifyingHost(scaleIndex / 8.0f);
        }
        else if (param == "fieldStrength")
        {
            auto* p = audioProcessor.parameters.getParameter(IonAudioProcessor::PARAM_FIELD_STRENGTH);
            if (p != nullptr)
                p->setValueNotifyingHost((float)value / 100.0f);
        }
        else if (param == "attractionStrength")
        {
            auto* p = audioProcessor.parameters.getParameter(IonAudioProcessor::PARAM_ATTRACTION_STRENGTH);
            if (p != nullptr)
                p->setValueNotifyingHost((float)value / 100.0f);
        }
        else if (param == "attractionType")
        {
            juce::String typeStr = value.toString();
            int typeIndex = 0;
            if (typeStr == "snap") typeIndex = 0;
            else if (typeStr == "pull") typeIndex = 1;
            else if (typeStr == "guide") typeIndex = 2;
            
            auto* p = audioProcessor.parameters.getParameter(IonAudioProcessor::PARAM_ATTRACTION_TYPE);
            if (p != nullptr)
                p->setValueNotifyingHost(typeIndex / 2.0f);
        }
    }
}
