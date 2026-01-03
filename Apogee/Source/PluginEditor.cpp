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
    
    // Check authorization first, then load appropriate UI
    isAuthorized = checkAuthorization();
    
    if (isAuthorized)
    {
        loadWebUI();
        startTimer(1000 * 60 * 15); // Re-check every 15 minutes
    }
    else
    {
        loadAuthScreen();
        startTimer(5000); // Re-check every 5 seconds
    }
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
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void ApogeeAudioProcessorEditor::loadWebUI()
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
        htmlFile = projectRoot.getChildFile ("Apogee").getChildFile ("UI").getChildFile ("index.html");
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

    // Disable right-click context menu
    juce::String disableRightClickScript = R"(<script>
        document.addEventListener('contextmenu', function(e) { e.preventDefault(); return false; });
        document.addEventListener('selectstart', function(e) { e.preventDefault(); return false; });
    </script>)";
    
    // Inject script before closing body tag
    if (htmlContent.contains("</body>"))
        htmlContent = htmlContent.replace("</body>", disableRightClickScript + "</body>");
    else if (htmlContent.contains("</html>"))
        htmlContent = htmlContent.replace("</html>", disableRightClickScript + "</html>");
    else
        htmlContent += disableRightClickScript;
    
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

//==============================================================================
void ApogeeAudioProcessorEditor::loadAuthScreen()
{
    // Create auth HTML content with background image
    juce::String authHTML = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Apogee - Authentication Required</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        html, body {
            width: 1200px; height: 750px; overflow: hidden;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
            background: #0a0a0f; color: #e8e8f0;
            position: relative;
        }
        body::before {
            content: '';
            position: absolute;
            top: 0; left: 0; width: 100%; height: 100%;
            background-image: url('PLACEHOLDER_BACKGROUND');
            background-size: cover;
            background-position: center;
            background-repeat: no-repeat;
            opacity: 0.6;
            z-index: 0;
        }
        .auth-container {
            position: relative; z-index: 1;
            text-align: center; padding: 40px;
            display: flex; flex-direction: column;
            align-items: center; justify-content: center;
            height: 100%; width: 100%;
        }
        .logo-container {
            position: relative; z-index: 2;
            margin-bottom: 40px;
        }
        .logo-container img {
            max-width: 300px; height: auto;
            filter: drop-shadow(0 0 10px rgba(194, 107, 44, 0.5));
        }
        .text-content {
            position: relative; z-index: 2;
            background: rgba(10, 10, 15, 0.85);
            padding: 50px 60px;
            border-radius: 20px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);
            backdrop-filter: blur(10px);
            max-width: 700px;
        }
        .auth-title {
            font-size: 48px; font-weight: bold; color: #00d4ff;
            margin-bottom: 30px; text-shadow: 0 0 20px #00d4ff;
            font-family: 'Orbitron', monospace; letter-spacing: 4px;
        }
        .auth-message {
            font-size: 20px; line-height: 1.8;
            margin: 0 auto 20px; color: #e8e8f0;
        }
        .auth-status {
            margin-top: 30px; font-size: 14px; color: #888;
        }
    </style>
</head>
<body>
    <div class="auth-container">
        <div class="logo-container">
            <img src="PLACEHOLDER_LOGO" alt="NNAudio Logo">
        </div>
        <div class="text-content">
            <h1 class="auth-title">APOGEE</h1>
            <p class="auth-message">
                Your plugin's authentication needs to be refreshed.<br><br>
                Please launch the NNAudio Access app to continue.
            </p>
            <p class="auth-status">Checking license...</p>
        </div>
    </div>
    <script>
        document.addEventListener('contextmenu', function(e) { e.preventDefault(); return false; });
        document.addEventListener('selectstart', function(e) { e.preventDefault(); return false; });
    </script>
</body>
</html>)";
    
    // Load and inline background image as base64
    auto projectRoot = juce::File("/Users/rjmacbookpro/Development/Orbitals");
    auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds/apogee-background.png");
    
    if (backgroundImage.existsAsFile())
    {
        juce::MemoryBlock imageData;
        if (backgroundImage.loadFileAsData(imageData))
        {
            juce::String base64 = juce::Base64::toBase64(imageData.getData(), imageData.getSize());
            juce::String dataURL = "data:image/png;base64," + base64;
            authHTML = authHTML.replace("PLACEHOLDER_BACKGROUND", dataURL);
        }
    }
    else
    {
        // Fallback if image not found
        authHTML = authHTML.replace("PLACEHOLDER_BACKGROUND", "none");
    }
    
    // Load and inline NNAudio logo as base64
    auto logoImage = projectRoot.getChildFile("_Shared/Assets/logos/nnaudio-logo.png");
    
    if (logoImage.existsAsFile())
    {
        juce::MemoryBlock logoData;
        if (logoImage.loadFileAsData(logoData))
        {
            juce::String logoBase64 = juce::Base64::toBase64(logoData.getData(), logoData.getSize());
            juce::String logoDataURL = "data:image/png;base64," + logoBase64;
            authHTML = authHTML.replace("PLACEHOLDER_LOGO", logoDataURL);
        }
    }
    else
    {
        // Fallback if logo not found
        authHTML = authHTML.replace("PLACEHOLDER_LOGO", "");
    }
    
    // Load auth HTML using temporary file approach
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("ApogeeAuth_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    auto tempFile = tempDir.getChildFile("auth.html");
    tempFile.replaceWithText(authHTML);
    
    auto filePath = tempFile.getFullPathName().replace(" ", "%20");
    juce::String fileURL = "file://" + filePath;
    webView->goToURL(fileURL);
}

//==============================================================================
bool ApogeeAudioProcessorEditor::checkAuthorization()
{
    const auto decrypted_text = loadAndDecryptLicenseFile();
    
    if (decrypted_text.isEmpty())
        return false;
    
    juce::StringArray product_list;
    product_list.addTokens(decrypted_text, ",", "\"");
    product_list.trim();
    product_list.removeEmptyStrings();
    
    auto expiration_date = juce::Time::getCurrentTime();
    if (!product_list.isEmpty())
        expiration_date = juce::Time::fromISO8601(product_list[0]);
    
    bool authorized = (expiration_date > juce::Time::getCurrentTime() && product_list.contains("200002"));
    
    if (authorized != isAuthorized)
    {
        isAuthorized = authorized;
        if (isAuthorized)
        {
            loadWebUI();
            startTimer(1000 * 60 * 15); // Check every 15 minutes
        }
        else
        {
            loadAuthScreen();
            startTimer(5000); // Check every 5 seconds
        }
    }
    
    return isAuthorized;
}

//==============================================================================
juce::File ApogeeAudioProcessorEditor::getAuthFile()
{
    juce::File app_data_dir;

#if JUCE_MAC
    app_data_dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                   .getChildFile("Application Support")
                   .getChildFile("NNAudio Access");
#else
    app_data_dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).
      getChildFile("NNAudio Access");
#endif

    if (!app_data_dir.exists())
        auto _ = app_data_dir.createDirectory();

    auto auth_file = app_data_dir.getChildFile("license.dat");
    return auth_file;
}

//==============================================================================
juce::String ApogeeAudioProcessorEditor::loadAndDecryptLicenseFile()
{
    const auto auth_file = getAuthFile();

    if (!auth_file.existsAsFile())
        return {};

    const auto file_contents = auth_file.loadFileAsString();

    juce::MemoryOutputStream mo;
    if (!juce::Base64::convertFromBase64(mo, file_contents))
        return {};

    juce::MemoryBlock raw_data(mo.getData(), mo.getDataSize());

    static auto key = "TrXkVijSb2SCGrHY$vPpTpq^5Vxim%4qAl@cj#iB9!1*C0jE3p&!V@Kkw9TBG&yY2fQeh7yO";
    const juce::BlowFish blow_fish(key, static_cast<int>(std::strlen(key)));

    blow_fish.decrypt(raw_data.getData(), raw_data.getSize());

    auto decrypted_text = juce::String::fromUTF8(
        static_cast<const char*>(raw_data.getData()),
        static_cast<int>(raw_data.getSize()));

    return decrypted_text;
}

//==============================================================================
void ApogeeAudioProcessorEditor::timerCallback()
{
    // Periodically check authorization status
    checkAuthorization();
}
