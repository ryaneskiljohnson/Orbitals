/*
  ==============================================================================

    Quarks - Timing Stabilizer
    MIDI FX Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
QuarksAudioProcessorEditor::QuarksAudioProcessorEditor (QuarksAudioProcessor& p)
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
    // Longer delay ensures HTML/CSS is fully loaded and rendered
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

QuarksAudioProcessorEditor::~QuarksAudioProcessorEditor()
{
}

void QuarksAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black); // Fill with black background to prevent white flash (like NNAudioAccess)
}

void QuarksAudioProcessorEditor::resized()
{
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void QuarksAudioProcessorEditor::loadWebUI()
{
    // Navigate DIRECTLY to actual content while webView is HIDDEN (like NNAudioAccess)
    // Content has inline black styles, so it will be black when it loads
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
            auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");
            htmlFile = projectRoot.getChildFile ("Quarks").getChildFile ("UI").getChildFile ("index.html");
        }

        if (htmlFile.existsAsFile())
        {
            loadHTMLFile(htmlFile);
            // webView will be shown automatically via onPageFinishedLoading callback
        }
        else
        {
            DBG ("Could not find index.html");
        }
    });
}

void QuarksAudioProcessorEditor::loadHTMLFile (const juce::File& htmlFile)
{
    auto htmlContent = htmlFile.loadFileAsString();
    auto uiDir = htmlFile.getParentDirectory();
    auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");
    auto sharedDir = projectRoot.getChildFile ("_Shared").getChildFile ("UI");

    // CRITICAL: Inject inline black styles FIRST (before any CSS links)
    // This prevents white flash - black background applies immediately when HTML loads
    juce::String blackStyles = R"(<style>
        /* Set background to black immediately to prevent white flash */
        html, body { 
            background-color: #000000 !important; 
            margin: 0; 
            padding: 0; 
        }
    </style>)";
    
    // Inject black styles right after <head> tag (before any CSS links)
    if (htmlContent.contains("<head>"))
    {
        htmlContent = htmlContent.replace("<head>", "<head>\n    " + blackStyles);
    }
    else if (htmlContent.contains("<head "))
    {
        // Handle <head> with attributes - find the closing > of <head ...>
        int headStart = htmlContent.indexOf("<head");
        if (headStart >= 0)
        {
            // Find the closing > after <head
            for (int i = headStart; i < htmlContent.length(); ++i)
            {
                if (htmlContent[i] == '>')
                {
                    htmlContent = htmlContent.substring(0, i + 1) + "\n    " + blackStyles + htmlContent.substring(i + 1);
                    break;
                }
            }
        }
    }

    // Inline CSS
    auto cssFile = uiDir.getChildFile ("styles.css");
    if (cssFile.existsAsFile())
    {
        auto cssContent = cssFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<link rel=\"stylesheet\" href=\"styles.css\">",
                                           "<style>" + cssContent + "</style>");
    }

    auto designSystemFile = sharedDir.getChildFile ("mandelbrot-design-system.css");
    if (designSystemFile.existsAsFile())
    {
        auto designSystemContent = designSystemFile.loadFileAsString();
        
        // Replace logo image path with relative path for temp directory
        // Handle both single and double quotes
        juce::String logoPattern = "../../_Shared/Assets/logos/nnaudio-logo.png";
        juce::String logoOldPattern1 = "url('" + logoPattern + "')";
        juce::String logoOldPattern2 = "url(\"" + logoPattern + "\")";
        juce::String logoNewPattern = "url('nnaudio-logo.png')";
        designSystemContent = designSystemContent.replace (logoOldPattern1, logoNewPattern);
        designSystemContent = designSystemContent.replace (logoOldPattern2, logoNewPattern);
        
        htmlContent = htmlContent.replace ("<link rel=\"stylesheet\" href=\"../../_Shared/UI/mandelbrot-design-system.css\">",
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

    auto animationsFile = sharedDir.getChildFile ("mandelbrot-animations.js");
    if (animationsFile.existsAsFile())
    {
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/mandelbrot-animations.js\"></script>",
                                           "<script>" + animationsFile.loadFileAsString() + "</script>");
    }

    auto particlesFile = sharedDir.getChildFile ("mandelbrot-particles.js");
    if (particlesFile.existsAsFile())
    {
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/mandelbrot-particles.js\"></script>",
                                           "<script>" + particlesFile.loadFileAsString() + "</script>");
    }

    auto componentsFile = sharedDir.getChildFile ("mandelbrot-components.js");
    if (componentsFile.existsAsFile())
    {
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/mandelbrot-components.js\"></script>",
                                           "<script>" + componentsFile.loadFileAsString() + "</script>");
    }

    // Handle background image
    auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds/quarks.png");
    if (backgroundImage.existsAsFile())
    {
        juce::MemoryBlock imageData;
        if (backgroundImage.loadFileAsData(imageData))
        {
            juce::String base64 = juce::Base64::toBase64(imageData.getData(), imageData.getSize());
            htmlContent = htmlContent.replace("../../_Shared/Assets/backgrounds/quarks.png", 
                                            "data:image/png;base64," + base64);
        }
    }

    // Disable right-click context menu and set standalone mode flag
    juce::String disableRightClickScript = R"(<script>
        document.addEventListener('contextmenu', function(e) { e.preventDefault(); return false; });
        document.addEventListener('selectstart', function(e) { e.preventDefault(); return false; });
        // Set standalone mode flag (only true in standalone builds)
        window.isStandaloneMode = )";
    
#if JucePlugin_Build_Standalone
    disableRightClickScript += "true";
#else
    disableRightClickScript += "false";
#endif
    
    disableRightClickScript += R"(;
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
        .getChildFile("QuarksUI_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    // Copy logo image to temp directory if it exists
    auto logosDir = projectRoot.getChildFile ("_Shared").getChildFile ("Assets").getChildFile ("logos");
    auto logoFile = logosDir.getChildFile ("nnaudio-logo.png");
    if (logoFile.existsAsFile())
    {
        auto tempLogoFile = tempDir.getChildFile ("nnaudio-logo.png");
        logoFile.copyFileTo (tempLogoFile);
    }
    
    auto tempFile = tempDir.getChildFile("index.html");
    tempFile.replaceWithText(htmlContent);
    
    auto filePath = tempFile.getFullPathName().replace(" ", "%20");
    juce::String fileURL = "file://" + filePath;
    webView->goToURL(fileURL);
}

void QuarksAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
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
            if (param == "expansion")
                p->setValueNotifyingHost((float)value / 100.0f);
            else if (param == "threshold")
                p->setValueNotifyingHost((float)value / 127.0f);
            else if (param == "ceiling")
                p->setValueNotifyingHost((float)value / 127.0f);
            else if (param == "curve")
                p->setValueNotifyingHost((float)value / 100.0f);
            else if (param == "bypass")
                p->setValueNotifyingHost((float)value);
        }
    }
}

//==============================================================================
void QuarksAudioProcessorEditor::loadAuthScreen()
{
    // Navigate DIRECTLY to auth content while webView is HIDDEN (like NNAudioAccess)
    // Content has inline black styles, so it will be black when it loads
    juce::MessageManager::callAsync([this]()
    {
        // Create auth HTML content with background image
        juce::String authHTML = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Quarks - Authentication Required</title>
    <style>
        /* Set background to black immediately to prevent white flash */
        html, body { 
            background-color: #000000 !important; 
            margin: 0; 
            padding: 0; 
        }
    </style>
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
            <h1 class="auth-title">QUARKS</h1>
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
    auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds/quarks.png");
    
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
        .getChildFile("QuarksAuth_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    auto tempFile = tempDir.getChildFile("auth.html");
    tempFile.replaceWithText(authHTML);
    
    auto filePath = tempFile.getFullPathName().replace(" ", "%20");
    juce::String fileURL = "file://" + filePath;
    
    // Navigate to auth screen while webView is HIDDEN
    webView->goToURL(fileURL);
    // webView will be shown automatically via onPageFinishedLoading callback
    });
}

//==============================================================================
bool QuarksAudioProcessorEditor::checkAuthorization()
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
    
    // Read product ID from BinaryData resource (product_id.txt)
    juce::String product_id = juce::String::fromUTF8(BinaryData::product_id_txt, BinaryData::product_id_txtSize).trim();
    bool authorized = (expiration_date > juce::Time::getCurrentTime() && product_list.contains(product_id));
    
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
juce::File QuarksAudioProcessorEditor::getAuthFile()
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
juce::String QuarksAudioProcessorEditor::loadAndDecryptLicenseFile()
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
void QuarksAudioProcessorEditor::timerCallback()
{
    if (!isAuthorized)
    {
        bool newAuthState = checkAuthorization();
        if (newAuthState != isAuthorized)
        {
            isAuthorized = newAuthState;
            if (isAuthorized)
            {
                loadWebUI();
                startTimer(1000 * 60 * 15);
            }
        }
    }
}

void QuarksAudioProcessorEditor::notifyMIDINote(int noteNumber, int velocity)
{
    if (webView != nullptr && isAuthorized)
    {
        juce::String script = "if (window.receiveMessageFromJUCE) { window.receiveMessageFromJUCE({ type: 'midiNote', note: " 
            + juce::String(noteNumber) + ", velocity: " + juce::String(velocity) + " }); }";
        webView->emitEventIfBrowserIsVisible("eval", script);
    }
}
