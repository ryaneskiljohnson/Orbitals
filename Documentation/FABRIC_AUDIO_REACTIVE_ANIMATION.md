# Fabric Audio-Reactive Animation

## 🎨 Enhanced Spacetime Bending Visuals

**Status**: ✅ **IMPLEMENTED**  
**Type**: Audio-reactive canvas animation

---

## ✨ Features

### 1. Dynamic Ripple Effects
**When audio peaks occur** (sudden volume increases):
- Ripples emanate from center of fabric
- Wave circles expand outward
- Strength based on audio intensity
- Automatically fade out over time

### 2. Spacetime Grid Warping
**The fabric grid responds to audio in real-time**:
- Grid points displace based on audio level
- Higher audio = more dramatic warping
- Smooth wave motion through the grid
- Creates visual "gravity well" effect

### 3. Audio-Reactive Properties
**Multiple visual elements respond to audio**:

| Element | Response |
|---------|----------|
| **Grid warp** | Amplitude increases with audio level |
| **Line brightness** | Glows brighter with louder audio |
| **Line thickness** | Gets thicker with audio peaks |
| **Point size** | Grid intersections pulse larger |
| **Point brightness** | Shines brighter during peaks |
| **Center burst** | Energy glow appears at high levels |
| **Ripple waves** | Triggered by audio transients |

---

## 🎯 How It Works

### Audio Level Detection
```javascript
// Audio level from C++ (sent every frame)
const audioLevel = state.inputLevel; // -60 to 0 dB
const normalizedLevel = (audioLevel + 60) / 60; // 0 to 1
```

### Ripple Trigger System
```javascript
// Detects sudden audio increases
const levelDiff = normalizedLevel - lastAudioLevel;
if (levelDiff > 0.15 && normalizedLevel > 0.3) {
    // Create ripple wave
    ripples.push({
        x: centerX,      // Origin
        y: centerY,
        radius: 0,       // Starts small
        maxRadius: 500,  // Expands outward
        strength: normalizedLevel * 2,
        age: 0
    });
}
```

### Spacetime Warping
```javascript
// Each grid point displaced by:
// 1. Base wave (constant motion)
const waveOffset = sin(dist * 0.02 + time) * baseWarp;

// 2. Ripple effects (audio-triggered)
ripples.forEach(ripple => {
    const rippleWave = sin((rippleDist - ripple.radius) * 0.1);
    const rippleStrength = rippleWave * ripple.strength * 20;
    // Apply displacement toward/away from ripple origin
});

// 3. Combined displacement
point.x = baseX + waveX + rippleX;
point.y = baseY + waveY + rippleY;
```

---

## 🌊 Visual Effects

### Quiet Audio (Low Level)
- Gentle wave motion
- Dim cyan grid
- Small grid points
- No ripples
- Subtle animation

### Moderate Audio (Medium Level)
- More pronounced warping
- Brighter grid lines
- Larger grid points
- Occasional ripples
- Increased brightness

### Loud Audio (High Level)
- **Dramatic spacetime bending**
- **Multiple expanding ripples**
- **Bright glowing grid**
- **Large pulsing points**
- **Center energy burst**
- Creates visual "impact" effect

---

## 🎨 Color Scheme

| Color | Purpose | RGB |
|-------|---------|-----|
| **Cyan** | Grid lines | rgb(0, 229, 255) |
| **Bright Cyan** | Grid points | rgb(102, 255, 255) |
| **Dark Blue** | Background | rgb(13, 13, 21) |
| **Glow** | Center burst | Radial gradient (cyan) |

All colors respond to audio intensity with increased opacity/brightness.

---

## ⚡ Performance

### Optimization
- **Canvas-based** (hardware accelerated)
- **~60 FPS** animation
- **Efficient grid rendering**
- **Smart ripple culling** (removed when expanded)
- **Minimal memory allocation**

### Grid Stats
- **Grid points**: ~300 (17x17 grid at 40px spacing)
- **Update rate**: Every frame (~16ms)
- **Ripples**: Dynamic (created on audio peaks)
- **Max ripples**: Auto-limited by lifetime

---

## 🎯 User Experience

### What You'll See

1. **Speak or play audio** → Grid warps and pulses
2. **Loud sounds** → Ripples emanate from center
3. **Continuous audio** → Constant spacetime bending
4. **Silence** → Gentle baseline wave motion

### The "Fabric" Metaphor

The animation visualizes **spacetime as a fabric**:
- **Audio input** = objects with mass
- **Ripples** = gravitational waves
- **Grid warping** = spacetime curvature
- **Brightness** = energy density

---

## 🔧 Technical Details

### Audio Data Flow
```
1. C++ AudioProcessor
   ↓ Calculates audio levels (input/output)
   
2. PluginEditor::timerCallback()
   ↓ Every 50ms, sends levels to JavaScript
   
3. window.receiveAudioData({ inputLevel, outputLevel })
   ↓ Updates state.inputLevel
   
4. Animation loop
   ↓ Uses state.inputLevel to drive visuals
   ↓ Creates ripples on peaks
   ↓ Warps grid points
   ↓ Adjusts brightness/size
```

### Key Functions
- `initializeFabricAnimation()` - Sets up canvas and animation loop
- `animate()` - Main render loop (60 FPS)
- `receiveAudioData()` - Receives audio levels from C++
- Ripple system - Detects peaks and creates expanding waves

---

## 🎉 Result

**Fabric now has dramatic audio-reactive visuals!**

- ✅ Ripples triggered by audio peaks
- ✅ Spacetime grid warps with audio
- ✅ Brightness responds to volume
- ✅ Center energy burst at high levels
- ✅ Smooth 60 FPS animation
- ✅ Real-time response to audio input

**The fabric of spacetime visually bends in response to sound!** 🌊✨

---

*Implemented: January 6, 2026*
