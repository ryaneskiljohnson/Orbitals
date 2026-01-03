/*
  ==============================================================================

    Apogee - Velocity & Energy Booster
    MIDI FX Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ApogeeAudioProcessorEditor::ApogeeAudioProcessorEditor (ApogeeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), m_auth_component("apogee")
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

ApogeeAudioProcessorEditor::~ApogeeAudioProcessorEditor()
{
}

//==============================================================================
void ApogeeAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12121a));
}

void ApogeeAudioProcessorEditor::resized()
{
    // Resize auth component to match editor bounds
    m_auth_component.setBounds(getLocalBounds());
    
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void ApogeeAudioProcessorEditor::loadWebUI()
{
    auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals");
    auto htmlFile = projectRoot.getChildFile ("Apogee").getChildFile ("UI").getChildFile ("index.html");

    if (htmlFile.existsAsFile())
    {
        loadHTMLFile(htmlFile);
    }
}

void ApogeeAudioProcessorEditor::loadHTMLFile (const juce::File& htmlFile)
{
    auto htmlContent = htmlFile.loadFileAsString();
    auto uiDir = htmlFile.getParentDirectory();
    auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals");
    auto sharedDir = projectRoot.getChildFile ("_Shared").getChildFile ("UI");

    // Inline CSS files
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

    // Inline JavaScript files
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
        auto animationsContent = animationsFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/orbitals-animations.js\"></script>",
                                           "<script>" + animationsContent + "</script>");
    }

    auto particlesFile = sharedDir.getChildFile ("orbitals-particles.js");
    if (particlesFile.existsAsFile())
    {
        auto particlesContent = particlesFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/orbitals-particles.js\"></script>",
                                           "<script>" + particlesContent + "</script>");
    }

    auto componentsFile = sharedDir.getChildFile ("orbitals-components.js");
    if (componentsFile.existsAsFile())
    {
        auto componentsContent = componentsFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/orbitals-components.js\"></script>",
                                           "<script>" + componentsContent + "</script>");
    }

    // Handle background image
    auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds/apogee-background.png");
    if (backgroundImage.existsAsFile())
    {
        juce::MemoryBlock imageData;
        if (backgroundImage.loadFileAsData(imageData))
        {
            juce::String base64 = juce::Base64::toBase64(imageData.getData(), imageData.getSize());
            juce::String dataURL = "data:image/png;base64," + base64;
            htmlContent = htmlContent.replace("../../_Shared/Assets/backgrounds/apogee-background.png", dataURL);
        }
    }

    // Load HTML using temporary file approach (avoids data URL encoding issues)
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("ApogeeUI_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    auto tempFile = tempDir.getChildFile("index.html");
    tempFile.replaceWithText(htmlContent);
    
    auto filePath = tempFile.getFullPathName().replace(" ", "%20");
    juce::String fileURL = "file://" + filePath;
    webView->goToURL(fileURL);
}

void ApogeeAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
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

        if (param == "lift")
        {
            auto* p = audioProcessor.parameters.getParameter(ApogeeAudioProcessor::PARAM_LIFT);
            if (p != nullptr)
                p->setValueNotifyingHost((float)value / 100.0f);
        }
        else if (param == "curve")
        {
            juce::String curveStr = value.toString();
            int curveIndex = 0;
            if (curveStr == "linear") curveIndex = 0;
            else if (curveStr == "exp") curveIndex = 1;
            else if (curveStr == "s") curveIndex = 2;
            else if (curveStr == "log") curveIndex = 3;
            
            auto* p = audioProcessor.parameters.getParameter(ApogeeAudioProcessor::PARAM_CURVE);
            if (p != nullptr)
                p->setValueNotifyingHost(curveIndex / 3.0f);
        }
        else if (param == "ceiling")
        {
            auto* p = audioProcessor.parameters.getParameter(ApogeeAudioProcessor::PARAM_CEILING);
            if (p != nullptr)
                p->setValueNotifyingHost(((int)value - 1) / 126.0f);
        }
        else if (param == "momentum")
        {
            auto* p = audioProcessor.parameters.getParameter(ApogeeAudioProcessor::PARAM_MOMENTUM);
            if (p != nullptr)
                p->setValueNotifyingHost((float)value / 100.0f);
        }
    }
}
