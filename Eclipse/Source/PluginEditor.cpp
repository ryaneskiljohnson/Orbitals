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
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make component opaque so black background shows through (like NNAudioAccess)
    setOpaque(true);
    // Enable native title bar on the top-level window (for standalone builds)
    if (auto* top_level = juce::TopLevelWindow::getTopLevelWindow(0))
        top_level->setUsingNativeTitleBar(true);

    setSize (1200, 750);
    setResizable (false, false);
    
    // Create WebView with native integration enabled for message passing
    auto options = juce::WebBrowserComponent::Options{}
        .withNativeIntegrationEnabled (true)
        .withKeepPageLoadedWhenBrowserIsHidden() // Keep page loaded when hidden (like NNAudioAccess)
        .withEventListener ("message", [this](const juce::var& message) {
            handleJavaScriptMessage (message);
        });

    webView = std::make_unique<WebBrowserWithCallbacks> (options);
    // Don't call setOpaque(false) - it causes white background flash
    addChildComponent (webView.get()); // Add as child but keep hidden until page loads
    webView->setBounds (getLocalBounds());
    webView->setVisible (false); // Start hidden to avoid white screen flash
    
    // Set up page finished loading callback - show webView only after page loads AND renders (like NNAudioAccess)
    webView->onPageFinishedLoading = [this](const juce::String& url) {
        juce::MessageManager::callAsync([this]() {
            // Wait 2 seconds to ensure HTML with inline black styles has fully rendered
            // Then verify background is black before showing
            juce::Timer::callAfterDelay(2000, [this]() {
                if (webView == nullptr || webView->isVisible()) return;
                
                // Verify background is black via JavaScript before showing
                juce::String checkScript = R"(
                    (function() {
                        if (!document.body) return false;
                        var style = window.getComputedStyle(document.body);
                        var bg = style.backgroundColor;
                        return bg === 'rgb(0, 0, 0)' || bg === 'black' || bg.indexOf('0, 0, 0') >= 0;
                    })();
                )";
                
                webView->evaluateJavascript(checkScript, [this](const juce::WebBrowserComponent::EvaluationResult& result) {
                    if (webView == nullptr || webView->isVisible()) return;
                    
                    bool isBlack = false;
                    if (auto* value = result.getResult()) {
                        if (value->isBool()) {
                            isBlack = static_cast<bool>(*value);
                        }
                    }
                    
                    // Show if black, or after additional delay if not confirmed
                    juce::Timer::callAfterDelay(isBlack ? 0 : 500, [this]() {
                        if (webView != nullptr && !webView->isVisible()) {
                            webView->setVisible(true);
                            repaint();
                        }
                    });
                });
            });
        });
    };
    
    // Fallback: Show webView after 3 seconds if callback doesn't fire (like NNAudioAccess fallback)
    juce::Timer::callAfterDelay(3000, [this]() {
        if (webView != nullptr && !webView->isVisible()) {
            webView->setVisible(true);
            repaint();
        }
    });
    
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

EclipseAudioProcessorEditor::~EclipseAudioProcessorEditor()
{
}

void EclipseAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black); // Fill with black background to prevent white flash (like NNAudioAccess)
}

void EclipseAudioProcessorEditor::resized()
{
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void EclipseAudioProcessorEditor::loadWebUI()
{
    // First, load a minimal black HTML to prevent white flash (like NNAudioAccess)
    juce::String blackHTML = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Eclipse</title>
    <style>
        /* Set background to black immediately to prevent white flash */
        html, body { 
            background-color: #000000; 
            margin: 0; 
            padding: 0; 
            width: 100%;
            height: 100%;
            overflow: hidden;
        }
    </style>
</head>
<body></body>
</html>)";
    
    // Load black HTML using temporary file
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("EclipseUI_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    auto blackFile = tempDir.getChildFile("black.html");
    blackFile.replaceWithText(blackHTML);
    auto blackFilePath = blackFile.getFullPathName().replace(" ", "%20");
    juce::String blackFileURL = "file://" + blackFilePath;
    // Load black HTML first (while webView is still hidden)
    webView->goToURL(blackFileURL);
    
    // Show webView after a brief delay to ensure black HTML has loaded
    // This prevents white flash - parent's black background shows until webView is visible
    juce::Timer::callAfterDelay(50, [this]()
    {
        webView->setVisible(true);
        addAndMakeVisible(webView.get());
    });
    
    // Then load the actual UI after a brief delay to ensure black page is rendered
    juce::MessageManager::callAsync([this]()
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
            htmlFile = projectRoot.getChildFile ("Eclipse").getChildFile ("UI").getChildFile ("index.html");
        }

        if (htmlFile.existsAsFile())
        {
            loadHTMLFile(htmlFile);
        }
        else
        {
            DBG ("Could not find index.html");
        }
    });
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
        else if (param == "bypass")
        {
            auto* p = audioProcessor.parameters.getParameter(EclipseAudioProcessor::PARAM_BYPASS);
            if (p != nullptr)
                p->setValueNotifyingHost((float)value);
        }
    }
}

//==============================================================================
void EclipseAudioProcessorEditor::loadAuthScreen()
{
    // First, load a minimal black HTML to prevent white flash (like NNAudioAccess)
    juce::String blackHTML = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Eclipse</title>
    <style>
        /* Set background to black immediately to prevent white flash */
        html, body { 
            background-color: #000000; 
            margin: 0; 
            padding: 0; 
            width: 100%;
            height: 100%;
            overflow: hidden;
        }
    </style>
</head>
<body></body>
</html>)";
    
    // Load black HTML using temporary file
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("EclipseAuth_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    auto blackFile = tempDir.getChildFile("black.html");
    blackFile.replaceWithText(blackHTML);
    auto blackFilePath = blackFile.getFullPathName().replace(" ", "%20");
    juce::String blackFileURL = "file://" + blackFilePath;
    // Load black HTML first (while webView is still hidden)
    webView->goToURL(blackFileURL);
    
    // Show webView after a brief delay to ensure black HTML has loaded
    // This prevents white flash - parent's black background shows until webView is visible
    juce::Timer::callAfterDelay(50, [this]()
    {
        webView->setVisible(true);
        addAndMakeVisible(webView.get());
    });
    
    // Then load the actual auth screen after a brief delay
    juce::MessageManager::callAsync([this]()
    {
        // Create auth HTML content with background image
        juce::String authHTML = R"(<!DOCTYPE html>// Create auth HTML content with background image
    juce::String authHTML = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Eclipse - Authentication Required</title>
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
            <h1 class="auth-title">ECLIPSE</h1>
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
    auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds/eclipse-background.png");
    
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
        .getChildFile("EclipseAuth_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    auto tempFile = tempDir.getChildFile("auth.html");
    tempFile.replaceWithText(authHTML);
    
    auto filePath = tempFile.getFullPathName().replace(" ", "%20");
    juce::String fileURL = "file://" + filePath;
    webView->goToURL(fileURL);
}    });

//==============================================================================
bool EclipseAudioProcessorEditor::checkAuthorization()
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
    
    bool authorized = (expiration_date > juce::Time::getCurrentTime() && product_list.contains("200005"));
    
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
juce::File EclipseAudioProcessorEditor::getAuthFile()
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
juce::String EclipseAudioProcessorEditor::loadAndDecryptLicenseFile()
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
void EclipseAudioProcessorEditor::timerCallback()
{
    // Periodically check authorization status
    checkAuthorization();
}
