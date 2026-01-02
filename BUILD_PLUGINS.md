# Building Orbitals MIDI FX Plugins

Complete guide to building all 8 Orbitals plugins as VST3 and AU formats.

---

## Prerequisites

- **JUCE Framework 8+** (already present in each plugin's JUCE/ folder)
- **Projucer** (JUCE's project manager)
- **Xcode** (macOS) with Command Line Tools
- **Visual Studio 2022** (Windows)

---

## Quick Build (macOS)

### Step 1: Open Projucer

Download Projucer from [juce.com](https://juce.com) or build it from JUCE source:

```bash
cd /Users/rjmacbookpro/Development/Orbitals/Tidal/JUCE/extras/Projucer/Builds/MacOSX
open Projucer.xcodeproj
# Build and run Projucer
```

### Step 2: Regenerate Each Plugin

For each plugin, open the `.jucer` file in Projucer and click **"Save and Open in IDE"**:

```bash
# List of .jucer files to open:
/Users/rjmacbookpro/Development/Orbitals/Tidal/Tidal.jucer
/Users/rjmacbookpro/Development/Orbitals/Apogee/Apogee.jucer
/Users/rjmacbookpro/Development/Orbitals/Lagrange/Lagrange.jucer
/Users/rjmacbookpro/Development/Orbitals/Eclipse/Eclipse.jucer
/Users/rjmacbookpro/Development/Orbitals/Ion/Ion.jucer
/Users/rjmacbookpro/Development/Orbitals/Perihelion/Perihelion.jucer
/Users/rjmacbookpro/Development/Orbitals/Retrograde/Retrograde.jucer
/Users/rjmacbookpro/Development/Orbitals/Kepler/Kepler.jucer
```

This regenerates the Xcode projects with:
- Plugin wrapper code
- Proper MIDI I/O setup
- VST3 and AU build targets

### Step 3: Build in Xcode

For each plugin:

```bash
cd /Users/rjmacbookpro/Development/Orbitals/Tidal/Builds/MacOSX

# Build VST3
xcodebuild -project Tidal.xcodeproj \
           -scheme "Tidal - VST3" \
           -configuration Release \
           build

# Build AU
xcodebuild -project Tidal.xcodeproj \
           -scheme "Tidal - AU" \
           -configuration Release \
           build
```

### Step 4: Install Plugins

```bash
# VST3
cp -R Builds/MacOSX/build/Release/Tidal.vst3 ~/Library/Audio/Plug-Ins/VST3/

# AU
cp -R Builds/MacOSX/build/Release/Tidal.component ~/Library/Audio/Plug-Ins/Components/
```

---

## Automated Build Script

Create a script to build all plugins:

```bash
#!/bin/bash
# build_all_orbitals.sh

PLUGINS=("Tidal" "Apogee" "Lagrange" "Eclipse" "Ion" "Perihelion" "Retrograde" "Kepler")
BASE_DIR="/Users/rjmacbookpro/Development/Orbitals"

for PLUGIN in "${PLUGINS[@]}"; do
    echo "Building $PLUGIN..."
    cd "$BASE_DIR/$PLUGIN/Builds/MacOSX"
    
    # Build VST3
    xcodebuild -project "$PLUGIN.xcodeproj" \
               -scheme "$PLUGIN - VST3" \
               -configuration Release \
               build
    
    # Build AU
    xcodebuild -project "$PLUGIN.xcodeproj" \
               -scheme "$PLUGIN - AU" \
               -configuration Release \
               build
    
    # Install
    cp -R "build/Release/$PLUGIN.vst3" ~/Library/Audio/Plug-Ins/VST3/
    cp -R "build/Release/$PLUGIN.component" ~/Library/Audio/Plug-Ins/Components/
    
    echo "$PLUGIN complete!"
done

echo "All plugins built and installed!"
```

---

## Using CMake (Alternative)

If you prefer CMake over Projucer:

```bash
cd /Users/rjmacbookpro/Development/Orbitals/Tidal
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release

# Install
cp -R Tidal_artefacts/VST3/Tidal.vst3 ~/Library/Audio/Plug-Ins/VST3/
cp -R Tidal_artefacts/AU/Tidal.component ~/Library/Audio/Plug-Ins/Components/
```

**Note**: CMakeLists.txt files have been updated for plugin format, but Projucer is recommended for initial project generation.

---

## Verification

After building, verify plugins are recognized:

```bash
# Check VST3
ls -la ~/Library/Audio/Plug-Ins/VST3/

# Check AU
ls -la ~/Library/Audio/Plug-Ins/Components/

# Validate AU
auval -v aufx Tidl NNau
```

---

## Testing in DAW

1. **Open your DAW** (Logic Pro, Ableton Live, Reaper, etc.)
2. **Rescan plugins**
3. **Create MIDI track**
4. **Insert Orbitals plugin** as MIDI FX
5. **Play MIDI** and adjust parameters
6. **Verify**:
   - UI loads correctly
   - Parameters respond to UI changes
   - MIDI is processed according to plugin function
   - Automation works

---

## Troubleshooting

### WebView Not Loading

If the UI doesn't appear:
- Check Console.app for error messages
- Verify JUCE 8+ is being used
- Ensure WebBrowserComponent is available

### MIDI Not Processing

- Check plugin is inserted on MIDI track (not audio track)
- Verify MIDI is actually reaching the plugin
- Check bypass is not enabled
- Look for console debug output

### Build Errors

- Ensure all JUCE modules are present
- Check C++17 is enabled
- Verify Xcode Command Line Tools are installed: `xcode-select --install`

---

## Plugin Formats

### VST3
- **Location**: `~/Library/Audio/Plug-Ins/VST3/`
- **Extension**: `.vst3`
- **Supported DAWs**: Ableton Live, Reaper, FL Studio, Cubase, etc.

### AU (Audio Units)
- **Location**: `~/Library/Audio/Plug-Ins/Components/`
- **Extension**: `.component`
- **Supported DAWs**: Logic Pro, GarageBand, MainStage (macOS only)

---

## Development Workflow

1. **Edit UI**: Modify files in `PluginName/UI/`
2. **Edit DSP**: Modify `PluginProcessor.cpp`
3. **Rebuild**: Run build command
4. **Test**: Reload in DAW (may need to restart DAW)

---

## Performance Notes

- **Tidal**: Minimal CPU usage, real-time safe
- **Apogee**: Minimal CPU, uses smoothing arrays
- **Lagrange**: Low CPU, maintains small timing history
- **Eclipse**: Minimal CPU, simple zone checks
- **Ion**: Low CPU, scale lookup is fast
- **Perihelion**: Minimal CPU, simple math
- **Retrograde**: Low CPU, small note buffer
- **Kepler**: Low CPU, simple orbital calculations

All plugins are designed for real-time performance with negligible latency (except Retrograde which buffers notes).

---

**© NNAudio 2026**
