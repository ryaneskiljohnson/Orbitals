# 🚀 ORBITALS - BUILD & RUN INSTRUCTIONS

## Quick Start

```bash
cd /Users/rjmacbookpro/Development/Orbitals

# Build a single plugin (example: Tidal)
cd Tidal
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Install to system
cp -R Tidal_artefacts/VST3/Tidal.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

---

## Prerequisites

### Required Software
- ✅ **JUCE Framework 8.0+** (already in each plugin's JUCE/ folder)
- ✅ **CMake 3.22+** (install via Homebrew: `brew install cmake`)
- ✅ **Xcode** (macOS) with Command Line Tools
- ✅ **C++17 Compiler**

### Verify Prerequisites

```bash
cmake --version  # Should be 3.22 or higher
xcodebuild -version  # Should show Xcode version
```

---

## Building All Plugins

### Option 1: Build Script (Recommended)

```bash
cd /Users/rjmacbookpro/Development/Orbitals

# Create build script
cat > build_all.sh << 'EOF'
#!/bin/bash
PLUGINS=("Tidal" "Apogee" "Lagrange" "Eclipse" "Ion" "Perihelion" "Retrograde" "Kepler")

for plugin in "${PLUGINS[@]}"; do
    echo "========================================="
    echo "Building $plugin..."
    echo "========================================="
    
    cd "$plugin"
    rm -rf build
    mkdir build && cd build
    
    cmake .. || { echo "❌ CMake failed for $plugin"; cd ../..; continue; }
    cmake --build . --config Release || { echo "❌ Build failed for $plugin"; cd ../..; continue; }
    
    echo "✅ $plugin built successfully"
    cd ../..
done

echo ""
echo "========================================="
echo "Build Summary"
echo "========================================="
ls -d */build/*_artefacts/VST3/*.vst3 2>/dev/null | wc -l | xargs echo "VST3 plugins built:"
ls -d */build/*_artefacts/AU/*.component 2>/dev/null | wc -l | xargs echo "AU plugins built:"
EOF

chmod +x build_all.sh
./build_all.sh
```

### Option 2: Manual Build (One Plugin)

```bash
cd /Users/rjmacbookpro/Development/Orbitals/Tidal

# Clean previous build
rm -rf build

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build . --config Release

# Check output
ls -la Tidal_artefacts/VST3/
ls -la Tidal_artefacts/AU/
```

---

## Installing Plugins

### Install All VST3 Plugins

```bash
cd /Users/rjmacbookpro/Development/Orbitals

for plugin in Tidal Apogee Lagrange Eclipse Ion Perihelion Retrograde Kepler; do
    if [ -d "$plugin/build/${plugin}_artefacts/VST3" ]; then
        cp -R "$plugin/build/${plugin}_artefacts/VST3/${plugin}.vst3" ~/Library/Audio/Plug-Ins/VST3/
        echo "✅ Installed $plugin.vst3"
    fi
done
```

### Install All AU Plugins

```bash
cd /Users/rjmacbookpro/Development/Orbitals

for plugin in Tidal Apogee Lagrange Eclipse Ion Perihelion Retrograde Kepler; do
    if [ -d "$plugin/build/${plugin}_artefacts/AU" ]; then
        cp -R "$plugin/build/${plugin}_artefacts/AU/${plugin}.component" ~/Library/Audio/Plug-Ins/Components/
        echo "✅ Installed $plugin.component"
    fi
done
```

### Verify Installation

```bash
# Check VST3
ls -la ~/Library/Audio/Plug-Ins/VST3/ | grep -i orbitals

# Check AU
ls -la ~/Library/Audio/Plug-Ins/Components/ | grep -i orbitals
```

---

## Using in DAW

### Logic Pro
1. Open Logic Pro
2. **Preferences** → **Plug-in Manager** → **Reset & Rescan**
3. Create **MIDI track** or **Software Instrument track**
4. Click **MIDI FX** slot
5. Find **NNAudio** → Select plugin (e.g., **Tidal**)
6. UI opens automatically

### Ableton Live
1. Open Ableton Live
2. **Preferences** → **Plug-ins** → **Rescan**
3. Create **MIDI track**
4. Drag plugin from **Browser** → **Plug-ins** → **NNAudio**
5. Drop on **MIDI Effects** section

### Reaper
1. Open Reaper
2. **Options** → **Preferences** → **Plug-ins** → **VST** → **Re-scan**
3. Create **MIDI track**
4. Click **FX** button
5. Search for plugin name
6. Add as **MIDI FX**

---

## Troubleshooting

### CMake Can't Find JUCE

**Problem**: `Could not find JUCE`

**Solution**: Ensure JUCE folder exists in each plugin directory:
```bash
cd Tidal
ls -la JUCE  # Should show JUCE framework files
```

If missing, JUCE should be a symlink or copy in each plugin folder.

### Build Fails with "JucePlugin_Name not defined"

**Problem**: Plugin preprocessor definitions missing

**Solution**: Open `.jucer` file in Projucer, click "Save and Open in IDE" to regenerate project files.

### Plugin Doesn't Appear in DAW

**Problem**: Plugin not recognized

**Solutions**:
1. Verify installation path is correct
2. Check plugin is not quarantined: `xattr -d com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/Tidal.vst3`
3. Rescan plugins in DAW
4. Check Console.app for errors

### UI Doesn't Load

**Problem**: WebView shows blank screen

**Solutions**:
1. Check Console.app for JavaScript errors
2. Verify HTML/CSS/JS files are being inlined correctly
3. Check background image paths
4. Ensure JUCE 8+ with WebView support

### MIDI Not Processing

**Problem**: Notes pass through unchanged

**Solutions**:
1. Verify bypass is off
2. Check parameter values are being received
3. Add DBG() statements in processBlock()
4. Verify DAW is sending MIDI to plugin

---

## Development Workflow

### Modify MIDI Processing

1. Edit `Source/PluginProcessor.cpp`
2. Rebuild plugin
3. Restart DAW or reload plugin
4. Test changes

### Modify UI

1. Edit `UI/index.html`, `UI/styles.css`, or `UI/app.js`
2. Rebuild plugin (to inline changes)
3. Reload plugin in DAW
4. UI updates automatically

### Add New Parameter

1. Add to `PluginProcessor.h` (static const String)
2. Add to `createParameterLayout()` in `PluginProcessor.cpp`
3. Use in `processBlock()`
4. Handle in `PluginEditor.cpp` → `handleJavaScriptMessage()`
5. Add UI control in `UI/index.html`
6. Wire up in `UI/app.js`

---

## Performance Optimization

### CPU Usage
- All plugins optimized for real-time performance
- No dynamic memory allocation in audio thread
- Efficient MIDI buffer operations

### Memory Usage
- UI assets inlined at compile time
- Minimal runtime allocations
- Shared design system reduces duplication

### Latency
- Most plugins: 0 samples latency
- Retrograde: Buffer size latency (configurable)
- All suitable for live performance

---

## Support

For issues or questions:
- Check Console.app for errors
- Review JUCE documentation: https://docs.juce.com
- Verify JUCE version compatibility

---

**Happy Music Making! 🎵**

© 2026 NNAudio
