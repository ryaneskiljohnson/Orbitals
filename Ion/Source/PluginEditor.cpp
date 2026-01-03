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
    : AudioProcessorEditor (&p), audioProcessor (p), m_auth_component("ion")
{
    // Enable native title bar on the top-level window (for standalone builds)
    if (auto* top_level = juce::TopLevelWindow::getTopLevelWindow(0))
        top_level->setUsingNativeTitleBar(true);

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
    
    // Add authentication component AFTER webview (so it renders on top)
    // Hidden by default, shown if not authorized
    addChildComponent(m_auth_component);
    m_auth_component.setAlwaysOnTop(true);  // Ensure it's always on top

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
    // Resize auth component to match editor bounds
    m_auth_component.setBounds(getLocalBounds());
    
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void IonAudioProcessorEditor::loadWebUI()
{
    auto htmlContent = OrbitalsEditorHelpers::loadPluginHTML("Ion");
    if (htmlContent.isNotEmpty())
    {
            // Load HTML using temporary file approach (avoids data URL encoding issues)
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("IonUI_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    auto tempFile = tempDir.getChildFile("index.html");
    tempFile.replaceWithText(htmlContent);
    
    auto filePath = tempFile.getFullPathName().replace(" ", "%20");
    juce::String fileURL = "file://" + filePath;
    webView->goToURL(fileURL);
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
