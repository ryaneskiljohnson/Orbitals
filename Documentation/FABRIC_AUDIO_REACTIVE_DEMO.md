# 🌊 Fabric Audio-Reactive Demo

## 🎉 NEW FEATURE: Spacetime Ripples!

Fabric's animation now **dramatically responds to audio input** with ripple effects that bend spacetime!

---

## 🚀 Test It Now

```bash
./MandelbrotSet/Fabric/launch_fabric.sh
```

Or:
```bash
open MandelbrotSet/Fabric/Builds/MacOSX/build/Debug/Fabric.app
```

---

## 🎤 How to See the Effect

1. **Launch Fabric** (use script above)
2. **Allow microphone** when prompted
3. **Click ⚙️ → Select "Built-in Microphone"**
4. **Speak, clap, or play music** into your microphone
5. **Watch the fabric bend and ripple!**

---

## ✨ What You'll See

### Quiet/Silence
- Gentle wave motion on the grid
- Subtle cyan glow
- Calm baseline animation

### Moderate Volume
- Grid warps more dramatically
- Brighter lines
- Occasional ripples from center

### **Loud Audio (The Fun Part!)**
- 🌊 **Multiple expanding ripple waves**
- 💥 **Dramatic spacetime bending**
- ✨ **Bright glowing center burst**
- 🔵 **Grid points pulse larger**
- 🌟 **Lines glow intensely**

### Audio Peaks/Transients
- **Instant ripple creation** on sudden sounds
- Waves expand outward from center
- Grid points displaced by ripple energy
- Creates visible "impact" in spacetime

---

## 🎨 Visual Effects

### Ripple Waves
- Cyan circles expand from center
- Triggered by audio peaks
- Push/pull grid points as they pass
- Fade out gracefully
- Multiple ripples can overlap

### Spacetime Warping
- Grid bends more with louder audio
- Points displaced radially
- Wave motion synchronized to audio
- Creates "gravity well" effect

### Energy Burst
- Glowing center during loud audio
- Radial gradient (bright to transparent)
- Pulses with audio level
- Cyan/white glow

---

## 🎯 Try These

### For Best Effect:

1. **Clap near the microphone**
   - Watch instant ripple burst
   - Grid dramatically warps
   - Multiple waves expand

2. **Speak continuously**
   - Constant ripples on syllables
   - Grid pulses with speech
   - Visual feedback for every word

3. **Play music**
   - Bass hits create ripples
   - Grid warps with dynamics
   - Continuous spacetime bending

4. **Adjust Wet/Dry slider**
   - Move it from 0% to 100%
   - Hear reverb increase
   - See visuals respond

---

## 🎊 The Experience

**It's like watching sound bend spacetime!**

- Audio becomes visible as ripples in the fabric
- Loud sounds create "gravitational waves"
- The grid behaves like actual spacetime
- Real-time visual feedback for audio processing

---

## 🔧 Technical Implementation

### Audio Detection
- Peak detection algorithm
- Threshold: 0.3 normalized level
- Sensitivity: 0.15 level change
- Update rate: 60 FPS

### Ripple Physics
- Expansion speed: 4 pixels/frame
- Max radius: 300-500 pixels (audio-dependent)
- Strength: 0-2 (based on audio level)
- Lifetime: Automatically culled when expanded

### Grid Displacement
- Base wave: 8-33 pixels (audio-dependent)
- Ripple displacement: Up to 40 pixels per ripple
- Combined effects for dramatic warping
- Smooth interpolation

---

## 🎉 Result

**Fabric is now a living, breathing spacetime visualizer!**

Launch it and **speak into your microphone** to see the fabric of spacetime ripple and bend in real-time. 

The animation makes the reverb effect **visible** - you can literally see sound warping space! 🌊✨

---

*Test it now: `./MandelbrotSet/Fabric/launch_fabric.sh`*
