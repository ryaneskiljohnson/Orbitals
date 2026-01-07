# Mandelbrot Series - Audio-Reactive Animation Plan

## 🎯 Goal
Add audio-reactive animations to all 8 remaining Mandelbrot plugins, each appropriate to their effect type.

---

## ✅ COMPLETE: Fabric (Reverb)
- Spacetime grid with ripples
- All parameters affect animation
- Audio-reactive warping
- **Reference implementation**

---

## 🎨 Animation Designs for Each Plugin

### 1. Entanglement (Delay) - Quantum Entanglement Particles
**Theme**: Entangled particle pairs

**Animation**:
- Pairs of particles connected by lines
- Particles move in synchronized patterns
- Delay time = distance between pairs
- Feedback = line brightness
- Mix = number of visible pairs
- Audio peaks create new particle pairs

**Parameters**:
- TIME: Distance between entangled pairs
- FEEDBACK: Line connection brightness
- MIX: Number of visible pairs
- DAMPING: Particle fade rate

---

### 2. Hadron (Saturation) - Particle Collision
**Theme**: Particle accelerator collisions

**Animation**:
- Particles orbiting in circular paths
- Collisions at center create bursts
- Drive = particle speed
- Tone = particle color (bright to dark)
- Bias = orbit asymmetry
- Audio peaks trigger collision bursts

**Parameters**:
- DRIVE: Particle velocity
- TONE: Color temperature (bright/dark)
- BIAS: Orbit shape (circular to elliptical)
- MIX: Collision intensity

---

### 3. Mandelbrot (Phaser) - Fractal Iteration
**Theme**: Mandelbrot set iteration

**Animation**:
- Rotating spiral arms
- Fractal patterns emerge
- Rate = rotation speed
- Depth = spiral complexity
- Stages = number of arms
- Audio creates fractal blooms

**Parameters**:
- RATE: Rotation speed
- DEPTH: Spiral arm length
- STAGES: Number of spiral arms
- FEEDBACK: Spiral brightness

---

### 4. Observer (Dynamic EQ) - Wavefunction Collapse
**Theme**: Quantum measurement

**Animation**:
- Probability cloud particles
- Collapse to specific frequencies
- Frequency = cloud center position
- Threshold = collapse trigger
- Audio causes wavefunction collapse

**Parameters**:
- FREQUENCY: Cloud center position
- THRESHOLD: Collapse sensitivity
- RATIO: Collapse speed
- ATTACK/RELEASE: Particle behavior

---

### 5. Planck (EQ) - Quantum Energy Levels
**Theme**: Discrete energy levels

**Animation**:
- Horizontal energy bands (3 levels)
- Particles jumping between levels
- Band brightness = gain
- Frequency = band position
- Audio causes quantum jumps

**Parameters**:
- LOW/MID/HIGH FREQ: Band positions
- LOW/MID/HIGH GAIN: Band brightness
- Q: Band width

---

### 6. Quarks (Gate) - Color Charge
**Theme**: Quark confinement

**Animation**:
- Particles with color charges (RGB)
- Confined in boundary when below threshold
- Released when above threshold
- Threshold = boundary size
- Audio triggers particle release

**Parameters**:
- THRESHOLD: Confinement boundary size
- RATIO: Release intensity
- RANGE: Maximum particle spread

---

### 7. Rabbithole (Flanger) - Wormhole
**Theme**: Falling through wormhole

**Animation**:
- Spiral tunnel effect
- Rotating inward motion
- Rate = rotation speed
- Depth = tunnel depth
- Stereo = tunnel wobble
- Audio creates tunnel pulses

**Parameters**:
- RATE: Spiral rotation speed
- DEPTH: Tunnel depth/perspective
- STEREO: Tunnel wobble amount
- FEEDBACK: Tunnel brightness

---

### 8. Singularity (Compressor) - Black Hole
**Theme**: Gravitational compression

**Animation**:
- Particles spiraling into center
- Compression ring at threshold
- Particles compressed at center
- Threshold = event horizon size
- Audio pulls particles inward

**Parameters**:
- THRESHOLD: Event horizon radius
- RATIO: Compression strength
- ATTACK/RELEASE: Particle acceleration
- MAKEUP: Particle density

---

## 📊 Implementation Strategy

### Phase 1: Core Animation (Each Plugin)
1. Create canvas element
2. Initialize animation loop
3. Basic visual theme
4. Audio reactivity

### Phase 2: Parameter Integration
1. Map each parameter to visual property
2. Update visuals when parameters change
3. Ensure smooth transitions

### Phase 3: Audio Reactivity
1. Receive audio levels from C++
2. Trigger effects on peaks
3. Scale intensity with volume
4. Smooth response

---

## 🔧 Technical Pattern (Reusable)

### JavaScript Structure
```javascript
function initialize[Plugin]Animation() {
    const canvas = document.getElementById('[plugin]Canvas');
    const ctx = canvas.getContext('2d');
    
    // Setup
    let time = 0;
    let particles = [];
    
    function animate() {
        // 1. Get audio level
        const audioLevel = state.inputLevel;
        const normalized = (audioLevel + 60) / 60;
        
        // 2. Get parameters
        const param1 = state.param1 / 100;
        const param2 = state.param2 / 100;
        
        // 3. Update physics
        updateParticles(param1, param2, normalized);
        
        // 4. Render
        render(ctx, normalized);
        
        requestAnimationFrame(animate);
    }
    
    animate();
}
```

### C++ Integration (Already Done)
- Timer sends audio levels every 50ms
- JavaScript receives via `window.receiveAudioData()`
- State updated automatically

---

## 🎯 Next Steps

1. ✅ **Fabric** - Complete with all parameters
2. 🚧 **Entanglement** - Implement entangled particles
3. 🚧 **Hadron** - Implement particle collisions
4. 🚧 **Mandelbrot** - Implement fractal spirals
5. 🚧 **Observer** - Implement wavefunction collapse
6. 🚧 **Planck** - Implement energy levels
7. 🚧 **Quarks** - Implement color charge
8. 🚧 **Rabbithole** - Implement wormhole tunnel
9. 🚧 **Singularity** - Implement black hole

---

*Each animation will be unique and thematically appropriate to its effect!*
