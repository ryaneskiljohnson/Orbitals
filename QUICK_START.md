# ORBITALS - Quick Start Guide

**Get Perihelion running in 5 minutes!**

---

## ⚡ Fastest Way to Run

### Step 1: Build Perihelion

```bash
cd /Users/rjmacbookpro/Development/Orbitals
./build-perihelion.sh
```

Or manually:
```bash
cd 01_Perihelion
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Step 2: Install Plugin

**macOS**:
```bash
# VST3
cp -R build/Perihelion_artefacts/VST3/Perihelion.vst3 ~/Library/Audio/Plug-Ins/VST3/

# AU
cp -R build/Perihelion_artefacts/AU/Perihelion.component ~/Library/Audio/Plug-Ins/Components/
```

### Step 3: Load in DAW

1. Open your DAW (Logic, Ableton, Reaper, etc.)
2. Rescan plugins
3. Create a MIDI track
4. Insert **Perihelion** as MIDI FX
5. **Beautiful WebView UI opens!** ✨

---

## 🎹 What You'll See

When Perihelion loads, you'll see:

- **Stunning solar-themed interface** with golden/orange colors
- **Gravity knob** (large center control)
- **Solar Point slider** (vertical slider)
- **Orbit arc** (top)
- **Bias slider** (bottom)
- **Real-time particle animation** showing velocity being pulled toward sun
- **Smooth 60fps animations**

---

## 🐛 Troubleshooting

### Build Fails?

**"JUCE not found"**:
- Check JUCE is at: `/Users/rjmacbookpro/Development/Synth/JUCE`
- Or edit `CMakeLists.txt` with your JUCE path

**"UI files not found"**:
- Ensure you're in the Orbitals directory
- Check `01_Perihelion/UI/` has `index.html`, `styles.css`, `app.js`

### Plugin Doesn't Load?

**"Plugin not found in DAW"**:
1. Verify plugin copied to correct location
2. Rescan plugins in DAW
3. Check DAW supports MIDI FX (some don't)

**"White screen"**:
1. Check console for errors
2. Verify UI files exist
3. Try opening HTML in browser to test

---

## 📚 Full Documentation

For detailed build instructions, see:
- `BUILD_AND_RUN.md` - Complete build guide
- `README.md` - Project overview
- `01_Perihelion/README.md` - Plugin-specific docs

---

## 🎯 Success!

You'll know it's working when:
- ✅ Plugin builds without errors
- ✅ Appears in DAW plugin list
- ✅ Beautiful WebView UI opens
- ✅ Controls respond smoothly
- ✅ MIDI processes correctly

---

**Ready to sculpt some velocity! 🎹✨**
