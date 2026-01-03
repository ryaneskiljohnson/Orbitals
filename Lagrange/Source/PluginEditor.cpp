/*
  ==============================================================================

    Lagrange - Timing Stabilizer
    MIDI FX Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LagrangeAudioProcessorEditor::LagrangeAudioProcessorEditor (LagrangeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), m_auth_component("lagrange")
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

LagrangeAudioProcessorEditor::~LagrangeAudioProcessorEditor()
{
}

void LagrangeAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12121a));
}

void LagrangeAudioProcessorEditor::resized()
{
    // Resize auth component to match editor bounds
    m_auth_component.setBounds(getLocalBounds());
    
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void LagrangeAudioProcessorEditor::loadWebUI()
{
    auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals");
    auto htmlFile = projectRoot.getChildFile ("Lagrange").getChildFile ("UI").getChildFile ("index.html");

    if (htmlFile.existsAsFile())
        loadHTMLFile(htmlFile);
}

void LagrangeAudioProcessorEditor::loadHTMLFile (const juce::File& htmlFile)
{
    auto htmlContent = htmlFile.loadFileAsString();
    auto uiDir = htmlFile.getParentDirectory();
    auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals");
    auto sharedDir = projectRoot.getChildFile ("_Shared").getChildFile ("UI");

    // Inline CSS
    auto cssFile = uiDir.getChildFile ("styles.css");
    if (cssFile.existsAsFile())
    {
        auto cssContent = cssFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<link rel=\"stylesheet\" href=\"styles.css\">",
                                           "<style>" + cssContent + "</style>");
    }

    auto designSystemFile = sharedDir.getChildFile ("orbitals-design-system.css");
    if (designSystemFile.existsAsFile())
    {
        auto designSystemContent = designSystemFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<link rel=\"stylesheet\" href=\"../../_Shared/UI/orbitals-design-system.css\">",
                                           "<style>" + designSystemContent + "</style>");
    }

    // Inline JavaScript
    auto jsFile = uiDir.getChildFile ("app.js");
    if (jsFile.existsAsFile())
    {
        auto jsContent = jsFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<script src=\"app.js\"></script>",
                                           "<script>" + jsContent + "</script>");
    }

    auto animationsFile = sharedDir.getChildFile ("orbitals-animations.js");
    if (animationsFile.existsAsFile())
    {
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/orbitals-animations.js\"></script>",
                                           "<script>" + animationsFile.loadFileAsString() + "</script>");
    }

    auto particlesFile = sharedDir.getChildFile ("orbitals-particles.js");
    if (particlesFile.existsAsFile())
    {
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/orbitals-particles.js\"></script>",
                                           "<script>" + particlesFile.loadFileAsString() + "</script>");
    }

    auto componentsFile = sharedDir.getChildFile ("orbitals-components.js");
    if (componentsFile.existsAsFile())
    {
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/orbitals-components.js\"></script>",
                                           "<script>" + componentsFile.loadFileAsString() + "</script>");
    }

    // Handle background image
    auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds/lagrange-background.png");
    if (backgroundImage.existsAsFile())
    {
        juce::MemoryBlock imageData;
        if (backgroundImage.loadFileAsData(imageData))
        {
            juce::String base64 = juce::Base64::toBase64(imageData.getData(), imageData.getSize());
            htmlContent = htmlContent.replace("../../_Shared/Assets/backgrounds/lagrange-background.png", 
                                            "data:image/png;base64," + base64);
        }
    }

        // Load HTML using temporary file approach (avoids data URL encoding issues)
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("LagrangeUI_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    auto tempFile = tempDir.getChildFile("index.html");
    tempFile.replaceWithText(htmlContent);
    
    auto filePath = tempFile.getFullPathName().replace(" ", "%20");
    juce::String fileURL = "file://" + filePath;
    webView->goToURL(fileURL);
}

void LagrangeAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
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
            if (param == "stability" || param == "mass")
                p->setValueNotifyingHost((float)value / 100.0f);
            else if (param == "driftMin" || param == "driftMax")
                p->setValueNotifyingHost((float)value / 100.0f);
            else if (param == "chaosX" || param == "chaosY")
                p->setValueNotifyingHost((float)value / 100.0f);
        }
    }
}
