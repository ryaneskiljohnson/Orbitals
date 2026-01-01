# ORBITALS - Build and Run Guide

**How to build and run the Orbitals MIDI FX plugins**

---

## 🚀 Quick Start

### Prerequisites

1. **JUCE Framework** (8.0+)
   - Should be in `/Users/rjmacbookpro/Development/Synth/JUCE`
   - Or update CMakeLists.txt with your JUCE path

2. **CMake** (3.22 or higher)
   ```bash
   cmake --version
   ```

3. **C++ Compiler**
   - macOS: Xcode Command Line Tools
   - Windows: Visual Studio 2019+
   - Linux: GCC 7+ or Clang 8+

4. **Build Tools**
   - macOS: Xcode
   - Windows: Visual Studio
   - Linux: Make

---

## 📦 Building Perihelion (Example)

### Step 1: Navigate to Plugin Directory

```bash
cd /Users/rjmacbookpro/Development/Orbitals/01_Perihelion
```

### Step 2: Create Build Directory

```bash
mkdir build
cd build
```

### Step 3: Configure with CMake

```bash
cmake ..
```

This will:
- Find JUCE framework
- Configure the plugin project
- Generate build files for your platform

**If JUCE not found**, update the path in `CMakeLists.txt`:
```cmake
set(JUCE_DIR "/path/to/your/JUCE")
```

### Step 4: Build the Plugin

**macOS (Xcode)**:
```bash
cmake --build . --config Release
```

**macOS (Make)**:
```bash
make -j8
```

**Windows (Visual Studio)**:
```bash
cmake --build . --config Release
```

**Linux**:
```bash
make -j8
```

### Step 5: Find the Built Plugin

**macOS**:
- **VST3**: `build/Perihelion_artefacts/VST3/Perihelion.vst3`
- **AU**: `build/Perihelion_artefacts/AU/Perihelion.component`

**Windows**:
- **VST3**: `build/Perihelion_artefacts/Release/VST3/Perihelion.vst3`

**Linux**:
- **VST3**: `build/Perihelion_artefacts/VST3/Perihelion.so`

---

## 🎹 Running the Plugin

### Option 1: Load in DAW

1. **Copy plugin to system location**:

   **macOS VST3**:
   ```bash
   cp -R build/Perihelion_artefacts/VST3/Perihelion.vst3 ~/Library/Audio/Plug-Ins/VST3/
   ```

   **macOS AU**:
   ```bash
   cp -R build/Perihelion_artefacts/AU/Perihelion.component ~/Library/Audio/Plug-Ins/Components/
   ```

   **Windows VST3**:
   ```
   Copy to: C:\Program Files\Common Files\VST3\Perihelion.vst3
   ```

   **Linux VST3**:
   ```bash
   cp -R build/Perihelion_artefacts/VST3/Perihelion.so ~/.vst3/
   ```

2. **Rescan plugins in your DAW**:
   - Logic Pro: Preferences → Plug-ins → Rescan
   - Ableton Live: Options → Preferences → Plug-ins → Rescan
   - Reaper: Options → Preferences → Plug-ins → VST → Re-scan
   - Pro Tools: (AU only) Rescan in plugin manager

3. **Load plugin**:
   - Create MIDI track
   - Insert Perihelion as MIDI FX
   - Send MIDI through it
   - UI should open showing the beautiful WebView interface!

### Option 2: Use JUCE Plugin Host (for testing)

1. **Build JUCE Plugin Host**:
   ```bash
   cd /Users/rjmacbookpro/Development/Synth/JUCE/extras/PluginHost
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

2. **Run Plugin Host**:
   ```bash
   ./PluginHost.app  # macOS
   # or
   ./PluginHost.exe  # Windows
   ```

3. **Load Perihelion**:
   - File → Load Plugin
   - Navigate to built `.vst3` or `.component` file
   - Plugin opens with WebView UI!

---

## 🔧 Troubleshooting

### "JUCE not found"

**Solution**: Update CMakeLists.txt with correct JUCE path:
```cmake
set(JUCE_DIR "/absolute/path/to/JUCE")
```

### "UI files not found"

**Solution**: Ensure UI files are in correct location:
```
01_Perihelion/
├── Source/
└── UI/
    ├── index.html
    ├── styles.css
    └── app.js
```

### "WebView not working"

**Solution**: 
1. Ensure JUCE 8.0+ (WebView requires JUCE 8)
2. Check console for error messages
3. Verify HTML/CSS/JS files are being loaded

### "Plugin doesn't appear in DAW"

**Solution**:
1. Check plugin is in correct system folder
2. Rescan plugins in DAW
3. Verify plugin format (VST3 vs AU)
4. Check DAW supports MIDI FX plugins
5. macOS: May need to allow plugin in System Preferences → Security

### "White screen or no UI"

**Solution**:
1. Check console output for file path errors
2. Verify UI files exist at expected paths
3. Check WebView is enabled in JUCE
4. Try opening HTML file directly in browser to test

---

## 🏗️ Building All 8 Plugins

### Quick Build Script

Create `build-all.sh`:

```bash
#!/bin/bash

PLUGINS=(
    "01_Perihelion"
    "02_Lagrange"
    "03_Apogee"
    "04_Retrograde"
    "05_Tidal"
    "06_Ion"
    "07_Eclipse"
    "08_Kepler"
)

ORBITALS_DIR="/Users/rjmacbookpro/Development/Orbitals"

for plugin in "${PLUGINS[@]}"; do
    echo "Building $plugin..."
    cd "$ORBITALS_DIR/$plugin"
    mkdir -p build
    cd build
    cmake ..
    cmake --build . --config Release
    echo "$plugin build complete!"
    echo ""
done

echo "All plugins built!"
```

Make executable:
```bash
chmod +x build-all.sh
./build-all.sh
```

---

## 📝 Development Workflow

### 1. Edit UI Files

Edit HTML/CSS/JS in `UI/` folder:
```bash
code 01_Perihelion/UI/index.html
code 01_Perihelion/UI/styles.css
code 01_Perihelion/UI/app.js
```

### 2. Rebuild Plugin

```bash
cd 01_Perihelion/build
cmake --build . --config Release
```

### 3. Reload in DAW

- Close plugin window
- Reopen plugin
- New UI changes appear!

**Note**: For faster iteration, you can edit HTML directly in temp file, but rebuilding is recommended for production.

---

## 🎯 Testing Checklist

Before releasing, test:

- [ ] Plugin loads in major DAWs (Logic, Ableton, Reaper, etc.)
- [ ] UI displays correctly (WebView loads)
- [ ] All controls respond (knobs, sliders, etc.)
- [ ] MIDI passes through when bypassed
- [ ] MIDI processing works (velocity sculpting)
- [ ] Parameters save/load with project
- [ ] No crashes or memory leaks
- [ ] Performance is good (no audio dropouts)

---

## 🚀 Next Steps

Once Perihelion is working:

1. **Copy template to other plugins**:
   - Copy `PluginProcessor.h/cpp` and `PluginEditor.h/cpp`
   - Update plugin names and parameter IDs
   - Update UI file paths

2. **Implement DSP algorithms**:
   - Each plugin needs its specific MIDI processing
   - See README files for algorithm descriptions

3. **Add preset management**:
   - Create preset files
   - Add preset browser functionality

4. **Optimize and polish**:
   - Performance profiling
   - Memory optimization
   - UI responsiveness

---

## 📚 Additional Resources

- **JUCE Documentation**: https://juce.com/learn/documentation
- **JUCE Forum**: https://forum.juce.com
- **Plugin Development**: See JUCE examples in `JUCE/examples/`

---

## ✅ Success Indicators

You'll know it's working when:

1. ✅ Plugin builds without errors
2. ✅ Plugin appears in DAW plugin list
3. ✅ Beautiful WebView UI opens
4. ✅ Controls respond to mouse/touch
5. ✅ MIDI passes through and is processed
6. ✅ Parameters save with project

---

**Happy coding! 🎹✨**

*Orbitals MIDI FX Plugin Line - Build Guide*
*© NNAudio - Premium Music Production Tools*
