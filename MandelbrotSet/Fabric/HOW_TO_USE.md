# Fabric Reverb - Usage Instructions

## ✅ Status: Production Ready

All test code removed. Plugin now uses real audio input from configured device.

---

## 🎧 How to Use Fabric Standalone

### Step 1: Open the App
```bash
open /Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Fabric/Builds/MacOSX/build/Debug/Fabric.app
```

Or navigate in Finder to:
```
MandelbrotSet/Fabric/Builds/MacOSX/build/Debug/Fabric.app
```

### Step 2: Configure Audio (First Time Only)

1. **Click the ⚙️ Settings button** (top-right corner)
2. **In the Audio Settings dialog:**
   - **Input Device**: Select your microphone or audio interface
   - **Output Device**: Select your speakers or headphones
   - **Sample Rate**: 44100 Hz (or your preferred rate)
   - **Buffer Size**: 512 samples (or adjust for latency)
3. **Click OK**

### Step 3: Send Audio to Fabric

Choose one of these options:

#### Option A: Microphone Input
- Talk or play an instrument into your microphone
- You'll hear the reverb effect in real-time

#### Option B: System Audio (macOS)
- Use a tool like BlackHole or Loopback to route system audio
- Play music from Spotify/iTunes/browser
- Fabric will process the audio with reverb

#### Option C: Audio Interface
- Connect an audio interface
- Send audio from external source
- Fabric processes it through reverb

### Step 4: Adjust Controls

All controls affect the reverb in real-time:

- **Weave** (Size): Room size - larger = longer reverb tail
- **Thread Density** (Diffusion): Stereo width - higher = wider
- **Fold** (Damping): High-frequency absorption - higher = darker
- **Time Dilation** (Predelay): Delay before reverb (0-200ms)
- **Warp** (Mix): Overall reverb intensity
- **Wet/Dry**: Balance between dry signal and reverb

---

## 🎚️ Using Fabric in a DAW

### VST3 Installation
1. Copy `Fabric.vst3` to:
   - macOS: `~/Library/Audio/Plug-Ins/VST3/`
   - Windows: `C:\Program Files\Common Files\VST3\`

2. Rescan plugins in your DAW

3. Insert Fabric on an audio track (not MIDI track)

### Audio Unit (AU) Installation
1. Copy `Fabric.component` to:
   - `~/Library/Audio/Plug-Ins/Components/`

2. Rescan plugins in your DAW

3. Insert Fabric on an audio track

### In Your DAW
- Insert Fabric on an **audio track** (not MIDI/instrument track)
- Send audio to the track
- Fabric processes it with reverb
- No additional configuration needed

---

## 🔧 Controls Reference

### Weave (Size)
- **Range**: 0% - 100%
- **Effect**: Controls reverb room size and decay time
- **Usage**: Small rooms (10-30%), halls (50-70%), cathedrals (80-100%)

### Thread Density (Diffusion)
- **Range**: 0% - 100%
- **Effect**: Controls stereo width of reverb
- **Usage**: Narrow (20-40%), normal (50%), wide (70-100%)

### Fold (Damping)
- **Range**: 0% - 100%
- **Effect**: High-frequency absorption
- **Usage**: Bright (0-30%), natural (40-60%), dark (70-100%)

### Time Dilation (Predelay)
- **Range**: 0ms - 200ms
- **Effect**: Delay before reverb starts
- **Usage**: Vocals (20-50ms), instruments (0-30ms), special FX (100-200ms)

### Warp (Mix)
- **Range**: 0% - 100%
- **Effect**: Overall reverb intensity
- **Usage**: Subtle (10-30%), normal (40-60%), heavy (70-100%)

### Wet/Dry
- **Range**: 0% - 100%
- **Effect**: Balance between original signal and reverb
- **Usage**:
  - 0% = 100% dry (no reverb)
  - 50% = 50/50 mix
  - 100% = 100% wet (reverb only)

---

## 💡 Tips

### Getting Good Reverb Sounds

1. **For Vocals**:
   - Size: 60-75%
   - Diffusion: 50-70%
   - Damping: 40-60%
   - Predelay: 20-50ms
   - Mix: 40-60%
   - Wet/Dry: 25-40%

2. **For Drums**:
   - Size: 40-60%
   - Diffusion: 60-80%
   - Damping: 50-70%
   - Predelay: 0-20ms
   - Mix: 50-70%
   - Wet/Dry: 30-50%

3. **For Pads/Synths**:
   - Size: 70-90%
   - Diffusion: 70-90%
   - Damping: 30-50%
   - Predelay: 50-100ms
   - Mix: 60-80%
   - Wet/Dry: 50-80%

---

## ❗ Troubleshooting

### No Audio
1. Check that an **input device is selected** in settings (⚙️)
2. Ensure audio is actually being sent to Fabric
3. Check macOS system volume
4. Verify output device is correct

### Latency Issues
1. Open settings (⚙️)
2. Reduce buffer size (try 256 or 128 samples)
3. Note: Smaller buffer = lower latency but higher CPU usage

### No Reverb Effect
1. Check **Mix (Warp)** is not at 0%
2. Check **Wet/Dry** is not at 0% (100% dry)
3. Try increasing **Size (Weave)** to 70%+

---

## 📊 Technical Specs

- **Plugin Type**: Audio Effects (Reverb)
- **Format**: VST3, AU, Standalone
- **Audio**: Stereo In / Stereo Out
- **Sample Rates**: 44.1kHz, 48kHz, 88.2kHz, 96kHz, 192kHz
- **Latency**: Depends on buffer size (default: 512 samples = ~11.6ms @ 44.1kHz)
- **CPU Usage**: Low to moderate (depends on settings)
- **Parameters**: 6 (all automatable in DAW)

---

## ✅ Verification Complete

The plugin has been **comprehensively tested**:
- ✅ Audio input routing working
- ✅ Audio output routing working
- ✅ All 6 parameters affecting DSP
- ✅ Reverb processing correctly
- ✅ Bypass working
- ✅ Wet/Dry balance correct
- ✅ Follows all JUCE standards

**The plugin is ready for use with real audio input.**

---

**Quick Start**: Open app → Click ⚙️ → Select input device → Play audio → Hear reverb!
