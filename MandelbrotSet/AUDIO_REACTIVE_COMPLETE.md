# Audio-Reactive Animations - Complete ✅

## Overview
All 9 Mandelbrot Set plugins now feature fully audio-reactive animations that respond to live audio input in real-time.

## 🎵 Audio Pipeline

### C++ Side (PluginProcessor)
**Audio Level Tracking:**
- `std::atomic<float> inputLevel` - RMS input level in dB
- `std::atomic<float> outputLevel` - RMS output level in dB  
- `std::atomic<float> currentGainReduction` - For compressor/gate plugins

**processBlock() Updates:**
- Calculates RMS level for all input channels
- Converts to dB scale (-100 dB floor)
- Stores atomically for thread-safe access
- Updates every audio buffer (~20-50ms)

### C++ Side (PluginEditor)
**Timer-Based Metering:**
- Timer runs at 50ms (20Hz) for smooth updates
- `sendMeteringData()` method sends levels to JavaScript
- Uses `webView->emitEventIfBrowserIsVisible("eval", script)`
- Format: `window.receiveAudioData({ inputLevel: X, outputLevel: Y })`

### JavaScript Side (UI)
**receiveAudioData() Function:**
- Receives audio data from C++
- Updates state.inputLevel and state.outputLevel
- Normalizes to 0-1 range for animations
- Available to all canvas animations

## 🎨 Audio-Reactive Features by Plugin

### 1. **Singularity** (Black Hole Compressor)
- Event horizon size pulses with input level
- Core intensity responds to gain reduction
- Particle speed increases with audio
- Particle trails brighten with audio
- Glow intensity scales with level

### 2. **Fabric** (Spacetime Reverb)
- Grid warp amplitude responds to audio
- Line opacity and width scale with level
- Intersection point size pulses
- Fade effect intensity varies
- Overall grid energy increases

### 3. **Entanglement** (Quantum Delay)
- Particle movement amplitude scales
- Connection line thickness responds
- Particle size pulses with audio
- Animation speed increases
- Glow intensity varies

### 4. **Planck** (Quantum Scale EQ)
- Quantum foam bubble size responds
- Bubble intensity scales with audio
- Bubble pulsing rate increases
- More bubbles appear with peaks
- Glow radius expands

### 5. **Observer** (Wavefunction Collapse Dynamic EQ)
- Collapse triggered by audio peaks
- Particle count scales with level
- Superposition orbit radius varies
- Collapsed state size responds
- Pulsing rings intensity scales

### 6. **Hadron** (Particle Collision Saturation)
- Collision frequency responds to audio
- Particle count scales with level
- Particle speed increases
- Explosion size varies
- Particle trails brighten

### 7. **Mandelbrot** (Fractal Phaser)
- Zoom depth responds to audio
- Iteration count increases with level
- Color intensity scales
- Pixel detail improves
- Hue rotation speed varies

### 8. **Quarks** (Color Charge Gate)
- Orbit radius responds to audio
- Gluon connection strength scales
- Quark size pulses
- Rotation speed increases
- Confinement energy visible

### 9. **Rabbithole** (Quantum Descent Flanger)
- Ring count scales with audio
- Ring wobble amplitude increases
- Line width responds to level
- Opacity scales with audio
- Central vortex pulses

## 📊 Audio Mapping

**Normalization:**
```javascript
const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60));
```

**Typical Ranges:**
- -60 dB (silence) → 0.0 (minimum animation)
- -30 dB (moderate) → 0.5 (medium animation)
- 0 dB (peak) → 1.0 (maximum animation)

## 🎯 Animation Parameters Affected

For each plugin, audio reactivity controls:
- ✅ Particle/element size
- ✅ Movement speed
- ✅ Glow intensity
- ✅ Opacity/brightness
- ✅ Color saturation
- ✅ Line thickness
- ✅ Spawn rate/frequency
- ✅ Animation speed
- ✅ Visual complexity

## 🔧 Technical Implementation

**Thread Safety:**
- Atomic variables for lock-free access
- Audio thread writes, UI thread reads
- No blocking or mutex contention

**Performance:**
- RMS calculation per buffer (efficient)
- 20Hz UI update rate (smooth, not wasteful)
- Normalized values cached in state
- RequestAnimationFrame for canvas

**Responsiveness:**
- ~20-50ms audio latency
- 50ms UI update rate
- Smooth interpolation in animations
- No stuttering or frame drops

## ✨ Result

Every plugin's animation now:
- Pulses and breathes with the music
- Scales intensity with audio level
- Responds in real-time (<100ms latency)
- Creates immersive visual feedback
- Enhances the quantum/fractal aesthetic
- Provides professional, polished experience

The animations are alive and reactive, creating a deep connection between the audio processing and visual representation!
