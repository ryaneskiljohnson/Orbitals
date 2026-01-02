/*
  ==============================================================================

    Kepler - MIDI Orbit Generator
    MIDI FX Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../../_Shared/Source/OrbitalsPluginEditor.h"

//==============================================================================
KeplerAudioProcessorEditor::KeplerAudioProcessorEditor (KeplerAudioProcessor& p)
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

KeplerAudioProcessorEditor::~KeplerAudioProcessorEditor()
{
}

void KeplerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12121a));
}

void KeplerAudioProcessorEditor::resized()
{
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void KeplerAudioProcessorEditor::loadWebUI()
{
    auto htmlContent = OrbitalsEditorHelpers::loadPluginHTML("Kepler");
    if (htmlContent.isNotEmpty())
    {
        webView->goToURL("data:text/html;charset=utf-8," + juce::URL::addEscapeChars(htmlContent, true));
    }
}

void KeplerAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
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

        auto* p = audioProcessor.parameters.getParameter(param);
        if (p != nullptr)
        {
            // Normalize values based on parameter ranges
            if (param == "orbitCount")
                p->setValueNotifyingHost(((int)value - 1) / 7.0f);
            else if (param == "eccentricity")
                p->setValueNotifyingHost((float)value / 0.95f);
            else if (param == "rotationSpeed" || param == "stabilityX" || param == "stabilityY")
                p->setValueNotifyingHost((float)value / 100.0f);
            else
                p->setValueNotifyingHost((float)value);
        }
    }
}
