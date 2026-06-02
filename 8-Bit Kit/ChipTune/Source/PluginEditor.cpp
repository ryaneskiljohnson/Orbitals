/*
  ==============================================================================

    ChipTune - Classic Arcade Music Generator
    Video Game Music Generator Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../../../_Shared/Authentication/OrbitalsTemporaryAuthBypass.h"
#if JucePlugin_Build_Standalone
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

//==============================================================================
ChipTuneAudioProcessorEditor::ChipTuneAudioProcessorEditor (ChipTuneAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make component opaque so black background shows through (like NNAudioAccess)
    setOpaque(true);
    
    // Enable native title bar on the top-level window (for standalone builds)
    if (auto* top_level = juce::TopLevelWindow::getTopLevelWindow(0))
        top_level->setUsingNativeTitleBar(true);

    setSize (1200, 750);
    setResizable (false, false);
    
    // Enable keyboard focus
    setWantsKeyboardFocus(true);
    setMouseClickGrabsKeyboardFocus(true);
    
    OrbitalsWebViewHostConfig webConfig;
    webConfig.pluginDisplayName = "ChipTune";
    webConfig.devPluginDirectory = juce::File (__FILE__).getParentDirectory().getParentDirectory();

    webViewHost.initialize (*this, webConfig, [this](const juce::var& msg)
    {
        handleJavaScriptMessage (msg);
    });

    isAuthorized = checkAuthorization();

    if (isAuthorized)
        startTimer (50);
    else
        startTimer (5000);

    juce::Timer::callAfterDelay (300, [this]()
    {
        webViewHost.dispatchInitialNavigation (isAuthorized);
    });
    
    // Try to grab focus after a short delay
    juce::Timer::callAfterDelay(500, [this]() {
        DBG("Attempting to grab keyboard focus after delay");
        grabKeyboardFocus();
        DBG(juce::String("Focus grabbed: ") + (hasKeyboardFocus(true) ? "true" : "false"));
    });
}

ChipTuneAudioProcessorEditor::~ChipTuneAudioProcessorEditor()
{
    webViewHost.shutdown();
}

void ChipTuneAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black); // Fill with black background to prevent white flash (like NNAudioAccess)
}

void ChipTuneAudioProcessorEditor::resized()
{
    webViewHost.resized (*this);
}

void ChipTuneAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
{
    juce::var msg = message;
    if (msg.isString())
    {
        juce::var parsed = juce::JSON::parse (msg.toString());
        if (parsed.isObject())
            msg = parsed;
    }
    if (! msg.isObject())
        return;

    auto obj = msg.getDynamicObject();
    if (obj == nullptr)
        return;

    if (obj->hasProperty ("payload") && obj->getProperty ("payload").isObject())
    {
        msg = obj->getProperty ("payload");
        obj = msg.getDynamicObject();
        if (obj == nullptr)
            return;
    }

    auto type = obj->getProperty ("type").toString();

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
    else if (type == "playNote")
    {
        const int note = juce::jlimit (0, 127, (int) obj->getProperty ("note"));
        const int velocity = juce::jlimit (1, 127, (int) obj->getProperty ("velocity"));
        audioProcessor.queueMidiNote (note, velocity);
    }
    else if (type == "stopNote")
    {
        const int note = juce::jlimit (0, 127, (int) obj->getProperty ("note"));
        audioProcessor.queueMidiNote (note, 0);
    }
    else if (type == "openSettings")
    {
        openAudioSettings();
    }
}

//==============================================================================

//==============================================================================
bool ChipTuneAudioProcessorEditor::checkAuthorization()
{
    if (orbitals_auth_bypass_detail::isBypassActive())
        return true;

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
            webViewHost.reloadSurface (true);
            startTimer(50); // Check every 15 minutes
        }
        else
        {
            webViewHost.reloadSurface (false);
            startTimer(5000); // Check every 5 seconds
        }
    }
    
    return isAuthorized;
}

//==============================================================================
juce::File ChipTuneAudioProcessorEditor::getAuthFile()
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
juce::String ChipTuneAudioProcessorEditor::loadAndDecryptLicenseFile()
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
void ChipTuneAudioProcessorEditor::timerCallback()
{
    if (!isAuthorized)
    {
        bool newAuthState = checkAuthorization();
        if (newAuthState != isAuthorized)
        {
            isAuthorized = newAuthState;
            if (isAuthorized)
            {
                webViewHost.reloadSurface (true);
            }
        }
    }
    
            
    
    // Always send metering data when authorized and visible (fast update for animation)
    if (isAuthorized && webViewHost.isVisible())
    {
        sendMeteringData();
    }
}

void ChipTuneAudioProcessorEditor::sendMeteringData()
{
    float inputLevelDb = audioProcessor.inputLevel.load();
    float outputLevelDb = audioProcessor.outputLevel.load();

    juce::String script = juce::String::formatted(
        "if (window.receiveAudioData) { window.receiveAudioData({ inputLevel: %.2f, outputLevel: %.2f }); }",
        inputLevelDb, outputLevelDb
    );

    if (webViewHost.isVisible())
        webViewHost.evaluateJavascript (script);
}

void ChipTuneAudioProcessorEditor::openAudioSettings()
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

void ChipTuneAudioProcessorEditor::notifyMIDINote(int noteNumber, int velocity)
{
    if (webViewHost.isInitialized() && isAuthorized)
    {
        juce::String script = "if (window.receiveMessageFromJUCE) { window.receiveMessageFromJUCE({ type: 'midiNote', note: " 
            + juce::String(noteNumber) + ", velocity: " + juce::String(velocity) + " }); }";
        webViewHost.evaluateJavascript (script);
    }
}

void ChipTuneAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
    DBG("Mouse clicked - grabbing keyboard focus");
    grabKeyboardFocus();
    DBG(juce::String("Has focus after click: ") + (hasKeyboardFocus(true) ? "true" : "false"));
}

bool ChipTuneAudioProcessorEditor::keyPressed(const juce::KeyPress& key)
{
    DBG("=== KEY PRESSED IN C++ (ChipTune) ===");
    DBG(juce::String("Key code: ") + juce::String(key.getKeyCode()));
    DBG(juce::String("Text character: ") + juce::String(key.getTextCharacter()));
    DBG(juce::String("Has keyboard focus: ") + (hasKeyboardFocus(true) ? "true" : "false"));
    
    if (webViewHost.isVisible())
    {
        // Check for spacebar
        if (key.getKeyCode() == juce::KeyPress::spaceKey)
        {
            DBG("Spacebar pressed - sending jump command to JavaScript");
            juce::String script = "if (window.handleKeyFromJUCE) { window.handleKeyFromJUCE('Space'); } else if (window.handleJump) { window.handleJump(); }";
            webViewHost.evaluateJavascript(script);
            return true;
        }
    }
    
    return false;
}
