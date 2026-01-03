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
    : AudioProcessorEditor (&p), audioProcessor (p), m_auth_component("perihelion")
{
    // Enable native title bar on the top-level window (for standalone builds)
    if (auto* top_level = juce::TopLevelWindow::getTopLevelWindow(0))
        top_level->setUsingNativeTitleBar(true);

    setSize (1200, 750);
    setResizable (false, false);
    
    // Add authentication component (hidden by default, shown if not authorized)
    addChildComponent(m_auth_component);
    m_auth_component.setAlwaysOnTop(true);  // Ensure it\'s always on top

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
    // Resize auth component to match editor bounds
    m_auth_component.setBounds(getLocalBounds());
    
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void PerihelionAudioProcessorEditor::loadWebUI()
{
    auto htmlContent = OrbitalsEditorHelpers::loadPluginHTML("Perihelion");
    if (htmlContent.isNotEmpty())
    {
            // Load HTML using temporary file approach (avoids data URL encoding issues)
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("PerihelionUI_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    auto tempFile = tempDir.getChildFile("index.html");
    tempFile.replaceWithText(htmlContent);
    
    auto filePath = tempFile.getFullPathName().replace(" ", "%20");
    juce::String fileURL = "file://" + filePath;
    webView->goToURL(fileURL);
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
