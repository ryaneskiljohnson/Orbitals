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
    : AudioProcessorEditor (&p), audioProcessor (p), m_auth_component("retrograde")
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
    
    // Add authentication component AFTER webview (so it renders on top)
    // Hidden by default, shown if not authorized
    addChildComponent(m_auth_component);
    m_auth_component.setAlwaysOnTop(true);  // Ensure it's always on top

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
    // Resize auth component to match editor bounds
    m_auth_component.setBounds(getLocalBounds());
    
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void RetrogradeAudioProcessorEditor::loadWebUI()
{
    auto htmlContent = OrbitalsEditorHelpers::loadPluginHTML("Retrograde");
    if (htmlContent.isNotEmpty())
    {
            // Load HTML using temporary file approach (avoids data URL encoding issues)
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("RetrogradeUI_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    auto tempFile = tempDir.getChildFile("index.html");
    tempFile.replaceWithText(htmlContent);
    
    auto filePath = tempFile.getFullPathName().replace(" ", "%20");
    juce::String fileURL = "file://" + filePath;
    webView->goToURL(fileURL);
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
