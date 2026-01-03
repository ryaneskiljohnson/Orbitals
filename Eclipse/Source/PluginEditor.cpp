/*
  ==============================================================================

    Eclipse - Velocity Masking
    MIDI FX Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../../_Shared/Source/OrbitalsPluginEditor.h"

//==============================================================================
EclipseAudioProcessorEditor::EclipseAudioProcessorEditor (EclipseAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), m_auth_component("eclipse")
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

EclipseAudioProcessorEditor::~EclipseAudioProcessorEditor()
{
}

void EclipseAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12121a));
}

void EclipseAudioProcessorEditor::resized()
{
    // Resize auth component to match editor bounds
    m_auth_component.setBounds(getLocalBounds());
    
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void EclipseAudioProcessorEditor::loadWebUI()
{
    auto htmlContent = OrbitalsEditorHelpers::loadPluginHTML("Eclipse");
    if (htmlContent.isNotEmpty())
    {
            // Load HTML using temporary file approach (avoids data URL encoding issues)
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("EclipseUI_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    auto tempFile = tempDir.getChildFile("index.html");
    tempFile.replaceWithText(htmlContent);
    
    auto filePath = tempFile.getFullPathName().replace(" ", "%20");
    juce::String fileURL = "file://" + filePath;
    webView->goToURL(fileURL);
    }
}

void EclipseAudioProcessorEditor::loadHTMLFile (const juce::File& htmlFile)
{
    // Not used - using helper instead
}

void EclipseAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
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

        if (param == "threshold")
        {
            auto* p = audioProcessor.parameters.getParameter(EclipseAudioProcessor::PARAM_THRESHOLD);
            if (p != nullptr)
                p->setValueNotifyingHost((int)value / 127.0f);
        }
        else if (param == "shadowMin")
        {
            auto* p = audioProcessor.parameters.getParameter(EclipseAudioProcessor::PARAM_SHADOW_MIN);
            if (p != nullptr)
                p->setValueNotifyingHost((float)value / 127.0f);
        }
        else if (param == "shadowMax")
        {
            auto* p = audioProcessor.parameters.getParameter(EclipseAudioProcessor::PARAM_SHADOW_MAX);
            if (p != nullptr)
                p->setValueNotifyingHost((float)value / 127.0f);
        }
        else if (param == "umbra")
        {
            auto* p = audioProcessor.parameters.getParameter(EclipseAudioProcessor::PARAM_UMBRA);
            if (p != nullptr)
                p->setValueNotifyingHost((float)value / 100.0f);
        }
        else if (param == "penumbra")
        {
            auto* p = audioProcessor.parameters.getParameter(EclipseAudioProcessor::PARAM_PENUMBRA);
            if (p != nullptr)
                p->setValueNotifyingHost((float)value / 100.0f);
        }
        else if (param == "eclipseMode")
        {
            auto* p = audioProcessor.parameters.getParameter(EclipseAudioProcessor::PARAM_ECLIPSE_MODE);
            if (p != nullptr)
                p->setValueNotifyingHost((bool)value ? 1.0f : 0.0f);
        }
    }
}
