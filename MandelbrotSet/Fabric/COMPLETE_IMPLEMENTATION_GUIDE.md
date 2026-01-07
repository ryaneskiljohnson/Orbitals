# Fabric Reverb Plugin - Complete Implementation Guide

## 🎉 Final Status: FULLY FUNCTIONAL

This document chronicles the complete process of implementing Fabric, from initial build to a fully functional JUCE 8 reverb plugin with modern WebView UI and comprehensive DSP.

---

## 📋 Table of Contents

1. [Overview](#overview)
2. [Build System Configuration](#build-system-configuration)
3. [WebView UI Integration](#webview-ui-integration)
4. [DSP Implementation](#dsp-implementation)
5. [JavaScript ↔ C++ Bridge](#javascript--c-bridge)
6. [Audio Routing & Device Management](#audio-routing--device-management)
7. [Parameter Smoothing & Response](#parameter-smoothing--response)
8. [Standalone Application Architecture](#standalone-application-architecture)
9. [Issues Encountered & Solutions](#issues-encountered--solutions)
10. [Applying to Other Plugins](#applying-to-other-plugins)

---

## Overview

### What We Built
- **Plugin Type**: Audio Effects (Reverb)
- **Formats**: VST3, AU, Standalone
- **UI Technology**: JUCE 8 WebBrowserComponent (modern WebView with native integration)
- **DSP Engine**: juce::dsp::Reverb with custom parameter mapping
- **Communication**: JavaScript (UI) ↔ C++ (DSP) via JUCE event listeners

### Key Technologies
- **JUCE 8.0.9**: Modern C++ audio framework
- **WebView**: HTML/CSS/JavaScript UI embedded in native app
- **juce::dsp::Reverb**: Professional reverb DSP module
- **AudioProcessorValueTreeState**: Parameter management
- **SmoothedValue**: Parameter smoothing for audio-rate changes

---

## Build System Configuration

### 1. Xcode Project Setup

#### Header Search Paths
**Problem**: VST3 SDK header conflicts between global and local JUCE installations

**Solution**: Updated `project.pbxproj` to use local VST3 SDK:
```
HEADER_SEARCH_PATHS = (
    "$(inherited)",
    "$(SRCROOT)/../../JuceLibraryCode",
    "$(SRCROOT)/../../JuceLibraryCode/modules/juce_audio_processors/format_types/VST3_SDK"
);
```

#### Plugin Configuration (.jucer file)
```xml
projectType="audioplug"
pluginFormats="buildAU,buildVST3,buildStandalone"
pluginCharacteristicsValue="pluginIsFx,pluginWantsMidiInput"
```

**Key**: `pluginIsFx` marks it as an audio effect (not synth/MIDI effect)

---

## WebView UI Integration

### 1. Modern JUCE 8 WebView API

**Old API (JUCE 6/7)**:
```cpp
webView = std::make_unique<juce::WebBrowserComponent>();
```

**New API (JUCE 8)**:
```cpp
auto options = juce::WebBrowserComponent::Options{}
    .withNativeIntegrationEnabled(true)
    .withKeepPageLoadedWhenBrowserIsHidden()
    .withEventListener("message", [this](const juce::var& message) {
        handleJavaScriptMessage(message);
    });

webView = std::make_unique<WebBrowserComponent>(options);
```

### 2. Resource Inlining Strategy

**Why**: Avoid CORS issues and ensure all resources load correctly

**Implementation** (`PluginEditor.cpp`):

```cpp
// 1. Load HTML file
auto htmlContent = htmlFile.loadFileAsString();

// 2. Inline CSS files
auto cssContent = cssFile.loadFileAsString();
htmlContent = htmlContent.replace(
    "<link rel=\"stylesheet\" href=\"styles.css\">",
    "<style>" + cssContent + "</style>"
);

// 3. Inline JavaScript files
auto jsContent = jsFile.loadFileAsString();
htmlContent = htmlContent.replace(
    "<script src=\"app.js\"></script>",
    "<script>" + jsContent + "</script>"
);

// 4. Convert images to Base64 data URLs
juce::MemoryBlock imageData;
backgroundImage.loadFileAsData(imageData);
juce::String base64 = juce::Base64::toBase64(imageData.getData(), imageData.getSize());
htmlContent = htmlContent.replace(
    "background.png",
    "data:image/png;base64," + base64
);

// 5. Write to temp file and load
auto tempFile = tempDir.getChildFile("index.html");
tempFile.replaceWithText(htmlContent);
webView->goToURL("file://" + tempFile.getFullPathName());
```

### 3. JavaScript Files to Inline

**MandelbrotSet Plugins**:
- `mandelbrot-components.js` - Knobs, sliders, XY pads
- `mandelbrot-animations.js` - Particle effects, visual effects
- `mandelbrot-particles.js` - Background particle systems
- `mandelbrot-design-system.css` - Shared styling

**Orbital Plugins**:
- `orbitals-components.js`
- `orbitals-animations.js`
- `orbitals-particles.js`
- `orbitals-design-system.css`

---

## DSP Implementation

### 1. Core DSP Components

**Header** (`PluginProcessor.h`):
```cpp
class FabricAudioProcessor : public juce::AudioProcessor
{
private:
    // Reverb DSP
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> predelayLineL;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> predelayLineR;
    double currentSampleRate = 44100.0;
    
    // Parameter smoothing
    juce::SmoothedValue<float> smoothedSize;
    juce::SmoothedValue<float> smoothedDiffusion;
    juce::SmoothedValue<float> smoothedDamping;
    juce::SmoothedValue<float> smoothedPredelay;
    juce::SmoothedValue<float> smoothedMix;
    juce::SmoothedValue<float> smoothedWetDry;
    
    // Parameter management
    juce::AudioProcessorValueTreeState parameters;
};
```

### 2. Parameter Definitions

**All parameters use `AudioProcessorValueTreeState`**:

```cpp
params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID(PARAM_SIZE, 1),
    "Weave",                          // Display name
    juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
    50.0f,                            // Default value
    juce::String(),
    juce::AudioProcessorParameter::genericParameter,
    [](float value, int) { return juce::String((int)value) + "%"; }  // Display formatter
));
```

**6 Parameters**:
1. **Size** (Weave) - Room size → `reverbParams.roomSize`
2. **Diffusion** (Thread Density) - Stereo width → `reverbParams.width`
3. **Damping** (Fold) - High-freq absorption → `reverbParams.damping`
4. **Predelay** (Time Dilation) - Initial delay (0-200ms)
5. **Mix** (Warp) - Overall reverb intensity
6. **Wet/Dry** - Dry/wet balance
7. **Bypass** (bool) - Bypass all processing

### 3. DSP Initialization (`prepareToPlay`)

```cpp
void FabricAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    
    // Prepare reverb
    reverb.prepare(spec);
    
    // Initialize reverb parameters to default values
    reverbParams.roomSize = 0.5f;
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = 0.5f;
    reverbParams.dryLevel = 0.5f;
    reverbParams.width = 0.5f;
    reverbParams.freezeMode = 0.0f;
    reverb.setParameters(reverbParams);
    
    // Initialize predelay lines
    int maxDelaySamples = static_cast<int>(sampleRate * 0.2); // 200ms max
    predelayLineL.prepare(spec);
    predelayLineL.setMaximumDelayInSamples(maxDelaySamples);
    predelayLineL.setDelay(0);
    
    predelayLineR.prepare(spec);
    predelayLineR.setMaximumDelayInSamples(maxDelaySamples);
    predelayLineR.setDelay(0);
    
    // Initialize parameter smoothing (1ms for responsive controls)
    smoothedSize.reset(sampleRate, 0.02);
    smoothedDiffusion.reset(sampleRate, 0.02);
    smoothedDamping.reset(sampleRate, 0.02);
    smoothedPredelay.reset(sampleRate, 0.02);
    smoothedMix.reset(sampleRate, 0.001);    // Faster response
    smoothedWetDry.reset(sampleRate, 0.001); // Faster response
    
    // Set initial values from parameters
    smoothedSize.setCurrentAndTargetValue(*parameters.getRawParameterValue(PARAM_SIZE) / 100.0f);
    smoothedDiffusion.setCurrentAndTargetValue(*parameters.getRawParameterValue(PARAM_DIFFUSION) / 100.0f);
    smoothedDamping.setCurrentAndTargetValue(*parameters.getRawParameterValue(PARAM_DAMPING) / 100.0f);
    smoothedPredelay.setCurrentAndTargetValue(*parameters.getRawParameterValue(PARAM_PREDELAY));
    smoothedMix.setCurrentAndTargetValue(*parameters.getRawParameterValue(PARAM_MIX) / 100.0f);
    smoothedWetDry.setCurrentAndTargetValue(*parameters.getRawParameterValue(PARAM_WETDRY) / 100.0f);
}
```

### 4. Audio Processing (`processBlock`)

```cpp
void FabricAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Get parameters
    float rawSize = *parameters.getRawParameterValue(PARAM_SIZE);
    float rawWetDry = *parameters.getRawParameterValue(PARAM_WETDRY);
    // ... all parameters
    
    // Update smoothed values
    smoothedSize.setTargetValue(rawSize / 100.0f);
    smoothedWetDry.setTargetValue(rawWetDry / 100.0f);
    // ...
    
    // Get smoothed values using skip() for instant full-buffer response
    float size = smoothedSize.skip(numSamples);
    float wetDry = smoothedWetDry.skip(numSamples);
    // ...
    
    // Map parameters to reverb
    float roomSize = std::pow(size, 1.5f); // Exponential scaling
    float width = 0.5f + (diffusion * 0.5f); // 0.5-1.0 range
    
    reverbParams.roomSize = roomSize;
    reverbParams.width = width;
    reverbParams.damping = damping;
    
    // Wet/dry mixing
    float reverbIntensity = mix;
    reverbParams.wetLevel = reverbIntensity * wetDry;
    reverbParams.dryLevel = reverbIntensity * (1.0f - wetDry) + (1.0f - reverbIntensity);
    
    // Apply parameters
    reverb.setParameters(reverbParams);
    
    // Apply predelay if needed
    if (predelayMs > 0.5f) {
        // Process through delay lines
    }
    
    // Process reverb
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
}
```

### 5. Parameter Mapping Philosophy

**Size (Weave)**: Exponential scaling for musical feel
- 0% → Small room (0.0)
- 50% → Medium hall (0.35)
- 100% → Cathedral (1.0)
- Formula: `pow(size, 1.5)`

**Diffusion (Thread Density)**: Stereo width control
- 0% → Narrow (0.5 = mostly mono)
- 50% → Normal (0.75)
- 100% → Wide (1.0 = full stereo)
- Formula: `0.5 + (diffusion * 0.5)`

**Mix & Wet/Dry**: Two-stage mixing
- **Mix (Warp)**: Overall reverb intensity (0 = no reverb, 100 = full reverb)
- **Wet/Dry**: Balance within that intensity (0 = all dry, 100 = all wet)
- Formula: 
  - `wetLevel = mix * wetDry`
  - `dryLevel = mix * (1 - wetDry) + (1 - mix)`
  - Ensures: `wetLevel + dryLevel = 1.0` (unity gain)

---

## JavaScript ↔ C++ Bridge

### 1. JUCE 8 Event Listener Setup (C++)

**Modern withEventListener API**:

```cpp
auto options = juce::WebBrowserComponent::Options{}
    .withNativeIntegrationEnabled(true)
    .withKeepPageLoadedWhenBrowserIsHidden()
    .withEventListener("message", [this](const juce::var& message) {
        handleJavaScriptMessage(message);
    });

webView = std::make_unique<WebBrowserComponent>(options);
```

### 2. Message Handler (C++)

```cpp
void FabricAudioProcessorEditor::handleJavaScriptMessage(const juce::var& message)
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
    else if (type == "openSettings")
    {
        openAudioSettings();
    }
}
```

### 3. JavaScript Message Sending

**Using JUCE 8 emitEvent API**:

```javascript
function sendToPlugin(parameter, value) {
    const juceBackend = window.__JUCE__;
    
    if (juceBackend && juceBackend.backend) {
        const message = {
            type: 'parameterChange',
            parameter: parameter,
            value: value
        };
        juceBackend.backend.emitEvent('message', message);
    }
}
```

### 4. C++ → JavaScript Communication

```cpp
// Send metering data to UI
juce::String script = "if(window.updateMetering) { window.updateMetering(" +
    juce::String(inputLevel.load()) + "," +
    juce::String(outputLevel.load()) + "); }";
webView->evaluateJavascript(script);
```

**Key**: Use `evaluateJavascript()`, not `emitEventIfBrowserIsVisible()`

---

## Audio Routing & Device Management

### 1. Bus Configuration

```cpp
AudioProcessor(BusesProperties()
    #if ! JucePlugin_IsMidiEffect
     #if ! JucePlugin_IsSynth
      .withInput("Input", juce::AudioChannelSet::stereo(), true)
     #endif
      .withOutput("Output", juce::AudioChannelSet::stereo(), true)
    #endif
)
```

### 2. Bus Layout Support

```cpp
bool isBusesLayoutSupported(const BusesLayout& layouts) const override
{
    // Accept mono or stereo input
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // Output must be stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return true;
}
```

**Critical**: Must accept both mono (microphone) and stereo (audio interface) inputs!

### 3. Standalone Audio Settings

**Opening audio device dialog**:

```cpp
void openAudioSettings()
{
#if JucePlugin_Build_Standalone
    juce::MessageManager::callAsync([this]()
    {
        if (auto* pluginHolder = juce::StandalonePluginHolder::getInstance())
        {
            pluginHolder->showAudioSettingsDialog();
        }
    });
#else
    // In plugin mode (VST3/AU), show info message
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        "Audio/MIDI Settings",
        "Audio and MIDI device settings are managed by your DAW.",
        "OK"
    );
#endif
}
```

### 4. macOS Permissions

**Info.plist entry required**:
```xml
<key>NSMicrophoneUsageDescription</key>
<string>Fabric needs microphone access to process audio input for reverb effects.</string>
```

**Add via plutil**:
```bash
plutil -insert NSMicrophoneUsageDescription -string "Fabric needs microphone access..." Info.plist
```

Or add to `.jucer`:
```xml
<XCODE_MAC ... microphonePermissionNeeded="1" microphonePermissionsText="...">
```

---

## Parameter Smoothing & Response

### Problem
Parameters with 20ms smoothing felt laggy, especially mix and wet/dry controls

### Solution 1: Faster Smoothing for Critical Parameters

```cpp
// Reverb characteristics: 20ms (smooth, no zipper noise)
smoothedSize.reset(sampleRate, 0.02);
smoothedDiffusion.reset(sampleRate, 0.02);
smoothedDamping.reset(sampleRate, 0.02);
smoothedPredelay.reset(sampleRate, 0.02);

// Mix controls: 1ms (instant response, still smooth)
smoothedMix.reset(sampleRate, 0.001);
smoothedWetDry.reset(sampleRate, 0.001);
```

### Solution 2: Skip() for Full-Buffer Application

**Instead of per-sample**:
```cpp
// Old (laggy):
for (int sample = 0; sample < numSamples; ++sample) {
    float mix = smoothedMix.getNextValue();
}
```

**New (instant)**:
```cpp
// Apply smoothing across entire buffer at once
float mix = smoothedMix.skip(numSamples);
```

This makes parameters respond much faster while still preventing zipper noise.

---

## Standalone Application Architecture

### Problem
Initially had conflicting standalone application architectures:
- Custom `Main.cpp` + `MainComponent.cpp` (synth template)
- JUCE auto-generated plugin standalone wrapper

### Solution
**Removed custom standalone files**, let JUCE generate proper plugin standalone:

1. Deleted `Main.cpp`, `MainComponent.h`, `MainComponent.cpp`
2. Removed references from `project.pbxproj`
3. Used Projucer to regenerate `.jucer` file
4. JUCE now generates correct `juce_audio_plugin_client_Standalone.cpp`

**Result**: Proper standalone wrapper with:
- `StandalonePluginHolder` working correctly
- Audio device manager accessible
- Settings dialog functional

---

## Issues Encountered & Solutions

### Issue 1: VST3 SDK Header Conflicts
**Error**: `redefinition of enumerator 'kNoInterface'`

**Cause**: Global JUCE path conflicting with local JUCE

**Solution**: Remove global JUCE paths, use only local VST3 SDK path

---

### Issue 2: Product ID Authentication
**Problem**: Fabric authorizing despite not being in user's account

**Cause**: Hardcoded product ID "200002" (Apogee's ID)

**Solution**: Read from `BinaryData::product_id_txt`:
```cpp
juce::String productId = juce::String::fromUTF8(
    BinaryData::product_id_txt,
    BinaryData::product_id_txtSize
).trim();
```

---

### Issue 3: Background Images Not Loading
**Problems**:
1. Incorrect filename (`hadron-background.png` vs `hadron.png`)
2. Wrong `projectRoot` path

**Solution**:
```cpp
auto projectRoot = juce::File("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");
auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds/fabric.png");
```

---

### Issue 4: Settings Button Not Opening Dialog
**Problem**: `StandalonePluginHolder::getInstance()` returning `nullptr`

**Cause**: Custom `Main.cpp` creating wrong standalone app architecture

**Solution**: Remove custom standalone files, regenerate with Projucer

---

### Issue 5: UI Controls Not Affecting DSP
**Problem**: Moving knobs/sliders didn't change audio

**Causes**:
1. ❌ Logic bug in `handleJavaScriptMessage` (if/if/else instead of if/else if)
2. ❌ Using wrong API: `emitEventIfBrowserIsVisible` instead of `evaluateJavascript`

**Solutions**:
1. Refactored to single `if (p != nullptr)` block for all parameters
2. Changed to `webView->evaluateJavascript(script)`

---

### Issue 6: Wet/Dry Always 100% Wet
**Problem**: Wet/dry control not responding, stuck at one value

**Causes**:
1. ❌ Parameter smoothing too slow (20ms lag)
2. ❌ Using `getNextValue()` (per-sample) instead of `skip()` (per-buffer)

**Solutions**:
1. Reduced smoothing to 1ms for mix/wet-dry
2. Use `skip(numSamples)` for instant full-buffer application

---

### Issue 7: No Audio in Standalone
**Problems**:
1. No audio input device configured
2. Audio FX plugins need input to process

**Solution**: 
- Add microphone permissions
- User must configure input device via settings
- For testing: Auto-generate test signal when no input detected

---

### Issue 8: WebView Crashes with NaN Values
**Error**: Crash in `juce::String::String` when converting float to string

**Cause**: Attempting to convert NaN or Infinity to juce::String

**Solution**:
```cpp
#include <cmath>
#include <string>

// Check before converting
if (std::isnan(value) || std::isinf(value)) {
    std::cerr << "invalid" << std::endl;
} else {
    std::cerr << std::to_string(value) << std::endl;
}
```

---

### Issue 9: Delay Line Crashes
**Error**: Crash in `pushSample` / `popSample`

**Cause**: Delay lines not properly prepared before use

**Solution**:
```cpp
// MUST call prepare() before using delay lines
predelayLineL.prepare(spec);
predelayLineR.prepare(spec);

// MUST set maximum delay before using
predelayLineL.setMaximumDelayInSamples(maxDelaySamples);

// Add bounds checking
predelaySamples = juce::jlimit(0.0f, static_cast<float>(maxDelaySamples - 1), predelaySamples);
```

---

## Applying to Other Plugins

### Checklist for Each Plugin

#### 1. Build Configuration
- [ ] Fix header search paths (local VST3 SDK only)
- [ ] Verify `.jucer` has `pluginIsFx` in characteristics
- [ ] Check standalone is enabled in `pluginFormats`

#### 2. Product ID
- [ ] Remove hardcoded product ID
- [ ] Use `BinaryData::product_id_txt`
- [ ] Verify product_id.txt contains correct ID

#### 3. Background Images
- [ ] Fix filename (remove `-background` suffix if needed)
- [ ] Fix projectRoot path (include `MandelbrotSet` for Mandelbrot plugins)
- [ ] Replace path in CSS content before inlining

#### 4. WebView Integration
- [ ] Update to JUCE 8 `Options` API with `withEventListener`
- [ ] Implement `handleJavaScriptMessage` with single parameter handler
- [ ] Use `evaluateJavascript()` for C++ → JS communication
- [ ] Inline all resources (CSS, JS, images) using Base64

#### 5. DSP Implementation (if audio FX)
- [ ] Add DSP modules to header
- [ ] Initialize in `prepareToPlay`
- [ ] Implement processing in `processBlock`
- [ ] Add parameter smoothing
- [ ] Map parameters to DSP engine

#### 6. Audio Routing
- [ ] Fix `isBusesLayoutSupported` to accept mono + stereo input
- [ ] Verify bus configuration in constructor
- [ ] Test with microphone input

#### 7. Settings Button
- [ ] Remove custom standalone files if present
- [ ] Implement `openAudioSettings()` with StandalonePluginHolder
- [ ] Hide settings button in plugin mode (only show in standalone)

#### 8. Parameter Response
- [ ] Use `skip(numSamples)` for critical parameters (mix, wet/dry)
- [ ] Use shorter smoothing time (0.001s instead of 0.02s) for responsive controls
- [ ] Keep longer smoothing (0.02s) for tonal parameters to avoid artifacts

#### 9. JavaScript Component Classes
- [ ] **Mandelbrot plugins**: Use `MandelbrotKnob`, `MandelbrotSlider`
- [ ] **Orbital plugins**: Use `OrbitalsKnob`, `OrbitalsSlider`
- [ ] Verify correct file references in HTML
- [ ] Verify correct inlining paths in C++

#### 10. Permissions & Security
- [ ] Add `NSMicrophoneUsageDescription` to Info.plist
- [ ] Reset permissions: `tccutil reset Microphone <bundle-id>`
- [ ] Verify app requests mic access on first launch

---

## File Structure Reference

### Mandelbrot Plugins
```
MandelbrotSet/
  ├── _Shared/
  │   ├── UI/
  │   │   ├── mandelbrot-components.js       ← Knobs, sliders
  │   │   ├── mandelbrot-animations.js
  │   │   ├── mandelbrot-particles.js
  │   │   └── mandelbrot-design-system.css
  │   ├── Assets/
  │   │   └── backgrounds/[plugin].png       ← Background images
  │   └── Authentication/
  │       ├── HubAuthComponent.h
  │       └── HubAuthComponent.cpp
  ├── Fabric/
  │   ├── Source/
  │   │   ├── PluginProcessor.cpp
  │   │   ├── PluginProcessor.h
  │   │   ├── PluginEditor.cpp
  │   │   └── PluginEditor.h
  │   ├── UI/
  │   │   ├── index.html
  │   │   ├── app.js
  │   │   └── styles.css
  │   └── product_id.txt
```

### Orbital Plugins
```
Orbitals/
  ├── _Shared/
  │   └── UI/
  │       ├── orbitals-components.js         ← Different classes!
  │       ├── orbitals-animations.js
  │       ├── orbitals-particles.js
  │       └── orbitals-design-system.css
```

---

## Code Patterns

### Parameter Declaration Pattern
```cpp
// Header
static const juce::String PARAM_NAME;
juce::AudioProcessorValueTreeState parameters;
juce::SmoothedValue<float> smoothedName;

// Implementation
const juce::String PluginProcessor::PARAM_NAME = "name";

// In createParameterLayout()
params.push_back(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID(PARAM_NAME, 1),
    "Display Name",
    juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
    50.0f
));
```

### HTML Control Pattern
```html
<div class="slider horizontal-slider" id="paramSlider" 
     data-param="param"
     data-help-title="Parameter"
     data-help-description="Description">
    <div class="slider-track"></div>
    <div class="slider-handle"></div>
</div>
```

### JavaScript Control Pattern
```javascript
new MandelbrotSlider(document.getElementById('paramSlider'), {
    min: 0,
    max: 100,
    value: 50,
    orientation: 'horizontal',
    onChange: (value, param) => {
        state.param = value;
        document.getElementById('paramValue').textContent = `${value}`;
        sendToPlugin(param, value);
    }
});
```

---

## Testing Procedures

### 1. Build Test
```bash
xcodebuild -project Plugin.xcodeproj -scheme "Plugin - Standalone Plugin" -configuration Debug build
```

### 2. Component Test
```bash
# VST3
ls ~/Library/Audio/Plug-Ins/VST3/Plugin.vst3

# AU
ls ~/Library/Audio/Plug-Ins/Components/Plugin.component

# Standalone
ls Builds/MacOSX/build/Debug/Plugin.app
```

### 3. Audio Routing Test
```bash
# Run with debug output
./Plugin.app/Contents/MacOS/Plugin

# Look for:
# - "PREPARE TO PLAY" with sample rate
# - "Input: RMS=X Max=Y" (should be > 0 when speaking)
# - "Parameter change request" when moving controls
```

### 4. Parameter Test
- Move each control
- Watch console for "Parameter change request"
- Verify audio changes audibly
- Check wet/dry extremes (0% = dry, 100% = wet)

---

## Common Pitfalls

### ❌ Using getNextValue() Per-Sample
```cpp
// SLOW - recalculates every sample
for (int i = 0; i < numSamples; ++i) {
    float x = smoothed.getNextValue();
}
```

### ✅ Using skip() Per-Buffer
```cpp
// FAST - calculates once per buffer
float x = smoothed.skip(numSamples);
```

---

### ❌ Wrong Bus Layout Support
```cpp
// Only checks output, ignores input
if (layouts.getMainOutputChannelSet() != stereo())
    return false;
return true;
```

### ✅ Correct Bus Layout Support
```cpp
// Checks both input (mono/stereo) and output (stereo)
if (layouts.getMainInputChannelSet() != mono()
    && layouts.getMainInputChannelSet() != stereo())
    return false;

if (layouts.getMainOutputChannelSet() != stereo())
    return false;

return true;
```

---

### ❌ Hardcoded Product IDs
```cpp
juce::String productId = "200002"; // WRONG - conflicts with other plugins
```

### ✅ Reading from BinaryData
```cpp
juce::String productId = juce::String::fromUTF8(
    BinaryData::product_id_txt,
    BinaryData::product_id_txtSize
).trim();
```

---

## Build Scripts

### Build All Formats
```bash
#!/bin/bash
cd Builds/MacOSX
xcodebuild -project Plugin.xcodeproj -scheme "Plugin - All" -configuration Release build
```

### Copy to System Directories
```bash
#!/bin/bash
# VST3
cp -r build/Release/Plugin.vst3 ~/Library/Audio/Plug-Ins/VST3/

# AU
cp -r build/Release/Plugin.component ~/Library/Audio/Plug-Ins/Components/

echo "✅ Plugins installed"
```

---

## Quick Reference: File Locations

### Xcode Project
`Builds/MacOSX/Plugin.xcodeproj/project.pbxproj`

### JUCE Project
`Plugin.jucer`

### Plugin Defines
`JuceLibraryCode/JucePluginDefines.h` (auto-generated)

### Product ID
`product_id.txt` (one line, no trailing newline)

### Shared UI Components
- Mandelbrot: `MandelbrotSet/_Shared/UI/mandelbrot-*.js`
- Orbital: `_Shared/UI/orbitals-*.js`

---

## Summary: Complete Implementation

**Total implementation steps**: ~50 changes across 10+ files

**Key files modified**:
1. `PluginProcessor.h` - DSP components, smoothed values
2. `PluginProcessor.cpp` - DSP implementation, parameter mapping
3. `PluginEditor.h` - WebView, message handling
4. `PluginEditor.cpp` - Resource inlining, bridge setup
5. `app.js` - sendToPlugin, control initialization
6. `index.html` - Control elements with data-param attributes
7. `project.pbxproj` - Header paths, standalone files
8. `.jucer` - Plugin configuration, permissions

**Result**: Professional, fully-functional audio FX plugin with modern UI

---

This guide provides the complete blueprint for implementing any JUCE 8 audio plugin with WebView UI and comprehensive DSP.
