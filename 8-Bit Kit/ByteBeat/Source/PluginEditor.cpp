/*
  ==============================================================================

    ByteBeat - Timing Stabilizer
    MIDI FX Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ByteBeatAudioProcessorEditor::ByteBeatAudioProcessorEditor (ByteBeatAudioProcessor& p)
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
    
    // Set up page finished loading callback - show webView after page loads
    webView->onPageFinishedLoading = [this](const juce::String& url) {
        juce::MessageManager::callAsync([this]() {
            // Show webView after a short delay to ensure HTML is rendered
            juce::Timer::callAfterDelay(100, [this]() {
                if (webView != nullptr && !webView->isVisible()) {
                    webView->setVisible(true);
                    repaint();
                }
            });
        });
    };
    
    // Fallback: Show webView after 1 second if callback doesn't fire
    juce::Timer::callAfterDelay(1000, [this]() {
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
        startTimer(50); // Re-check every 15 minutes
    }
    else
    {
        loadAuthScreen();
        startTimer(5000); // Re-check every 5 seconds
    }
}

ByteBeatAudioProcessorEditor::~ByteBeatAudioProcessorEditor()
{
}

void ByteBeatAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black); // Fill with black background to prevent white flash (like NNAudioAccess)
}

void ByteBeatAudioProcessorEditor::resized()
{
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void ByteBeatAudioProcessorEditor::loadWebUI()
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
            auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals/8-Bit Kit");
            htmlFile = projectRoot.getChildFile ("ByteBeat").getChildFile ("UI").getChildFile ("index.html");
        }

        if (htmlFile.existsAsFile())
        {
            DBG("Found HTML file: " + htmlFile.getFullPathName());
            loadHTMLFile(htmlFile);
            // webView will be shown automatically via onPageFinishedLoading callback
        }
        else
        {
            DBG("Could not find index.html");
            DBG("Tried path: " + htmlFile.getFullPathName());
            // Try alternative path
            auto altPath = juce::File("/Users/rjmacbookpro/Development/Orbitals/8-Bit Kit/ByteBeat/UI/index.html");
            if (altPath.existsAsFile())
            {
                DBG("Found HTML file at alternative path: " + altPath.getFullPathName());
                loadHTMLFile(altPath);
            }
            else
            {
                DBG("Alternative path also failed: " + altPath.getFullPathName());
            }
        }
    });
}

void ByteBeatAudioProcessorEditor::loadHTMLFile (const juce::File& htmlFile)
{
    auto htmlContent = htmlFile.loadFileAsString();
    auto uiDir = htmlFile.getParentDirectory();
    auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals/8-Bit Kit");
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
    auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds/bytebeat.png");
    if (backgroundImage.existsAsFile())
    {
        juce::MemoryBlock imageData;
        if (backgroundImage.loadFileAsData(imageData))
        {
            juce::String base64 = juce::Base64::toBase64(imageData.getData(), imageData.getSize());
            htmlContent = htmlContent.replace("../../_Shared/Assets/backgrounds/bytebeat.png", 
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
        .getChildFile("ByteBeatUI_" + juce::String(juce::Time::currentTimeMillis()));
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

void ByteBeatAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
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
            auto& range = p->getNormalisableRange();
            float rawValue = static_cast<float>(value);
            rawValue = juce::jlimit(range.start, range.end, rawValue);
            float normalizedValue = range.convertTo0to1(rawValue);
            p->setValueNotifyingHost(normalizedValue);
        }
    }
}

//==============================================================================
void ByteBeatAudioProcessorEditor::loadAuthScreen()
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
    <title>ByteBeat - Authentication Required</title>
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
            <h1 class="auth-title">RABBITHOLE</h1>
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
    auto projectRoot = juce::File("/Users/rjmacbookpro/Development/Orbitals/8-Bit Kit");
    auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds/bytebeat.png");
    
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
        .getChildFile("ByteBeatAuth_" + juce::String(juce::Time::currentTimeMillis()));
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
bool ByteBeatAudioProcessorEditor::checkAuthorization()
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
            startTimer(50); // Check every 15 minutes
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
juce::File ByteBeatAudioProcessorEditor::getAuthFile()
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
juce::String ByteBeatAudioProcessorEditor::loadAndDecryptLicenseFile()
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
void ByteBeatAudioProcessorEditor::timerCallback()
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
                startTimer(50);
            }
        }
    }
    
            
    
    // Send metering data for audio-reactive animation
    if (isAuthorized && webView != nullptr && webView->isVisible())
    {
        sendMeteringData();
    }
}

void ByteBeatAudioProcessorEditor::sendMeteringData()
{
    // Get audio levels from processor
    float inputLevelDb = audioProcessor.inputLevel.load();
    float outputLevelDb = audioProcessor.outputLevel.load();
    
    // Send to JavaScript
    juce::String script = juce::String::formatted(
        "if (window.receiveAudioData) { window.receiveAudioData({ inputLevel: %.2f, outputLevel: %.2f }); }",
        inputLevelDb, outputLevelDb
    );
    
    if (webView != nullptr && webView->isVisible())
    {
        webView->evaluateJavascript(script);
    }
}

void ByteBeatAudioProcessorEditor::notifyMIDINote(int noteNumber, int velocity)
{
    if (webView != nullptr && isAuthorized)
    {
        juce::String script = "if (window.receiveMessageFromJUCE) { window.receiveMessageFromJUCE({ type: 'midiNote', note: " 
            + juce::String(noteNumber) + ", velocity: " + juce::String(velocity) + " }); }";
        webView->emitEventIfBrowserIsVisible("eval", script);
    }
}
