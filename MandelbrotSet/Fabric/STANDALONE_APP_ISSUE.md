# Fabric Standalone App Issue

## 🚨 ROOT CAUSE

The standalone app (`Main.cpp` + `MainComponent`) is using a **SYNTH TEMPLATE** that doesn't match Fabric's functionality!

### Current MainComponent (WRONG for Fabric):
- Handles: `volume`, `frequency`, `waveform`, `playNote`, `stopNote`
- Purpose: Synthesizer that generates audio
- Uses: `AudioDeviceManager` with audio output callback
- Problem: **Fabric is a REVERB EFFECT, not a synth!**

### What Fabric Needs:
- Parameters: `size`, `diffusion`, `damping`, `predelay`, `mix`, `bypass`
- Purpose: Process audio input with reverb effect
- Uses: `AudioProcessor` with `processBlock()`
- Solution: **Use the plugin architecture, not custom synth**

## 🎯 SOLUTION OPTIONS

### Option 1: Remove Custom Standalone (RECOMMENDED)
**Delete `Main.cpp` and `MainComponent.cpp`** from the project and let JUCE auto-generate the standalone wrapper.

**Pros:**
- Standard JUCE plugin architecture
- Uses `PluginEditor` (which has all reverb controls)
- Automatically handles audio I/O
- Settings button will work via `StandalonePluginHolder`

**Cons:**
- Loses custom window/app setup (if any)

### Option 2: Fix MainComponent to be a Reverb Host
Rewrite `MainComponent` to:
1. Load audio files or use audio input
2. Create an instance of `FabricAudioProcessor`
3. Process audio through the processor
4. Handle reverb parameters instead of synth parameters

**Pros:**
- Keeps custom standalone app
- More control over app behavior

**Cons:**
- Much more work
- Duplicates plugin code
- Harder to maintain

## 🔧 RECOMMENDED FIX

### Step 1: Remove Main.cpp and MainComponent from JUCER

Open `Fabric.jucer` in Projucer and:
1. Remove `Main.cpp` from the file list
2. Remove `MainComponent.h` from the file list
3. Remove `MainComponent.cpp` from the file list
4. Save and regenerate the Xcode project

### Step 2: Let JUCE Generate Standalone

Projucer will automatically generate:
- `juce_audio_plugin_client_Standalone.cpp`
- Standalone wrapper that uses `PluginEditor`
- Proper audio I/O handling
- `StandalonePluginHolder` for settings dialog

### Step 3: Rebuild

```bash
cd MandelbrotSet/Fabric/Builds/MacOSX
xcodebuild -project Fabric.xcodeproj -scheme "Fabric - Standalone Plugin" -configuration Debug clean build
```

## 🎯 ALTERNATIVE: Quick Fix for MainComponent

If you want to keep the custom standalone app, update `MainComponent` to handle Fabric parameters:

```cpp
// In MainComponent::handleJavaScriptMessage()
if (type == "openSettings")
{
    audioDeviceManager.showAudioDeviceSettingsDialog();
}
else if (type == "parameterChange")
{
    auto param = obj->getProperty("parameter").toString();
    auto value = (float)obj->getProperty("value");
    
    // Forward to the audio processor instance
    // (Need to create FabricAudioProcessor instance in MainComponent)
    if (param == "size") { /* update processor */ }
    else if (param == "diffusion") { /* update processor */ }
    // etc...
}
```

But this requires:
1. Creating `FabricAudioProcessor` instance in `MainComponent`
2. Processing audio through it
3. Handling all parameter mappings
4. Much more complex code

## 📝 RECOMMENDATION

**Use Option 1** - Remove the custom standalone and use JUCE's auto-generated one. This is:
- ✅ Standard practice
- ✅ Less code to maintain
- ✅ Proper plugin architecture
- ✅ Settings button will work automatically
- ✅ All parameters already connected in `PluginEditor`

The custom `MainComponent` approach only makes sense for synths (Orbital plugins), not for effects (Fabric).
