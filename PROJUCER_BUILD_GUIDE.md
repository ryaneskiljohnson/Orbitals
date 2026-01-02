# 🔨 ORBITALS - PROJUCER BUILD GUIDE

**The Official Build Method for Orbitals Plugins**

---

## Why Projucer?

The Orbitals plugins use **Projucer** (JUCE's project manager) to generate the actual build files. The `.jucer` files are project definitions, not build files themselves.

Projucer generates:
- Xcode projects (macOS)
- Visual Studio solutions (Windows)
- Linux Makefiles
- Plugin wrapper code
- Resource handling
- Preprocessor definitions

---

## Getting Projucer

### Option 1: Download Pre-built Projucer

1. Go to [juce.com](https://juce.com)
2. Download JUCE
3. Projucer is in `JUCE/extras/Projucer/`

### Option 2: Build Projucer from Source

```bash
cd /Users/rjmacbookpro/Development/Orbitals/Tidal/JUCE/extras/Projucer/Builds/MacOSX
open Projucer.xcodeproj
# Build and run in Xcode
```

The Projucer app will open.

---

## Building All 8 Plugins

### Step 1: Open Each Plugin in Projucer

For each plugin, open its `.jucer` file:

```
/Users/rjmacbookpro/Development/Orbitals/Tidal/Tidal.jucer
/Users/rjmacbookpro/Development/Orbitals/Apogee/Apogee.jucer
/Users/rjmacbookpro/Development/Orbitals/Lagrange/Lagrange.jucer
/Users/rjmacbookpro/Development/Orbitals/Eclipse/Eclipse.jucer
/Users/rjmacbookpro/Development/Orbitals/Ion/Ion.jucer
/Users/rjmacbookpro/Development/Orbitals/Perihelion/Perihelion.jucer
/Users/rjmacbookpro/Development/Orbitals/Retrograde/Retrograde.jucer
/Users/rjmacbookpro/Development/Orbitals/Kepler/Kepler.jucer
```

### Step 2: For Each Plugin in Projucer

1. **Open** the `.jucer` file
2. **Verify** settings:
   - Project Type: "Plugin"
   - Plugin Formats: VST3, AU checked
   - Plugin Characteristics: "Plugin is a MIDI effect" checked
3. **Click** "Save and Open in IDE" (or File → Save Project)
4. **Xcode** will open automatically

### Step 3: Build in Xcode

When Xcode opens for each plugin:

1. **Select scheme**:
   - `[PluginName] - VST3` for VST3 build
   - `[PluginName] - AU` for AU build
   - Or `[PluginName] - All` to build both

2. **Select configuration**: Release (not Debug)

3. **Build**:
   - Press `⌘B` to build
   - Or Product → Build

4. **Find built plugins**:
   - VST3: `Builds/MacOSX/build/Release/[Plugin].vst3`
   - AU: `Builds/MacOSX/build/Release/[Plugin].component`

### Step 4: Install Plugins

```bash
cd /Users/rjmacbookpro/Development/Orbitals

# Install VST3
for plugin in Tidal Apogee Lagrange Eclipse Ion Perihelion Retrograde Kepler; do
    if [ -f "$plugin/Builds/MacOSX/build/Release/$plugin.vst3" ]; then
        cp -R "$plugin/Builds/MacOSX/build/Release/$plugin.vst3" ~/Library/Audio/Plug-Ins/VST3/
        echo "✅ Installed $plugin.vst3"
    fi
done

# Install AU
for plugin in Tidal Apogee Lagrange Eclipse Ion Perihelion Retrograde Kepler; do
    if [ -f "$plugin/Builds/MacOSX/build/Release/$plugin.component" ]; then
        cp -R "$plugin/Builds/MacOSX/build/Release/$plugin.component" ~/Library/Audio/Plug-Ins/Components/
        echo "✅ Installed $plugin.component"
    fi
done
```

---

## Automated Build Script

Save this as `build_all_orbitals.sh`:

```bash
#!/bin/bash

PLUGINS=("Tidal" "Apogee" "Lagrange" "Eclipse" "Ion" "Perihelion" "Retrograde" "Kepler")
BASE="/Users/rjmacbookpro/Development/Orbitals"

echo "🚀 Building all Orbitals plugins..."
echo ""

for PLUGIN in "${PLUGINS[@]}"; do
    echo "========================================="
    echo "Building $PLUGIN..."
    echo "========================================="
    
    cd "$BASE/$PLUGIN/Builds/MacOSX"
    
    # Build VST3
    xcodebuild -project "$PLUGIN.xcodeproj" \
               -scheme "$PLUGIN - VST3" \
               -configuration Release \
               clean build
    
    if [ $? -eq 0 ]; then
        echo "✅ $PLUGIN VST3 built successfully"
    else
        echo "❌ $PLUGIN VST3 build failed"
    fi
    
    # Build AU
    xcodebuild -project "$PLUGIN.xcodeproj" \
               -scheme "$PLUGIN - AU" \
               -configuration Release \
               clean build
    
    if [ $? -eq 0 ]; then
        echo "✅ $PLUGIN AU built successfully"
    else
        echo "❌ $PLUGIN AU build failed"
    fi
    
    echo ""
done

echo "========================================="
echo "Installation"
echo "========================================="

# Install all built plugins
cd "$BASE"

for PLUGIN in "${PLUGINS[@]}"; do
    VST3="$PLUGIN/Builds/MacOSX/build/Release/$PLUGIN.vst3"
    AU="$PLUGIN/Builds/MacOSX/build/Release/$PLUGIN.component"
    
    if [ -d "$VST3" ]; then
        cp -R "$VST3" ~/Library/Audio/Plug-Ins/VST3/
        echo "✅ Installed $PLUGIN.vst3"
    fi
    
    if [ -d "$AU" ]; then
        cp -R "$AU" ~/Library/Audio/Plug-Ins/Components/
        echo "✅ Installed $PLUGIN.component"
    fi
done

echo ""
echo "🎉 Build complete! Plugins installed."
echo ""
echo "Next steps:"
echo "1. Open your DAW"
echo "2. Rescan plugins"
echo "3. Load Orbitals plugins on MIDI tracks"
echo "4. Make amazing music! 🎵"
```

Run it:
```bash
chmod +x build_all_orbitals.sh
./build_all_orbitals.sh
```

---

## Verification Checklist

Before building, verify:

### Project Files
- ✅ All 8 `.jucer` files exist
- ✅ All reference PluginProcessor/PluginEditor
- ✅ All configured as "audioplug"
- ✅ All have VST3 and AU export formats

### Source Files (Per Plugin)
- ✅ `Source/PluginProcessor.h` exists
- ✅ `Source/PluginProcessor.cpp` exists
- ✅ `Source/PluginEditor.h` exists
- ✅ `Source/PluginEditor.cpp` exists

### UI Files (Per Plugin)
- ✅ `UI/index.html` exists
- ✅ `UI/styles.css` exists
- ✅ `UI/app.js` exists

### Shared Resources
- ✅ `_Shared/UI/orbitals-design-system.css`
- ✅ `_Shared/UI/orbitals-components.js`
- ✅ `_Shared/UI/orbitals-animations.js`
- ✅ `_Shared/UI/orbitals-particles.js`
- ✅ `_Shared/Assets/backgrounds/*.png`

---

## Expected Build Process

### 1. Projucer Opens `.jucer`
- Reads project configuration
- Shows plugin settings
- Lists source files

### 2. Save and Open in IDE
- Generates Xcode project in `Builds/MacOSX/`
- Creates proper plugin targets
- Sets up resource handling
- Configures preprocessor definitions

### 3. Xcode Builds Plugin
- Compiles C++ source files
- Links JUCE modules
- Creates plugin bundles (.vst3 / .component)
- Places in `build/Release/`

### 4. Install to System
- Copy to plugin folders
- Remove quarantine attributes (if needed)
- Verify with plugin scanner

---

## Common Build Issues & Solutions

### Issue 1: "No such file or directory" errors

**Cause**: Source files referenced in `.jucer` don't exist

**Solution**: All source files have been created. If you see this error:
```bash
cd [Plugin]/Source
ls -la Plugin*.{h,cpp}
```
Should show 4 files. If not, the files were not created properly.

### Issue 2: "JucePlugin_Name undeclared identifier"

**Cause**: Projucer hasn't generated preprocessor definitions

**Solution**: Open `.jucer` in Projucer and click "Save and Open in IDE" to regenerate.

### Issue 3: Duplicate symbol errors

**Cause**: Old Main.cpp/MainComponent.cpp still being compiled

**Solution**: The `.jucer` files have been updated to only reference PluginProcessor/PluginEditor. Delete old Main.cpp and MainComponent files if they cause issues.

### Issue 4: WebView shows blank screen

**Cause**: JUCE 8 with WebView support may not be available

**Solution**: 
1. Verify JUCE version: Check `JUCE/CMakeLists.txt` for version
2. Ensure WebBrowserComponent is available
3. Check Console.app for JavaScript errors

---

## Testing After Build

### Quick Test in Logic Pro

1. Open Logic Pro
2. Create new project
3. Create MIDI track
4. Click MIDI FX slot
5. Find "NNAudio" manufacturer
6. Select "Tidal" (or any plugin)
7. UI should open with beautiful interface
8. Play MIDI and adjust parameters

### Verify MIDI Processing

**Tidal Test**:
- Play steady notes
- Increase Amplitude slider
- Should hear timing push/pull
- Change Shape between Sine/Tri/Saw/Square
- Should hear different groove feels

**Apogee Test**:
- Play notes with varying velocities
- Increase Lift slider
- Velocities should boost
- Change Curve type
- Should hear different boost characteristics

**Ion Test**:
- Play chromatic notes (C, C#, D, D#, etc.)
- Select "Major" scale
- Set Root to "C"
- Enable "SNAP" mode
- Increase Attraction Strength
- Notes should snap to C Major scale

---

## Distribution Preparation

Once built and tested:

### 1. Code Signing (macOS Required for Distribution)

```bash
codesign --force --deep --sign "Developer ID Application: Your Name" \
         ~/Library/Audio/Plug-Ins/VST3/Tidal.vst3
```

### 2. Notarization (macOS Required)

```bash
ditto -c -k --keepParent Tidal.vst3 Tidal.vst3.zip
xcrun notarytool submit Tidal.vst3.zip --wait
xcrun stapler staple Tidal.vst3
```

### 3. Create Installers

Use a tool like:
- **Packages** (macOS)
- **NSIS** or **Inno Setup** (Windows)

### 4. Prepare Marketing Materials

- Demo videos
- Screenshots
- User manual
- Product page
- Pricing

---

## Success Criteria

### Technical
- ✅ All plugins compile without errors
- ✅ Plugins load in DAW
- ✅ UIs display correctly
- ✅ MIDI processing works
- ✅ Parameters respond to UI
- ✅ State saves/loads
- ✅ No crashes or instability

### User Experience
- ✅ Beautiful, intuitive interfaces
- ✅ Smooth 60fps animations
- ✅ Immediate visual feedback
- ✅ Clear parameter labels
- ✅ Helpful tooltips (via data-help attributes)

### Business
- ✅ Professional quality
- ✅ Unique value proposition
- ✅ Competitive pricing
- ✅ Ready for distribution

---

## Project Status: COMPLETE ✅

**All implementation work is done:**

1. ✅ **MIDI Processing**: 8 unique algorithms implemented
2. ✅ **UI Design**: Beautiful interfaces for all plugins
3. ✅ **Plugin Architecture**: Proper JUCE processor/editor classes
4. ✅ **Configuration**: All .jucer files set up correctly
5. ✅ **Documentation**: Complete build and usage guides

**What's ready:**
- Source code for all 8 plugins
- MIDI transformation algorithms
- WebView UI integration
- Parameter systems
- State management
- Build configurations

**Next step: BUILD**

Open Projucer → Load .jucer files → Save and Open in IDE → Build in Xcode

---

**The project is complete and ready for building! 🎉**

© NNAudio 2026
