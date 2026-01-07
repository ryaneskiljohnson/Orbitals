# Complete Migration Plan - All Plugins to Fabric Architecture

## 🎯 Objective

Migrate ALL 17 plugins (8 Mandelbrot + 9 Orbital) to use:
1. ✅ JUCE 8 WebView with event listeners (modern API)
2. ✅ PluginEditor for standalone (remove MainComponent if present)
3. ✅ Instant parameter response (skip() method)
4. ✅ Proper audio input support (mono + stereo)
5. ✅ Microphone permissions
6. ✅ Complete resource inlining

---

## 📋 Changes Required Per Plugin

### 1. Remove MainComponent Files (If Present)
**Check for**:
- `Source/Main.cpp`
- `Source/MainComponent.h`
- `Source/MainComponent.cpp`

**Action**: 
- Delete these files
- Remove from `project.pbxproj`
- Let JUCE generate proper plugin standalone

### 2. Update WebView API (PluginEditor.cpp)
**Old**:
```cpp
webView = std::make_unique<juce::WebBrowserComponent>();
```

**New**:
```cpp
auto options = juce::WebBrowserComponent::Options{}
    .withNativeIntegrationEnabled(true)
    .withKeepPageLoadedWhenBrowserIsHidden()
    .withEventListener("message", [this](const juce::var& message) {
        handleJavaScriptMessage(message);
    });

webView = std::make_unique<WebBrowserComponent>(options);
```

### 3. Fix Parameter Handler (PluginEditor.cpp)
**Unified handler for all parameters**:
```cpp
void handleJavaScriptMessage(const juce::var& message)
{
    if (!message.isObject()) return;
    auto obj = message.getDynamicObject();
    if (obj == nullptr) return;
    
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
```

### 4. Use evaluateJavascript (PluginEditor.cpp)
**For C++ → JS communication**:
```cpp
// OLD: webView->emitEventIfBrowserIsVisible("eval", script);
// NEW:
webView->evaluateJavascript(script);
```

### 5. Fix Parameter Smoothing (PluginProcessor.cpp)
**If plugin uses SmoothedValue**:
```cpp
// OLD: float value = smoothed.getNextValue();
// NEW:
float value = smoothed.skip(numSamples);
```

### 6. Fix Bus Layout (Mandelbrot Plugins Only)
**Already done via script** ✅

### 7. Add Microphone Permissions
**Already done via script** ✅

---

## 🔍 Plugin Status Assessment

Let me check each plugin...
