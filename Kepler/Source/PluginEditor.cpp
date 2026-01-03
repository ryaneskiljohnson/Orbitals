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
    : AudioProcessorEditor (&p), audioProcessor (p), m_auth_component("kepler")
{
    // Enable native title bar on the top-level window (for standalone builds)
    if (auto* top_level = juce::TopLevelWindow::getTopLevelWindow(0))
        top_level->setUsingNativeTitleBar(true);

    setSize (1200, 750);
    setResizable (false, false);
    
    // Add authentication component (hidden by default, shown if not authorized)
    addChildComponent(m_auth_component);

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
    // Resize auth component to match editor bounds
    m_auth_component.setBounds(getLocalBounds());
    
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void KeplerAudioProcessorEditor::loadWebUI()
{
    auto htmlContent = OrbitalsEditorHelpers::loadPluginHTML("Kepler");
    if (htmlContent.isNotEmpty())
    {
            // Load HTML using temporary file approach (avoids data URL encoding issues)
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("KeplerUI_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    auto tempFile = tempDir.getChildFile("index.html");
    tempFile.replaceWithText(htmlContent);
    
    auto filePath = tempFile.getFullPathName().replace(" ", "%20");
    juce::String fileURL = "file://" + filePath;
    webView->goToURL(fileURL);
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
