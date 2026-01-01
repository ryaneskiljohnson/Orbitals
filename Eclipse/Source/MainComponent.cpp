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
    setSize (1000, 800);
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
    
    addAndMakeVisible (webView.get());
    webView->setBounds (getLocalBounds());
    
    // Load a minimal black HTML page immediately to prevent white flash
    const char* blackHTML = 
        "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Loading</title>"
        "<style>html, body { background: #000000 !important; margin: 0; padding: 0; width: 100%; height: 100%; }</style>"
        "</head><body style='background: #000000; margin: 0; padding: 0; width: 100%; height: 100%;'></body></html>";
    
    juce::String htmlString (blackHTML);
    auto escapedHTML = juce::URL::addEscapeChars (htmlString, true);
    juce::String dataURL = "data:text/html;charset=utf-8," + escapedHTML;
    webView->goToURL (dataURL);
    webView->setVisible (true);
    
    DBG ("WebView created, bounds: " + getLocalBounds().toString());
    DBG ("Component size: " + juce::String (getWidth()) + "x" + juce::String (getHeight()));
    DBG ("Component visible: " + juce::String (isVisible() ? "YES" : "NO"));
    
    // Load UI after a brief delay to show black screen first
    juce::Timer::callAfterDelay (100, [this]()
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
    // Always paint black background initially
    g.fillAll (juce::Colours::black);
    
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
    auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals");
    auto htmlFile = projectRoot.getChildFile ("Eclipse").getChildFile ("UI").getChildFile ("index.html");
    
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

    // Inline shared CSS and JS from _Shared/UI
    auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals");
    auto sharedDir = projectRoot.getChildFile ("_Shared").getChildFile ("UI");
    
    // Shared CSS
    auto sharedCSS = sharedDir.getChildFile ("orbitals-design-system.css");
    if (sharedCSS.existsAsFile())
    {
        auto sharedCSSContent = sharedCSS.loadFileAsString();
        int headEnd = htmlContent.indexOf ("</head>");
        if (headEnd > 0)
        {
            htmlContent = htmlContent.substring (0, headEnd) + 
                         "<style>" + sharedCSSContent + "</style>" +
                         htmlContent.substring (headEnd);
        }
    }
    
    // Shared JS - Animations
    auto animFile = sharedDir.getChildFile ("orbitals-animations.js");
    if (animFile.existsAsFile())
    {
        auto animContent = animFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/orbitals-animations.js\"></script>",
                                      "<script>" + animContent + "</script>");
    }
    
    // Shared JS - Particles
    auto particlesFile = sharedDir.getChildFile ("orbitals-particles.js");
    if (particlesFile.existsAsFile())
    {
        auto particlesContent = particlesFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/orbitals-particles.js\"></script>",
                                      "<script>" + particlesContent + "</script>");
    }
    
    // Shared JS - Components
    auto componentsFile = sharedDir.getChildFile ("orbitals-components.js");
    if (componentsFile.existsAsFile())
    {
        auto componentsContent = componentsFile.loadFileAsString();
        htmlContent = htmlContent.replace ("<script src=\"../../_Shared/UI/orbitals-components.js\"></script>",
                                      "<script>" + componentsContent + "</script>");
    }
    
                                           "<script>" + jsContent + "</script>");
    }
    
    // Write to temp file and load
    auto tempDir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("Orbitals");
    tempDir.createDirectory();
    
    auto tempFile = tempDir.getChildFile ("EclipseUI.html");
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
