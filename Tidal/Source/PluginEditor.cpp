/*
  ==============================================================================

    Tidal - Rhythmic Push & Pull
    MIDI FX Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TidalAudioProcessorEditor::TidalAudioProcessorEditor (TidalAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Enable native title bar on the top-level window (for standalone builds)
    if (auto* top_level = juce::TopLevelWindow::getTopLevelWindow(0))
        top_level->setUsingNativeTitleBar(true);

    setSize (1200, 750);
    setResizable (false, false);

    // Create WebView with native integration enabled for message passing
    auto options = juce::WebBrowserComponent::Options{}
        .withNativeIntegrationEnabled (true)
        .withEventListener ("message", [this](const juce::var& message) {
            handleJavaScriptMessage (message);
        });

    webView = std::make_unique<juce::WebBrowserComponent> (options);
    webView->setOpaque (false);
    addAndMakeVisible (webView.get());
    webView->setBounds (getLocalBounds());

    // Load UI
    loadWebUI();
}

TidalAudioProcessorEditor::~TidalAudioProcessorEditor()
{
}

//==============================================================================
void TidalAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12121a)); // Deep space background
}

void TidalAudioProcessorEditor::resized()
{
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void TidalAudioProcessorEditor::loadWebUI()
{
    // Find UI files relative to plugin binary
    auto htmlFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
        .getParentDirectory()
        .getChildFile("Resources")
        .getChildFile("index.html");

    // Fallback: try development path
    if (!htmlFile.existsAsFile())
    {
        auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals");
        htmlFile = projectRoot.getChildFile ("Tidal").getChildFile ("UI").getChildFile ("index.html");
    }

    if (htmlFile.existsAsFile())
    {
        loadHTMLFile(htmlFile);
    }
    else
    {
        DBG ("Could not find index.html");
    }
}

void TidalAudioProcessorEditor::loadHTMLFile (const juce::File& htmlFile)
{
    auto htmlContent = htmlFile.loadFileAsString();
    auto uiDir = htmlFile.getParentDirectory();

    // Inline CSS
    auto cssFile = uiDir.getChildFile ("styles.css");
    if (cssFile.existsAsFile())
    {
        auto cssContent = cssFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<link rel=\"stylesheet\" href=\"styles.css\">",
                                           "<style>" + cssContent + "</style>");
    }

    // Inline shared design system
    auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals");
    auto sharedDir = projectRoot.getChildFile ("_Shared").getChildFile ("UI");
    
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

    // Shared JS
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
    auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds/tidal-background.png");
    if (backgroundImage.existsAsFile())
    {
        // Create data URL for the image
        juce::MemoryBlock imageData;
        if (backgroundImage.loadFileAsData(imageData))
        {
            juce::String base64 = juce::Base64::toBase64(imageData.getData(), imageData.getSize());
            juce::String dataURL = "data:image/png;base64," + base64;
            htmlContent = htmlContent.replace("../../_Shared/Assets/backgrounds/tidal-background.png", dataURL);
        }
    }

    // Load HTML using temporary file approach (avoids data URL encoding issues)
    // Create temporary directory for UI files
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("TidalUI_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    // Write HTML to temp file
    auto tempFile = tempDir.getChildFile("index.html");
    tempFile.replaceWithText(htmlContent);
    
    // Copy any referenced assets if needed (background images already inlined as base64)
    
    // Load via file:// URL
    auto filePath = tempFile.getFullPathName().replace(" ", "%20");
    juce::String fileURL = "file://" + filePath;
    webView->goToURL(fileURL);
}

void TidalAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
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

        if (param == "amplitude")
        {
            auto* p = audioProcessor.parameters.getParameter(TidalAudioProcessor::PARAM_AMPLITUDE);
            if (p != nullptr)
                p->setValueNotifyingHost((float)value / 100.0f);
        }
        else if (param == "phase")
        {
            auto* p = audioProcessor.parameters.getParameter(TidalAudioProcessor::PARAM_PHASE);
            if (p != nullptr)
                p->setValueNotifyingHost((float)value / 360.0f);
        }
        else if (param == "rate")
        {
            juce::String rateStr = value.toString();
            int rateIndex = 0;
            if (rateStr == "1/4") rateIndex = 0;
            else if (rateStr == "1/8") rateIndex = 1;
            else if (rateStr == "1/16") rateIndex = 2;
            else if (rateStr == "1/32") rateIndex = 3;
            
            auto* p = audioProcessor.parameters.getParameter(TidalAudioProcessor::PARAM_RATE);
            if (p != nullptr)
                p->setValueNotifyingHost(rateIndex / 3.0f);
        }
        else if (param == "shape")
        {
            juce::String shapeStr = value.toString();
            int shapeIndex = 0;
            if (shapeStr == "sine") shapeIndex = 0;
            else if (shapeStr == "triangle") shapeIndex = 1;
            else if (shapeStr == "saw") shapeIndex = 2;
            else if (shapeStr == "square") shapeIndex = 3;
            
            auto* p = audioProcessor.parameters.getParameter(TidalAudioProcessor::PARAM_SHAPE);
            if (p != nullptr)
                p->setValueNotifyingHost(shapeIndex / 3.0f);
        }
    }
}
