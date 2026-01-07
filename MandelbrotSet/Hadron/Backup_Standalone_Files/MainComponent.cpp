/*
  ==============================================================================

    This file contains the basic framework code for a JUCE processor editor.

  ==============================================================================
*/

#include "MainComponent.h"
#include <cstring>

//==============================================================================
MainComponent::MainComponent()
{
    setSize (1200, 750);
    setVisible (true);
    setOpaque (true);
    
    // Initialize audio device manager
    audioDeviceManager.initialiseWithDefaultDevices (0, 2);
    audioDeviceManager.addAudioCallback (this);
    
    // Use JUCE WebView for CSS-based UI (WebBrowserComponent is in juce_gui_extra)
    useWebView = true;
    
    // Create WebView with native integration enabled for message passing
    auto options = juce::WebBrowserComponent::Options{}
        .withNativeIntegrationEnabled (true)
        .withEventListener ("message", [this](const juce::var& message) {
            handleJavaScriptMessage (message);
        });
    
    webView = std::make_unique<juce::WebBrowserComponent> (options);
    
    // Set WebView to be non-opaque so black background shows through
    webView->setOpaque (false);
    
    addAndMakeVisible (webView.get());
    webView->setBounds (getLocalBounds());
    
    // Load shared loading animation (embedded inline)
    const char* loadingHTML = 
        "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Mandelbrot Set - Loading</title>"
        "<style>"
        "* { margin: 0; padding: 0; box-sizing: border-box; }"
        "body, html { width: 100%; height: 100%; overflow: hidden; font-family: 'Orbitron', 'Space Mono', monospace; background: #0a0a0f; color: #e8e8f0; }"
        ".loading-container { width: 100%; height: 100%; display: flex; flex-direction: column; align-items: center; justify-content: center; position: relative; overflow: hidden; }"
        ".loading-container::before { content: ''; position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-image: radial-gradient(1px 1px at 20% 30%, white, transparent), radial-gradient(1px 1px at 60% 70%, white, transparent), radial-gradient(1px 1px at 50% 50%, white, transparent), radial-gradient(1px 1px at 80% 10%, white, transparent), radial-gradient(1px 1px at 90% 60%, white, transparent), radial-gradient(2px 2px at 30% 80%, white, transparent), radial-gradient(1px 1px at 40% 20%, white, transparent); background-size: 200% 200%; background-position: 0% 0%; opacity: 0.4; animation: twinkle 4s ease-in-out infinite; }"
        ".mandelbrot-logo { width: 120px; height: 120px; background: radial-gradient(circle at 40% 40%, #00d4ff, #0094ff); border-radius: 50%; display: flex; align-items: center; justify-content: center; font-size: 64px; font-weight: bold; color: #0a0a0f; box-shadow: 0 0 60px rgba(0, 212, 255, 0.8), 0 0 100px rgba(0, 212, 255, 0.4); position: relative; margin-bottom: 40px; animation: pulse 2s ease-in-out infinite; }"
        ".mandelbrot-logo::before { content: ''; position: absolute; top: -20px; left: -20px; right: -20px; bottom: -20px; border: 2px solid rgba(0, 212, 255, 0.3); border-radius: 50%; animation: rotate 8s linear infinite; }"
        ".mandelbrot-logo::after { content: ''; position: absolute; top: -40px; left: -40px; right: -40px; bottom: -40px; border: 2px solid rgba(0, 212, 255, 0.2); border-radius: 50%; animation: rotate 12s linear infinite reverse; }"
        "h1 { font-size: 48px; font-weight: 700; letter-spacing: 8px; text-transform: uppercase; background: linear-gradient(90deg, #00d4ff, #0094ff, #00d4ff); background-size: 200% auto; -webkit-background-clip: text; -webkit-text-fill-color: transparent; background-clip: text; animation: shimmer 3s ease-in-out infinite; margin-bottom: 20px; }"
        ".subtitle { font-size: 16px; font-weight: 400; letter-spacing: 3px; text-transform: uppercase; opacity: 0.6; margin-bottom: 60px; }"
        ".loading-spinner { width: 200px; height: 4px; background: rgba(255, 255, 255, 0.1); border-radius: 2px; overflow: hidden; position: relative; }"
        ".loading-bar { height: 100%; background: linear-gradient(90deg, transparent, #00d4ff, #0094ff, #00d4ff, transparent); background-size: 200% 100%; animation: loading 1.5s ease-in-out infinite; box-shadow: 0 0 20px rgba(0, 212, 255, 0.6); }"
        ".loading-text { margin-top: 20px; font-size: 14px; letter-spacing: 2px; text-transform: uppercase; opacity: 0.5; animation: fade 2s ease-in-out infinite; }"
        "@keyframes pulse { 0%, 100% { transform: scale(1); box-shadow: 0 0 60px rgba(0, 212, 255, 0.8), 0 0 100px rgba(0, 212, 255, 0.4); } 50% { transform: scale(1.05); box-shadow: 0 0 80px rgba(0, 212, 255, 1), 0 0 120px rgba(0, 212, 255, 0.6); } }"
        "@keyframes rotate { from { transform: rotate(0deg); } to { transform: rotate(360deg); } }"
        "@keyframes shimmer { 0% { background-position: 0% center; } 100% { background-position: 200% center; } }"
        "@keyframes loading { 0% { transform: translateX(-100%); } 100% { transform: translateX(200%); } }"
        "@keyframes fade { 0%, 100% { opacity: 0.3; } 50% { opacity: 0.7; } }"
        "@keyframes twinkle { 0%, 100% { opacity: 0.3; } 50% { opacity: 0.5; } }"
        "</style>"
        "</head><body>"
        "<div class='loading-container'>"
        "<div class='mandelbrot-logo'>O</div>"
        "<h1>MANDELBROT SET</h1>"
        "<div class='subtitle'>Audio FX Plugin Line</div>"
        "<div class='loading-spinner'><div class='loading-bar'></div></div>"
        "<div class='loading-text'>Initializing...</div>"
        "</div>"
        "</body></html>";
    
    juce::String htmlString (loadingHTML);
    auto escapedHTML = juce::URL::addEscapeChars (htmlString, true);
    juce::String dataURL = "data:text/html;charset=utf-8," + escapedHTML;
    webView->goToURL (dataURL);
    
    webView->setVisible (true);
    
    DBG ("WebView created, bounds: " + getLocalBounds().toString());
    DBG ("Component size: " + juce::String (getWidth()) + "x" + juce::String (getHeight()));
    DBG ("Component visible: " + juce::String (isVisible() ? "YES" : "NO"));
    
    // Load UI after a delay to show loading animation
    juce::Timer::callAfterDelay (1500, [this]()
    {
        loadMainUI();
    });
}

MainComponent::~MainComponent()
{
    audioDeviceManager.removeAudioCallback (this);
    audioDeviceManager.closeAudioDevice();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // Always paint dark space background (matches --deep-space color)
    g.fillAll (juce::Colour (0xff12121a));
    
    // Only paint text if not using WebView
    if (!useWebView)
    {
        g.setColour (juce::Colours::white);
        g.setFont (20.0f);
        g.drawFittedText ("Synth (Native JUCE UI)", getLocalBounds(), juce::Justification::centred, 1);
    }
}

void MainComponent::resized()
{
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

void MainComponent::loadWebUI()
{
    loadMainUI();
}

void MainComponent::loadMainUI()
{
    // Find UI files in plugin's own UI folder
    auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");
    auto htmlFile = projectRoot.getChildFile ("Hadron").getChildFile ("UI").getChildFile ("index.html");
    
    DBG ("Looking for UI at: " + htmlFile.getFullPathName());
    
    // Try app bundle Resources
    if (!htmlFile.existsAsFile())
    {
        auto appDir = juce::File::getSpecialLocation (juce::File::currentApplicationFile);
        if (appDir.isDirectory())
        {
            htmlFile = appDir.getChildFile ("Contents")
                          .getChildFile ("Resources")
                          .getChildFile ("UI")
                          .getChildFile ("index.html");
        }
    }
    
    if (htmlFile.existsAsFile())
    {
        DBG ("Found UI file: " + htmlFile.getFullPathName());
        loadHTMLFile (htmlFile);
    }
    else
    {
        DBG ("UI file not found, showing fallback");
        showFallbackUI();
    }
}

void MainComponent::loadHTMLFile (const juce::File& htmlFile)
{
    if (!htmlFile.existsAsFile())
    {
        DBG ("HTML file does not exist: " + htmlFile.getFullPathName());
        showFallbackUI();
        return;
    }
    
    auto htmlContent = htmlFile.loadFileAsString();
    auto uiDir = htmlFile.getParentDirectory();
    
    // Inline CSS
    auto cssFile = uiDir.getChildFile ("styles.css");
    if (cssFile.existsAsFile())
    {
        auto cssContent = cssFile.loadFileAsString();
        
        // Replace relative background image paths with relative path for temp directory
        auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");
        auto backgroundsDir = projectRoot.getChildFile ("_Shared").getChildFile ("Assets").getChildFile ("backgrounds");
        
        // Find and replace background image URLs - copy to temp dir and use relative path
        juce::String searchPattern = "../../_Shared/Assets/backgrounds/hadron.png";
        auto imageFile = backgroundsDir.getChildFile ("hadron.png");
        
        if (imageFile.existsAsFile())
        {
            juce::String oldPattern = "url('" + searchPattern + "')";
            juce::String newPattern = "url('hadron.png')";
            cssContent = cssContent.replace (oldPattern, newPattern);
        }
        
        htmlContent = htmlContent.replace ("<link rel=\"stylesheet\" href=\"styles.css\">",
                                           "<style>" + cssContent + "</style>");
        htmlContent = htmlContent.replace ("<link rel='stylesheet' href='styles.css'>",
                                           "<style>" + cssContent + "</style>");
    }
    
    // Inline JavaScript files
    auto jsFile = uiDir.getChildFile ("app.js");
    if (jsFile.existsAsFile())
    {
        auto jsContent = jsFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<script src=\"app.js\"></script>",
                                           "<script>" + jsContent + "</script>");
        htmlContent = htmlContent.replace ("<script src='app.js'></script>",
                                           "<script>" + jsContent + "</script>");
    }
    
    // Inline shared CSS and JS from _Shared/UI
    auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");
    auto sharedDir = projectRoot.getChildFile ("_Shared").getChildFile ("UI");
    
    // Shared CSS
    auto sharedCSS = sharedDir.getChildFile ("mandelbrot-design-system.css");
    if (sharedCSS.existsAsFile())
    {
        auto sharedCSSContent = sharedCSS.loadFileAsString();
        
        // Replace logo image path with relative path for temp directory
        // Handle both single and double quotes
        juce::String logoPattern = "../../_Shared/Assets/logos/nnaudio-logo.png";
        juce::String logoOldPattern1 = "url('" + logoPattern + "')";
        juce::String logoOldPattern2 = "url(\"" + logoPattern + "\")";
        juce::String logoNewPattern = "url('nnaudio-logo.png')";
        sharedCSSContent = sharedCSSContent.replace (logoOldPattern1, logoNewPattern);
        sharedCSSContent = sharedCSSContent.replace (logoOldPattern2, logoNewPattern);
        
        int headEnd = htmlContent.indexOf ("</head>");
        if (headEnd > 0)
        {
            htmlContent = htmlContent.substring (0, headEnd) + 
                         "<style>" + sharedCSSContent + "</style>" +
                         htmlContent.substring (headEnd);
        }
    }
    
    // Shared JS - Animations
    auto animFile = sharedDir.getChildFile ("mandelbrot-animations.js");
    if (animFile.existsAsFile())
    {
        auto animContent = animFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/mandelbrot-animations.js\"></script>",
                                          "<script>" + animContent + "</script>");
    }
    
    // Shared JS - Particles
    auto particlesFile = sharedDir.getChildFile ("mandelbrot-particles.js");
    if (particlesFile.existsAsFile())
    {
        auto particlesContent = particlesFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/mandelbrot-particles.js\"></script>",
                                          "<script>" + particlesContent + "</script>");
    }
    
    // Shared JS - Components
    auto componentsFile = sharedDir.getChildFile ("mandelbrot-components.js");
    if (componentsFile.existsAsFile())
    {
        auto componentsContent = componentsFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/mandelbrot-components.js\"></script>",
                                          "<script>" + componentsContent + "</script>");
    }
    
    // Write to temp file and load
    auto tempDir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("MandelbrotSet");
    tempDir.createDirectory();
    
    // Copy background image to temp directory if it exists
    auto projectRootForImage = juce::File ("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");
    auto backgroundsDirForImage = projectRootForImage.getChildFile ("_Shared").getChildFile ("Assets").getChildFile ("backgrounds");
    auto imageFile = backgroundsDirForImage.getChildFile ("hadron.png");
    if (imageFile.existsAsFile())
    {
        auto tempImageFile = tempDir.getChildFile ("hadron.png");
        imageFile.copyFileTo (tempImageFile);
    }
    
    // Copy logo image to temp directory if it exists
    auto logosDir = projectRootForImage.getChildFile ("_Shared").getChildFile ("Assets").getChildFile ("logos");
    auto logoFile = logosDir.getChildFile ("nnaudio-logo.png");
    if (logoFile.existsAsFile())
    {
        auto tempLogoFile = tempDir.getChildFile ("nnaudio-logo.png");
        logoFile.copyFileTo (tempLogoFile);
    }
    
    auto tempFile = tempDir.getChildFile ("HadronUI.html");
    tempFile.replaceWithText (htmlContent);
    
    auto filePath = tempFile.getFullPathName().replace (" ", "%20");
    juce::String fileURL = "file://" + filePath;
    
    DBG ("Loading UI from: " + fileURL);
    if (webView != nullptr)
    {
        webView->setBounds (getLocalBounds());
        webView->toFront (false);
        webView->goToURL (fileURL);
        webView->setVisible (true);
        repaint();
        webView->repaint();
    }
}

void MainComponent::showFallbackUI()
{
    // Use a simple data URL instead of constructing a large string
    juce::String dataURL = "data:text/html;charset=utf-8,<!DOCTYPE html><html><head><meta charset='UTF-8'><title>UI Not Found</title></head><body style='background:#000;color:#fff;font-family:system-ui;padding:40px;text-align:center;'><h1>UI Files Not Found</h1><p>Please ensure UI/index.html exists</p></body></html>";
    
    if (webView != nullptr)
    {
        webView->goToURL (dataURL);
    }

}

void MainComponent::handleJavaScriptMessage (const juce::var& message)
{
    // Handle messages from JavaScript
    // Message can be a JSON string or already parsed var
    juce::var parsedMessage = message;
    
    // If it's a string, parse it as JSON
    if (message.isString())
    {
        auto result = juce::JSON::parse (message.toString(), parsedMessage);
        if (result.failed())
        {
            DBG ("Failed to parse message: " + result.getErrorMessage());
            return;
        }
    }
    
    // Extract message data
    if (auto* obj = parsedMessage.getDynamicObject())
    {
        auto type = obj->getProperty ("type").toString();
        auto value = obj->getProperty ("value");
        
        if (type == "volume")
        {
            auto volumeValue = (float) value;
            DBG ("Volume changed: " + juce::String (volumeValue));
            currentVolume = volumeValue / 100.0f; // Convert 0-100 to 0.0-1.0
        }
        else if (type == "frequency")
        {
            auto freqValue = (float) value;
            DBG ("Frequency changed: " + juce::String (freqValue));
            currentFrequency = (double) freqValue;
            // Update phase delta if audio is already started
            if (sampleRate > 0.0)
                phaseDelta = juce::MathConstants<double>::twoPi * currentFrequency / sampleRate;
        }
        else if (type == "detune")
        {
            auto detuneValue = (float) value;
            DBG ("Detune changed: " + juce::String (detuneValue));
            // TODO: Update detune
            // audioEngine.setDetune (detuneValue);
        }
        else if (type == "waveform")
        {
            auto waveformValue = value.toString();
            DBG ("Waveform changed: " + waveformValue);
            currentWaveform = waveformValue;
        }
        else if (type == "attack")
        {
            auto attackValue = (float) value;
            DBG ("Attack changed: " + juce::String (attackValue));
            // TODO: Update attack time
            // audioEngine.setAttack (attackValue / 1000.0f);
        }
        else if (type == "decay")
        {
            auto decayValue = (float) value;
            DBG ("Decay changed: " + juce::String (decayValue));
            // TODO: Update decay time
            // audioEngine.setDecay (decayValue / 1000.0f);
        }
        else if (type == "sustain")
        {
            auto sustainValue = (float) value;
            DBG ("Sustain changed: " + juce::String (sustainValue));
            // TODO: Update sustain level
            // audioEngine.setSustain (sustainValue / 100.0f);
        }
        else if (type == "release")
        {
            auto releaseValue = (float) value;
            DBG ("Release changed: " + juce::String (releaseValue));
            // TODO: Update release time
            // audioEngine.setRelease (releaseValue / 1000.0f);
        }
        else if (type == "cutoff")
        {
            auto cutoffValue = (float) value;
            DBG ("Cutoff changed: " + juce::String (cutoffValue));
            // TODO: Update filter cutoff
            // audioEngine.setCutoff (cutoffValue);
        }
        else if (type == "resonance")
        {
            auto resonanceValue = (float) value;
            DBG ("Resonance changed: " + juce::String (resonanceValue));
            // TODO: Update filter resonance
            // audioEngine.setResonance (resonanceValue);
        }
        else if (type == "filterType")
        {
            auto filterTypeValue = value.toString();
            DBG ("Filter type changed: " + filterTypeValue);
            // TODO: Update filter type
            // audioEngine.setFilterType (filterTypeValue);
        }
        else if (type == "playNote")
        {
            DBG ("Play note triggered");
            isPlaying = true;
            currentPhase = 0.0;
            // Update phase delta based on current frequency
            if (sampleRate > 0.0)
                phaseDelta = juce::MathConstants<double>::twoPi * currentFrequency / sampleRate;
        }
        else if (type == "stopNote")
        {
            DBG ("Stop note triggered");
            isPlaying = false;
        }
    }
}

void MainComponent::sendMessageToWebView (const juce::var& message)
{
    // Send message from C++ to JavaScript
    if (webView != nullptr)
    {
        webView->evaluateJavascript ("window.receiveMessageFromJUCE(" + 
                                     juce::JSON::toString (message) + ");");
    }
}

//==============================================================================
// Audio callback methods
void MainComponent::audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                                      int numInputChannels,
                                                      float* const* outputChannelData,
                                                      int numOutputChannels,
                                                      int numSamples,
                                                      const juce::AudioIODeviceCallbackContext& context)
{
    // Clear output buffers
    for (int channel = 0; channel < numOutputChannels; ++channel)
    {
        juce::FloatVectorOperations::clear (outputChannelData[channel], numSamples);
    }
    
    // Generate audio if playing
    if (isPlaying)
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float sampleValue = 0.0f;
            
            // Generate waveform based on current selection
            if (currentWaveform == "sine")
            {
                sampleValue = (float) std::sin (currentPhase);
            }
            else if (currentWaveform == "square")
            {
                sampleValue = (currentPhase < juce::MathConstants<double>::pi) ? 1.0f : -1.0f;
            }
            else if (currentWaveform == "sawtooth")
            {
                // Sawtooth: linear ramp from -1 to 1
                sampleValue = (float) ((currentPhase / juce::MathConstants<double>::twoPi) * 2.0 - 1.0);
            }
            else if (currentWaveform == "triangle")
            {
                // Triangle: linear ramp up then down
                if (currentPhase < juce::MathConstants<double>::pi)
                    sampleValue = (float) ((currentPhase / juce::MathConstants<double>::pi) * 2.0 - 1.0);
                else
                    sampleValue = (float) (3.0 - (currentPhase / juce::MathConstants<double>::pi) * 2.0);
            }
            else
            {
                // Default to sine
                sampleValue = (float) std::sin (currentPhase);
            }
            
            // Apply volume
            sampleValue *= currentVolume;
            
            // Write to all output channels
            for (int channel = 0; channel < numOutputChannels; ++channel)
            {
                outputChannelData[channel][sample] = sampleValue;
            }
            
            // Update phase
            currentPhase += phaseDelta;
            if (currentPhase >= juce::MathConstants<double>::twoPi)
                currentPhase -= juce::MathConstants<double>::twoPi;
        }
    }
}

void MainComponent::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    if (device != nullptr)
    {
        sampleRate = device->getCurrentSampleRate();
        phaseDelta = juce::MathConstants<double>::twoPi * currentFrequency / sampleRate;
        DBG ("Audio device started, sample rate: " + juce::String (sampleRate));
    }
}

void MainComponent::audioDeviceStopped()
{
    DBG ("Audio device stopped");
}

void MainComponent::audioDeviceError (const juce::String& errorMessage)
{
    DBG ("Audio device error: " + errorMessage);
}
