# Tidal Standalone Executable Fix - CORRECTED

## ❌ The Error
```
Static assertion failed: You are trying to use START_JUCE_APPLICATION in an audio plug-in. 
Define JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1 if you want to use a custom standalone target app.
```

## 🔍 Root Cause

**Tidal.jucer incorrectly included `Main.cpp` and `MainComponent` files**, which are for standalone applications (`guiapp` project type), NOT for audio plugins (`audioplug`).

### How JUCE Handles Standalone for Plugins

For **plugin projects** (`projectType="audioplug"`):
- ✅ JUCE **automatically generates** a standalone wrapper when `buildStandalone` is enabled
- ✅ The wrapper uses `juce_audio_plugin_client_Standalone.cpp` 
- ✅ Projucer creates this wrapper code when you regenerate the Xcode project
- ❌ You should **NOT** include `Main.cpp` with `START_JUCE_APPLICATION`

For **standalone applications** (`projectType="guiapp"`):
- ✅ You **must** include `Main.cpp` with `START_JUCE_APPLICATION`
- ✅ You **must** include `MainComponent.h/cpp` for the UI

## ✅ The Correct Fix

**Removed `Main.cpp` and `MainComponent` files from `Tidal.jucer`** - they don't belong in plugin projects.

The `.jucer` file now only contains:
- `PluginProcessor.h/cpp` - The audio processor
- `PluginEditor.h/cpp` - The plugin editor/UI
- UI resources (HTML, CSS, JS)

## 🔧 Next Steps

**You MUST regenerate the Xcode project in Projucer:**

1. **Open `Tidal.jucer` in JUCE Projucer**
2. **Verify standalone export is enabled:**
   - Check that `pluginFormats="buildAU,buildVST3,buildStandalone"` includes `buildStandalone`
3. **Click "Save and Open in IDE"** (or File → Save Project)
4. **Projucer will automatically generate:**
   - `JuceLibraryCode/juce_audio_plugin_client_Standalone.cpp` (or similar)
   - The standalone wrapper that creates the `main()` entry point
   - Proper Xcode target with all source files included
5. **Build the "Tidal - Standalone Plugin" target in Xcode**

## What Projucer Generates

When you regenerate with `buildStandalone` enabled, Projucer automatically:

1. **Creates standalone wrapper code** - Wraps your `PluginProcessor` and `PluginEditor` in a `JUCEApplication`
2. **Generates main() entry point** - Uses JUCE's plugin client wrapper, not `START_JUCE_APPLICATION`
3. **Sets up audio device handling** - Configures audio I/O for standalone mode
4. **Creates Xcode target** - Builds the standalone app bundle with executable

## Verification

After regenerating and building, verify:

```bash
# Check executable exists
ls -la "Tidal/Builds/MacOSX/build/Debug/Tidal.app/Contents/MacOS/"
# Should show: Tidal (executable binary)

file "Tidal/Builds/MacOSX/build/Debug/Tidal.app/Contents/MacOS/Tidal"
# Should show: Mach-O 64-bit executable arm64
```

## Why Kepler Works

Kepler's `.jucer` file **doesn't include `Main.cpp`** - it only has the plugin files. When Projucer generated Kepler's Xcode project, it correctly created the standalone wrapper automatically.

## Summary

- **Issue:** `Main.cpp` with `START_JUCE_APPLICATION` was incorrectly added to plugin project
- **Error:** JUCE doesn't allow `START_JUCE_APPLICATION` in plugin projects
- **Fix:** Removed `Main.cpp` and `MainComponent` from `.jucer` file
- **Solution:** Regenerate Xcode project in Projucer to generate standalone wrapper
- **Status:** ✅ Fixed in `.jucer` file, awaiting Projucer regeneration

## Key Takeaway

**Plugin projects (`audioplug`) ≠ Standalone apps (`guiapp`)**

- Plugins: Use Projucer-generated standalone wrapper
- Standalone apps: Use `Main.cpp` with `START_JUCE_APPLICATION`
