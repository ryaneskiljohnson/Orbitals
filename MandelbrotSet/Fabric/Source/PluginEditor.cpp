/*
  ==============================================================================

    Fabric - Spacetime Reverb
    Audio FX Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <cmath>
#include <string>
#if JucePlugin_Build_Standalone
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

//==============================================================================
FabricAudioProcessorEditor::FabricAudioProcessorEditor (FabricAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make component opaque so black background shows through (like NNAudioAccess)
    setOpaque(true);
    
    // Enable native title bar on the top-level window (for standalone builds)
    if (auto* top_level = juce::TopLevelWindow::getTopLevelWindow(0))
        top_level->setUsingNativeTitleBar(true);

    setSize (1200, 750);
    setResizable (false, false);
    
    std::cout << "Creating WebView..." << std::endl;
    DBG("Creating WebView...");
    
    // Create WebView with native integration enabled for message passing
    std::cout << "Setting up WebView options with native integration..." << std::endl;
    DBG("Setting up WebView options with native integration...");
    
    auto options = juce::WebBrowserComponent::Options{}
        .withNativeIntegrationEnabled (true)
        .withKeepPageLoadedWhenBrowserIsHidden() // Keep page loaded when hidden (like Zenith)
        .withEventListener ("message", [this](const juce::var& message) {
            std::cout << "=== EVENT LISTENER TRIGGERED ===" << std::endl;
            DBG("=== EVENT LISTENER TRIGGERED ===");
            std::cout << "Message received: " << message.toString() << std::endl;
            DBG("Message received: " + message.toString());
            handleJavaScriptMessage (message);
        });
    
    std::cout << "WebView options configured" << std::endl;
    DBG("WebView options configured");

    webView = std::make_unique<WebBrowserWithCallbacks> (options);
    std::cout << "WebView created" << std::endl;
    DBG("WebView created");
    
    // Don't call setOpaque(false) - it causes white background flash
    addChildComponent (webView.get()); // Add as child but keep hidden until page loads
    webView->setBounds (getLocalBounds());
    webView->setVisible (false); // Start hidden to avoid white screen flash
    
    std::cout << "WebView configured, bounds set" << std::endl;
    DBG("WebView configured, bounds set");
    
    // Set up page finished loading callback - show webView only after page loads AND renders (like NNAudioAccess)
    webView->onPageFinishedLoading = [this](const juce::String& url) {
        std::cout << "WebView page finished loading: " << url << std::endl;
        std::cout << "onPageFinishedLoading callback triggered" << std::endl;
        DBG("WebView page finished loading: " + url);
        DBG("onPageFinishedLoading callback triggered");
        
        std::cout << "Scheduling 2-second delay before checking background..." << std::endl;
        DBG("Scheduling 2-second delay before checking background...");
        
        juce::MessageManager::callAsync([this]() {
            std::cout << "In MessageManager::callAsync" << std::endl;
            DBG("In MessageManager::callAsync");
            
            // Wait 2 seconds to ensure HTML with inline black styles has fully rendered
            // Then verify background is black before showing
            juce::Timer::callAfterDelay(2000, [this]() {
                std::cout << "2-second delay complete, checking background..." << std::endl;
                DBG("2-second delay complete, checking background...");
                if (webView == nullptr || webView->isVisible()) return;
                
                // Verify background is black AND initialize bridge via JavaScript
                juce::String checkScript = R"(
                    (function() {
                        console.log('🔵 C++ initialization script running');
                        console.log('window.juce exists:', typeof window.juce !== 'undefined');
                        
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
                            
                            // After showing, test the bridge and send ready signal to JavaScript
                            juce::Timer::callAfterDelay(500, [this]() {
                                if (webView != nullptr) {
                                    std::cout << "Testing WebView bridge..." << std::endl;
                                    DBG("Testing WebView bridge...");
                                    
                                    // Test script that verifies window.juce and logs status
                                    juce::String testScript = R"(
                                        alert('WebView is visible and JavaScript is running!');
                                        console.log('🔵 C++ has shown the WebView');
                                        console.log('🔵 Testing bridge...');
                                        console.log('🔵 window.juce exists:', typeof window.juce !== 'undefined');
                                        console.log('🔵 window.juce.postMessage exists:', typeof window.juce?.postMessage === 'function');
                                        
                                        // Test sending a message back to C++
                                        if (window.juce && window.juce.postMessage) {
                                            console.log('🔵 Sending test message to C++...');
                                            window.juce.postMessage({type: 'test', value: 'Bridge test from JavaScript'});
                                            alert('Sent test message to C++ via window.juce.postMessage');
                                        } else {
                                            alert('ERROR: window.juce is NOT available! Bridge is broken!');
                                        }
                                    )";
                                    
                                    webView->evaluateJavascript(testScript);
                                    
                                    std::cout << "Bridge test script sent to JavaScript" << std::endl;
                                    DBG("Bridge test script sent to JavaScript");
                                }
                            });
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
    std::cout << "Checking authorization..." << std::endl;
    DBG("Checking authorization...");
    
    isAuthorized = checkAuthorization();
    
    std::cout << "Authorization status: " << (isAuthorized ? "AUTHORIZED" : "NOT AUTHORIZED") << std::endl;
    DBG(juce::String("Authorization status: ") + (isAuthorized ? "AUTHORIZED" : "NOT AUTHORIZED"));
    
    if (isAuthorized)
    {
        std::cout << "Loading Web UI..." << std::endl;
        DBG("Loading Web UI...");
        loadWebUI();
        startTimer(50); // Fast timer for audio-reactive animation (20 FPS data updates)
    }
    else
    {
        std::cout << "Loading Auth Screen..." << std::endl;
        DBG("Loading Auth Screen...");
        loadAuthScreen();
        startTimer(5000); // Re-check auth every 5 seconds
    }
    
    std::cout << "=== CONSTRUCTOR COMPLETE ===" << std::endl;
    DBG("=== CONSTRUCTOR COMPLETE ===");
}

FabricAudioProcessorEditor::~FabricAudioProcessorEditor()
{
}

//==============================================================================

//==============================================================================
void FabricAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
{
    std::cout << "=== RECEIVED MESSAGE FROM JAVASCRIPT ===" << std::endl;
    DBG("=== RECEIVED MESSAGE FROM JAVASCRIPT ===");
    std::cout << "Message type check: " << (message.isObject() ? "Object" : message.isString() ? "String" : "Other") << std::endl;
    DBG("Message type check: " + juce::String(message.isObject() ? "Object" : message.isString() ? "String" : "Other"));
    std::cout << "Message content: " << message.toString() << std::endl;
    DBG("Message content: " + message.toString());
    
    // JUCE sends the message as an already-parsed object from JSON::fromString()
    // So it should already be an object, not a string
    if (!message.isObject())
    {
        std::cout << "Message is not an object, returning" << std::endl;
        DBG("Message is not an object, returning");
        return;
    }

    auto obj = message.getDynamicObject();
    if (obj == nullptr)
    {
        DBG("Could not get dynamic object");
        return;
    }

    auto type = obj->getProperty("type").toString();
    std::cout << "Message type: " << type.toStdString() << std::endl;
    DBG("Message type: " + type);
    
    // Handle test messages
    if (type == "test")
    {
        auto testValue = obj->getProperty("value").toString();
        std::cout << "✅✅✅ BRIDGE TEST SUCCESSFUL! ✅✅✅" << std::endl;
        std::cout << "   Received test message: " << testValue.toStdString() << std::endl;
        DBG("✅✅✅ BRIDGE TEST SUCCESSFUL! ✅✅✅");
        DBG("   Received test message: " + testValue);
        return; // Don't process as parameter change
    }
    
    if (type == "parameterChange")
    {
        auto param = obj->getProperty("parameter").toString();
        auto value = obj->getProperty("value");
        
        std::cout << "Parameter change request: " << param.toStdString() << " = " << value.toString().toStdString() << std::endl;
        DBG("Parameter change request: " + param + " = " + value.toString());

        auto* p = audioProcessor.parameters.getParameter(param);
        if (p != nullptr)
        {
            // Get the parameter's range
            if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(p))
            {
                float rawValue = static_cast<float>(value);
                auto range = rangedParam->getNormalisableRange();
                
                std::cout << "  Parameter range: " << range.start << " to " << range.end << std::endl;
                std::cout << "  Raw value from JS: " << rawValue << std::endl;
                
                // Clamp value to parameter range
                rawValue = juce::jlimit(range.start, range.end, rawValue);
                
                // Convert raw value (in parameter's native range) to normalized 0-1
                float normalizedValue = range.convertTo0to1(rawValue);
                
                std::cout << "  Normalized value: " << normalizedValue << std::endl;
                std::cout << "  Setting parameter..." << std::endl;
                
                // Set the parameter value (this will notify the host and update the DSP)
                p->setValueNotifyingHost(normalizedValue);
                
                // Verify it was set
                float currentValue = p->getValue();
                float currentRawValue = range.convertFrom0to1(currentValue);
                
                // Also check the raw parameter value directly
                std::atomic<float>* rawParamValue = audioProcessor.parameters.getRawParameterValue(param);
                if (rawParamValue != nullptr)
                {
                    std::cout << "  Raw parameter value (direct): " << rawParamValue->load() << std::endl;
                }
                
                std::cout << "  ✅ Parameter set! Current normalized: " << currentValue << ", Current raw: " << currentRawValue << std::endl;
                
                // Use std::cout for logging instead of DBG to avoid JUCE String assertion issues
                // DBG calls with float-to-string conversions can cause assertions in JUCE
                if (!std::isnan(currentValue) && !std::isinf(currentValue) && 
                    !std::isnan(currentRawValue) && !std::isinf(currentRawValue))
                {
                    std::cout << "  ✅ Parameter updated: " << param.toStdString() 
                              << " -> normalized: " << currentValue 
                              << ", raw: " << currentRawValue << std::endl;
                }
                else
                {
                    std::cout << "  ⚠️ Parameter has invalid values (NaN/Inf)" << std::endl;
                }
            }
            else
            {
                // For non-ranged parameters (like bypass bool)
                float normalizedValue = static_cast<float>(value);
                p->setValueNotifyingHost(normalizedValue);
                std::cout << "  ✅ Non-ranged parameter set: " << normalizedValue << std::endl;
                
                // Use std::cout for logging instead of DBG to avoid JUCE String assertion issues
                if (!std::isnan(normalizedValue) && !std::isinf(normalizedValue))
                {
                    std::cout << "  ✅ Non-ranged parameter updated: " << param.toStdString() 
                              << " -> " << normalizedValue << std::endl;
                }
                else
                {
                    std::cout << "  ⚠️ Non-ranged parameter has invalid value (NaN/Inf)" << std::endl;
                }
            }
        }
        else
        {
            std::cout << "  ❌ Parameter not found: " << param.toStdString() << std::endl;
            DBG("❌ Parameter not found: " + param);
            DBG("Available parameters:");
            for (auto* availParam : audioProcessor.parameters.processor.getParameters())
            {
                if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(availParam))
                {
                    DBG("  - " + rangedParam->getParameterID());
                }
            }
        }
    }
    else if (type == "openSettings")
    {
        DBG("Received openSettings message from JavaScript");
        openAudioSettings();
    }
    else if (type == "test")
    {
        auto testValue = obj->getProperty("value").toString();
        DBG("✅ BRIDGE TEST SUCCESSFUL! Received: " + testValue);
    }
    else
    {
        DBG("Unknown message type: " + type);
    }
}

void FabricAudioProcessorEditor::openAudioSettings()
{
#if JucePlugin_Build_Standalone
    // In standalone mode, show the audio/MIDI settings dialog
    juce::MessageManager::callAsync([this]()
    {
        DBG("Opening audio settings...");
        
        // Try multiple approaches to access the settings dialog
        
        // Approach 1: Get through StandalonePluginHolder singleton
        if (auto* pluginHolder = juce::StandalonePluginHolder::getInstance())
        {
            DBG("Found StandalonePluginHolder via getInstance()");
            pluginHolder->showAudioSettingsDialog();
            return;
        }
        
        DBG("StandalonePluginHolder::getInstance() returned nullptr");
        
        // Approach 2: Try to find StandaloneFilterWindow in the component hierarchy
        juce::Component* comp = this;
        while (comp != nullptr)
        {
            if (auto* window = dynamic_cast<juce::StandaloneFilterWindow*>(comp))
            {
                DBG("Found StandaloneFilterWindow in component hierarchy");
                if (auto* holder = window->getPluginHolder())
                {
                    holder->showAudioSettingsDialog();
                    return;
                }
            }
            comp = comp->getParentComponent();
        }
        
        DBG("Could not find StandaloneFilterWindow in component hierarchy");
        
        // Approach 3: Try top-level window
        if (auto* topLevelWindow = juce::TopLevelWindow::getTopLevelWindow(0))
        {
            DBG("Found top-level window");
            if (auto* standaloneWindow = dynamic_cast<juce::StandaloneFilterWindow*>(topLevelWindow))
            {
                DBG("Top-level window is StandaloneFilterWindow");
                if (auto* pluginHolder = standaloneWindow->getPluginHolder())
                {
                    pluginHolder->showAudioSettingsDialog();
                    return;
                }
            }
            else
            {
                DBG("Top-level window is NOT StandaloneFilterWindow");
            }
        }
        
        DBG("Failed to open audio settings - no method worked");
        
        // Show error message
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Settings Unavailable",
            "Unable to access audio/MIDI settings.\n\nThis feature is only available in standalone mode.",
            "OK"
        );
    });
#else
    // In plugin mode (VST3/AU), show a message explaining that audio settings
    // are managed by the host DAW
    juce::MessageManager::callAsync([this]()
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "Audio/MIDI Settings",
            "Audio and MIDI device settings are managed by your DAW.\n\n"
            "Please use your DAW's audio/MIDI preferences to configure devices.",
            "OK"
        );
    });
#endif
}

void FabricAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black); // Fill with black background to prevent white flash (like NNAudioAccess)
}

void FabricAudioProcessorEditor::resized()
{
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void FabricAudioProcessorEditor::loadWebUI()
{
    std::cout << "=== LOAD WEB UI CALLED ===" << std::endl;
    DBG("=== LOAD WEB UI CALLED ===");
    
    // Navigate DIRECTLY to actual content while webView is HIDDEN (like NNAudioAccess)
    // Content has inline black styles, so it will be black when it loads
    juce::MessageManager::callAsync([this]()
    {
        std::cout << "In loadWebUI async callback" << std::endl;
        DBG("In loadWebUI async callback");
        
        // Find UI files relative to plugin binary
        auto htmlFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
            .getParentDirectory()
            .getChildFile("Resources")
            .getChildFile("index.html");

        std::cout << "Checking for HTML at: " << htmlFile.getFullPathName() << std::endl;
        DBG("Checking for HTML at: " + htmlFile.getFullPathName());

        // Fallback: try development path
        if (!htmlFile.existsAsFile())
        {
            std::cout << "Not found in Resources, trying development path..." << std::endl;
            DBG("Not found in Resources, trying development path...");
            
            auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");
            htmlFile = projectRoot.getChildFile ("Fabric").getChildFile ("UI").getChildFile ("index.html");
            
            std::cout << "Development path: " << htmlFile.getFullPathName() << std::endl;
            DBG("Development path: " + htmlFile.getFullPathName());
        }

        if (htmlFile.existsAsFile())
        {
            std::cout << "✅ Found HTML file, loading..." << std::endl;
            DBG("✅ Found HTML file, loading...");
            loadHTMLFile(htmlFile);
            // webView will be shown automatically via onPageFinishedLoading callback
        }
        else
        {
            std::cout << "❌ Could not find index.html" << std::endl;
            DBG ("Could not find index.html");
        }
    });
}

void FabricAudioProcessorEditor::loadHTMLFile (const juce::File& htmlFile)
{
    std::cout << "=== LOAD HTML FILE ===" << std::endl;
    std::cout << "HTML file: " << htmlFile.getFullPathName() << std::endl;
    DBG("=== LOAD HTML FILE ===");
    DBG("HTML file: " + htmlFile.getFullPathName());
    
    auto htmlContent = htmlFile.loadFileAsString();
    std::cout << "HTML content length: " << htmlContent.length() << " characters" << std::endl;
    DBG("HTML content length: " + juce::String(htmlContent.length()) + " characters");
    
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
    auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds/fabric.png");
    if (backgroundImage.existsAsFile())
    {
        juce::MemoryBlock imageData;
        if (backgroundImage.loadFileAsData(imageData))
        {
            juce::String base64 = juce::Base64::toBase64(imageData.getData(), imageData.getSize());
            htmlContent = htmlContent.replace("../../_Shared/Assets/backgrounds/fabric.png", 
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
        .getChildFile("FabricUI_" + juce::String(juce::Time::currentTimeMillis()));
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
    
    std::cout << "Loading URL: " << fileURL << std::endl;
    std::cout << "Temp file size: " << tempFile.getSize() << " bytes" << std::endl;
    DBG("Loading URL: " + fileURL);
    DBG("Temp file size: " + juce::String(tempFile.getSize()) + " bytes");
    
    webView->goToURL(fileURL);
    
    std::cout << "goToURL() called, waiting for page to load..." << std::endl;
    DBG("goToURL() called, waiting for page to load...");
    
    // Immediately try to run JavaScript (will fail if page not loaded yet, but worth testing)
    juce::Timer::callAfterDelay(3000, [this, fileURL]() {
        std::cout << "=== TESTING JAVASCRIPT EXECUTION (3 seconds after goToURL) ===" << std::endl;
        DBG("=== TESTING JAVASCRIPT EXECUTION (3 seconds after goToURL) ===");
        
        if (webView != nullptr) {
            std::cout << "WebView exists, trying to execute JavaScript..." << std::endl;
            std::cout << "WebView visible: " << (webView->isVisible() ? "YES" : "NO") << std::endl;
            DBG("WebView exists, trying to execute JavaScript...");
            DBG(juce::String("WebView visible: ") + (webView->isVisible() ? "YES" : "NO"));
            
            // Try BOTH methods to execute JavaScript
            juce::String simpleAlert = "alert('DIRECT TEST: JavaScript is working!');";
            
            std::cout << "Trying evaluateJavascript()..." << std::endl;
            webView->evaluateJavascript(simpleAlert);
            
            std::cout << "Trying emitEventIfBrowserIsVisible()..." << std::endl;
            webView->emitEventIfBrowserIsVisible("eval", simpleAlert);
            
            std::cout << "Both methods called" << std::endl;
            DBG("Both JavaScript execution methods called");
        } else {
            std::cout << "ERROR: webView is nullptr!" << std::endl;
            DBG("ERROR: webView is nullptr!");
        }
    });
}

//==============================================================================
void FabricAudioProcessorEditor::loadAuthScreen()
{
    // Navigate DIRECTLY to auth content while webView is HIDDEN (like Zenith)
    // Content has inline black styles, so it will be black when it loads
    juce::MessageManager::callAsync([this]()
    {
        // Create auth HTML content with background image
        juce::String authHTML = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Fabric - Authentication Required</title>
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
            <h1 class="auth-title">FABRIC</h1>
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
    auto projectRoot = juce::File("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");
    auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds/fabric.png");
    
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
        .getChildFile("FabricAuth_" + juce::String(juce::Time::currentTimeMillis()));
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
bool FabricAudioProcessorEditor::checkAuthorization()
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
            startTimer(50); // Fast timer for audio-reactive animation
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
juce::File FabricAudioProcessorEditor::getAuthFile()
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
juce::String FabricAudioProcessorEditor::loadAndDecryptLicenseFile()
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
void FabricAudioProcessorEditor::timerCallback()
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
            }
        }
    }
    
    // Always send metering data when authorized and visible (fast update for animation)
    if (isAuthorized && webView != nullptr && webView->isVisible())
    {
        sendMeteringData();
    }
}

void FabricAudioProcessorEditor::sendMeteringData()
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

