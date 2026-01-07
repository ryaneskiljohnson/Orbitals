# ORBITALS - JUCE Implementation Guide

**How the plugins use JUCE framework correctly**

---

## ✅ JUCE Components Used

### Core JUCE Classes

1. **`juce::AudioProcessor`** - Base class for all plugins
   - PerihelionProcessor inherits from this
   - Handles plugin lifecycle, state, parameters

2. **`juce::AudioProcessorEditor`** - Plugin UI base class
   - PerihelionEditor inherits from this
   - Manages plugin window and UI components

3. **`juce::AudioProcessorValueTreeState`** - Parameter management
   - Thread-safe parameter system
   - Automatic state save/load
   - Host automation support

4. **`juce::WebBrowserComponent`** - WebView for HTML/CSS/JS UI
   - JUCE 8+ feature
   - Embeds web content in plugin window
   - Bidirectional JavaScript ↔ C++ communication

5. **`juce::MidiBuffer`** - MIDI message processing
   - Thread-safe MIDI handling
   - Process MIDI in processBlock()

---

## 🔧 JUCE Plugin Structure

### Standard JUCE Plugin Entry Point

```cpp
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PerihelionProcessor();
}
```

This is JUCE's standard plugin factory function. The `juce_add_plugin()` CMake function automatically generates the wrapper code that calls this.

### Parameter System

**Using JUCE's AudioProcessorValueTreeState**:

```cpp
// In constructor
parameters (*this, nullptr, juce::Identifier ("Perihelion"), createParameterLayout())

// Parameter layout
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back (std::make_unique<juce::AudioParameterFloat> (...));
    return { params.begin(), params.end() };
}
```

**Reading parameters (thread-safe)**:
```cpp
auto* param = parameters.getRawParameterValue (PARAM_GRAVITY);
float value = param->load();  // Atomic load
```

**Setting parameters (from UI thread)**:
```cpp
auto* param = parameters.getParameter (PARAM_GRAVITY);
auto range = parameters.getParameterRange (PARAM_GRAVITY);
param->setValueNotifyingHost (range.convertTo0to1 (value));
```

---

## 🌐 WebView Integration (JUCE 8+)

### Creating WebView

```cpp
auto options = juce::WebBrowserComponent::Options{}
    .withNativeIntegrationEnabled (true)
    .withEventListener ("message", [this](const juce::var& message) {
        handleJavaScriptMessage (message);
    });

webView = std::make_unique<juce::WebBrowserComponent> (options);
```

### JavaScript → JUCE Communication

**JavaScript**:
```javascript
window.chrome.webview.postMessage({
    type: 'parameterChange',
    parameter: 'gravity',
    value: 75.0
});
```

**JUCE C++**:
```cpp
void handleJavaScriptMessage (const juce::var& message)
{
    if (auto* obj = message.getDynamicObject())
    {
        auto type = obj->getProperty ("type").toString();
        if (type == "parameterChange")
        {
            auto param = obj->getProperty ("parameter").toString();
            auto value = (float) obj->getProperty ("value");
            updateParameterFromUI (param, value);
        }
    }
}
```

### JUCE → JavaScript Communication

```cpp
void sendMessageToWebView (const juce::var& message)
{
    if (webView != nullptr)
    {
        webView->evaluateJavaScript (
            "window.receiveMessageFromJUCE(" + 
            juce::JSON::toString (message) + 
            ");"
        );
    }
}
```

---

## 🎹 MIDI Processing (JUCE Standard)

### processBlock() - JUCE's Audio Callback

```cpp
void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
{
    // Clear audio (MIDI FX doesn't process audio)
    buffer.clear();
    
    // Process MIDI
    juce::MidiBuffer processedMidi;
    
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        // ... process MIDI ...
        processedMidi.addEvent (message, metadata.samplePosition);
    }
    
    midiMessages.swapWith (processedMidi);
}
```

This is JUCE's standard MIDI processing pattern.

---

## 📦 CMake Integration (JUCE's Build System)

### Using juce_add_plugin()

```cmake
juce_add_plugin(Perihelion
    COMPANY_NAME "NNAudio"
    PLUGIN_MANUFACTURER_CODE "NNAu"
    PLUGIN_CODE "Prhl"
    FORMATS VST3 AU
    PRODUCT_NAME "Perihelion"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    ...
)
```

This JUCE CMake function:
- Generates plugin entry point code
- Sets up VST3/AU wrappers
- Configures platform-specific builds
- Handles code signing (macOS)
- Creates proper plugin bundles

---

## 🔒 Thread Safety

### JUCE's Thread Model

1. **Audio Thread** (`processBlock()`):
   - Only read parameters using `getRawParameterValue()->load()`
   - Never call UI functions
   - Never allocate memory

2. **Message Thread** (UI, WebView callbacks):
   - Can safely update parameters
   - Use `setValueNotifyingHost()` for parameter updates
   - UI operations are safe here

3. **Parameter Updates**:
   - WebView callbacks are on message thread ✅
   - Parameter updates are thread-safe ✅
   - Host automation works automatically ✅

---

## ✅ JUCE Compliance Checklist

- [x] Inherits from `juce::AudioProcessor`
- [x] Implements all required virtual methods
- [x] Uses `juce::AudioProcessorValueTreeState` for parameters
- [x] Uses `juce::AudioProcessorEditor` for UI
- [x] Uses `juce::WebBrowserComponent` for WebView
- [x] Implements `createPluginFilter()` entry point
- [x] Thread-safe parameter access
- [x] Proper state save/load
- [x] MIDI processing in `processBlock()`
- [x] Uses `juce_add_plugin()` CMake function
- [x] Follows JUCE plugin conventions

---

## 📚 JUCE Documentation References

- **AudioProcessor**: https://juce.com/doc/classAudioProcessor
- **AudioProcessorEditor**: https://juce.com/doc/classAudioProcessorEditor
- **AudioProcessorValueTreeState**: https://juce.com/doc/classAudioProcessorValueTreeState
- **WebBrowserComponent**: https://juce.com/doc/classWebBrowserComponent
- **MidiBuffer**: https://juce.com/doc/classMidiBuffer

---

## 🎯 Key JUCE Features Used

1. **Parameter System**: APVTS for thread-safe parameters
2. **WebView**: JUCE 8's WebBrowserComponent
3. **MIDI Processing**: Standard processBlock() pattern
4. **State Management**: XML-based save/load
5. **Plugin Formats**: VST3 + AU via juce_add_plugin()
6. **Thread Safety**: Proper atomic parameter access

---

**All code follows JUCE best practices and conventions! ✅**

*Orbitals MIDI FX Plugin Line - JUCE Implementation*
*© NNAudio - Premium Music Production Tools*
