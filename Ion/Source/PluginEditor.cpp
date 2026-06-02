/*
  ==============================================================================

    Ion - Timing Stabilizer
    MIDI FX Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../../_Shared/Authentication/OrbitalsTemporaryAuthBypass.h"

//==============================================================================
IonAudioProcessorEditor::IonAudioProcessorEditor (IonAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make component opaque so black background shows through (like NNAudioAccess)
    setOpaque(true);
    
    // Enable native title bar on the top-level window (for standalone builds)
    if (auto* top_level = juce::TopLevelWindow::getTopLevelWindow(0))
        top_level->setUsingNativeTitleBar(true);

    setSize (1200, 750);
    setResizable (false, false);
    
    OrbitalsWebViewHostConfig webConfig;
    webConfig.pluginDisplayName = "Ion";
    webConfig.devPluginDirectory = juce::File (__FILE__).getParentDirectory().getParentDirectory();

    webViewHost.initialize (*this, webConfig, [this](const juce::var& msg)
    {
        handleJavaScriptMessage (msg);
    });

    isAuthorized = checkAuthorization();

    if (isAuthorized)
        startTimer (900000);
    else
        startTimer (5000);

    juce::Timer::callAfterDelay (300, [this]()
    {
        webViewHost.dispatchInitialNavigation (isAuthorized);
    });
}

IonAudioProcessorEditor::~IonAudioProcessorEditor()
{
    webViewHost.shutdown();
}

void IonAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black); // Fill with black background to prevent white flash (like NNAudioAccess)
}

void IonAudioProcessorEditor::resized()
{
    webViewHost.resized (*this);
}



void IonAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
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

auto type = obj->getProperty("type").toString();
    
        if (type == "parameterChange")
    {
        auto param = obj->getProperty ("parameter").toString();
        auto value = obj->getProperty ("value");

        auto setRawParameter = [this] (const juce::String& paramId, float rawValue)
        {
            if (auto* p = audioProcessor.parameters.getParameter (paramId))
            {
                auto& range = p->getNormalisableRange();
                rawValue = juce::jlimit (range.start, range.end, rawValue);
                p->setValueNotifyingHost (range.convertTo0to1 (rawValue));
            }
        };

        if (param == "scale")
        {
            const juce::String scale = value.toString().toLowerCase();
            int scaleIndex = 0;

            if (scale == "major")           scaleIndex = 0;
            else if (scale == "minor")      scaleIndex = 1;
            else if (scale == "dorian")     scaleIndex = 2;
            else if (scale == "phrygian")   scaleIndex = 3;
            else if (scale == "lydian")     scaleIndex = 4;
            else if (scale == "mixolydian") scaleIndex = 5;
            else if (scale == "aeolian")    scaleIndex = 6;
            else if (scale == "locrian")    scaleIndex = 7;
            else if (scale == "chromatic")  scaleIndex = 8;

            setRawParameter (IonAudioProcessor::PARAM_SCALE, static_cast<float> (scaleIndex));
            return;
        }

        if (param == "attractionType")
        {
            const juce::String typeName = value.toString().toLowerCase();
            int typeIndex = 0;

            if (typeName == "snap")       typeIndex = 0;
            else if (typeName == "pull")  typeIndex = 1;
            else if (typeName == "guide") typeIndex = 2;

            setRawParameter (IonAudioProcessor::PARAM_ATTRACTION_TYPE, static_cast<float> (typeIndex));
            return;
        }

        setRawParameter (param, static_cast<float> (value));
    }
}

//==============================================================================

//==============================================================================
bool IonAudioProcessorEditor::checkAuthorization()
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
    
    bool authorized = (expiration_date > juce::Time::getCurrentTime() && product_list.contains("200006"));
    
    if (authorized != isAuthorized)
    {
        isAuthorized = authorized;
        if (isAuthorized)
        {
            webViewHost.reloadSurface (true);
            startTimer(1000 * 60 * 15); // Check every 15 minutes
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
juce::File IonAudioProcessorEditor::getAuthFile()
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
juce::String IonAudioProcessorEditor::loadAndDecryptLicenseFile()
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
void IonAudioProcessorEditor::timerCallback()
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
                startTimer(1000 * 60 * 15);
            }
        }
    }
}

void IonAudioProcessorEditor::notifyMIDINote(int noteNumber, int velocity)
{
    if (webViewHost.isInitialized() && isAuthorized)
    {
        juce::String script = "if (window.receiveMessageFromJUCE) { window.receiveMessageFromJUCE({ type: 'midiNote', note: " 
            + juce::String(noteNumber) + ", velocity: " + juce::String(velocity) + " }); }";
        webViewHost.evaluateJavascript (script);
    }
}
