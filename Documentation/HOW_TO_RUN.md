# 🚀 HOW TO RUN ORBITALS PLUGINS

## Quick Answer

```bash
# 1. Build Perihelion
cd /Users/rjmacbookpro/Development/Orbitals/01_Perihelion
mkdir build && cd build
cmake ..
cmake --build . --config Release

# 2. Install to system
cp -R Perihelion_artefacts/VST3/Perihelion.vst3 ~/Library/Audio/Plug-Ins/VST3/

# 3. Open DAW, rescan plugins, load Perihelion on MIDI track!
```

---

## 📋 Detailed Steps

### Prerequisites Check

✅ **JUCE Framework** - Should be at `/Users/rjmacbookpro/Development/Synth/JUCE`
✅ **CMake 3.22+** - Check with `cmake --version`
✅ **C++ Compiler** - Xcode on macOS

### Build Process

1. **Navigate to plugin**:
   ```bash
   cd /Users/rjmacbookpro/Development/Orbitals/01_Perihelion
   ```

2. **Create build directory**:
   ```bash
   mkdir build
   cd build
   ```

3. **Configure**:
   ```bash
   cmake ..
   ```
   This finds JUCE and sets up the build.

4. **Build**:
   ```bash
   cmake --build . --config Release
   ```
   Or use the script:
   ```bash
   cd /Users/rjmacbookpro/Development/Orbitals
   ./build-perihelion.sh
   ```

5. **Find built plugin**:
   - VST3: `build/Perihelion_artefacts/VST3/Perihelion.vst3`
   - AU: `build/Perihelion_artefacts/AU/Perihelion.component`

### Installation

**macOS VST3**:
```bash
cp -R build/Perihelion_artefacts/VST3/Perihelion.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

**macOS AU**:
```bash
cp -R build/Perihelion_artefacts/AU/Perihelion.component ~/Library/Audio/Plug-Ins/Components/
```

### Running in DAW

1. **Open your DAW** (Logic Pro, Ableton Live, Reaper, etc.)
2. **Rescan plugins**:
   - Logic: Preferences → Plug-ins → Rescan
   - Ableton: Options → Preferences → Plug-ins → Rescan
   - Reaper: Options → Preferences → Plug-ins → VST → Re-scan
3. **Create MIDI track**
4. **Insert Perihelion** as MIDI FX
5. **Beautiful UI opens!** 🎨

---

## 🎨 What You'll See

When Perihelion loads:

- **Solar-themed interface** with golden sun visualization
- **Interactive controls**:
  - Gravity knob (center, large)
  - Solar Point slider (vertical)
  - Orbit arc (top)
  - Bias slider (bottom)
- **Real-time particle animation** showing velocity being pulled
- **Smooth 60fps animations**
- **Professional glassmorphism design**

---

## 🔧 Troubleshooting

### Build Issues

**"JUCE not found"**:
```bash
# Check JUCE location
ls /Users/rjmacbookpro/Development/Synth/JUCE

# If missing, update CMakeLists.txt:
# set(JUCE_DIR "/your/path/to/JUCE")
```

**"CMake error"**:
- Ensure CMake 3.22+
- Check C++ compiler installed
- Verify JUCE path is correct

### Runtime Issues

**"Plugin not in DAW"**:
- Verify copied to correct folder
- Rescan plugins
- Check DAW supports MIDI FX

**"White screen"**:
- Check console for errors
- Verify UI files exist
- Check WebView enabled in JUCE

**"Controls don't work"**:
- Open browser console (if possible)
- Check JavaScript errors
- Verify parameter names match

---

## 📁 File Structure

```
01_Perihelion/
├── Source/
│   ├── PluginProcessor.h      ← MIDI processing
│   ├── PluginProcessor.cpp
│   ├── PluginEditor.h          ← WebView UI
│   └── PluginEditor.cpp
├── UI/
│   ├── index.html              ← Main interface
│   ├── styles.css              ← Styling
│   └── app.js                  ← Control logic
├── CMakeLists.txt              ← Build config
└── README.md
```

---

## 🎯 Testing

1. **Load plugin** in DAW ✅
2. **UI displays** correctly ✅
3. **Controls respond** to mouse ✅
4. **MIDI passes through** when bypassed ✅
5. **Velocity sculpting works** when active ✅

---

## 📚 More Info

- **Full build guide**: `BUILD_AND_RUN.md`
- **Quick start**: `QUICK_START.md`
- **Plugin docs**: `01_Perihelion/README.md`

---

**Ready to run! 🚀**
