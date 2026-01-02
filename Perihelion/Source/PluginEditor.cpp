/*
  ==============================================================================

    Perihelion - Velocity Sculptor
    MIDI FX Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../../_Shared/Source/OrbitalsPluginEditor.h"

//==============================================================================
PerihelionAudioProcessorEditor::PerihelionAudioProcessorEditor (PerihelionAudioProcessor& p)
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

PerihelionAudioProcessorEditor::~PerihelionAudioProcessorEditor()
{
}

void PerihelionAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12121a));
}

void PerihelionAudioProcessorEditor::resized()
{
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void PerihelionAudioProcessorEditor::loadWebUI()
{
    auto htmlContent = OrbitalsEditorHelpers::loadPluginHTML("Perihelion");
    if (htmlContent.isNotEmpty())
    {
        webView->goToURL("data:text/html;charset=utf-8," + juce::URL::addEscapeChars(htmlContent, true));
    }
}

void PerihelionAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
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

        if (param == "gravity")
        {
            auto* p = audioProcessor.parameters.getParameter(PerihelionAudioProcessor::PARAM_GRAVITY);
            if (p != nullptr)
                p->setValueNotifyingHost((float)value / 100.0f);
        }
        else if (param == "orbit")
        {
            auto* p = audioProcessor.parameters.getParameter(PerihelionAudioProcessor::PARAM_ORBIT);
            if (p != nullptr)
                p->setValueNotifyingHost((float)value / 100.0f);
        }
        else if (param == "solarPoint")
        {
            auto* p = audioProcessor.parameters.getParameter(PerihelionAudioProcessor::PARAM_SOLAR_POINT);
            if (p != nullptr)
                p->setValueNotifyingHost(((int)value - 1) / 126.0f);
        }
        else if (param == "bias")
        {
            auto* p = audioProcessor.parameters.getParameter(PerihelionAudioProcessor::PARAM_BIAS);
            if (p != nullptr)
                p->setValueNotifyingHost(((float)value + 100.0f) / 200.0f);
        }
    }
}
