/*
  ==============================================================================

    Retrograde - MIDI Motion Reversal
    MIDI FX Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../../_Shared/Source/OrbitalsPluginEditor.h"

//==============================================================================
RetrogradeAudioProcessorEditor::RetrogradeAudioProcessorEditor (RetrogradeAudioProcessor& p)
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

RetrogradeAudioProcessorEditor::~RetrogradeAudioProcessorEditor()
{
}

void RetrogradeAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12121a));
}

void RetrogradeAudioProcessorEditor::resized()
{
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void RetrogradeAudioProcessorEditor::loadWebUI()
{
    auto htmlContent = OrbitalsEditorHelpers::loadPluginHTML("Retrograde");
    if (htmlContent.isNotEmpty())
    {
        webView->goToURL("data:text/html;charset=utf-8," + juce::URL::addEscapeChars(htmlContent, true));
    }
}

void RetrogradeAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
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
            // Handle different parameter types appropriately
            if (param == "mode" || param == "period" || param == "direction")
            {
                // Choice parameters need string-to-index conversion
                // For now, assume value is already the index
                p->setValueNotifyingHost((float)value);
            }
            else if (param == "scope" || param == "echo")
            {
                p->setValueNotifyingHost((float)value);
            }
            else if (param == "symmetry")
            {
                p->setValueNotifyingHost((float)value / 360.0f);
            }
        }
    }
}
