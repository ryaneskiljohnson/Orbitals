# Fabric Animation - Parameter Mapping

## 🎨 How Each Parameter Affects the Animation

### 1. SIZE (Weave) 🌊
**DSP Effect**: Room size / reverb tail length  
**Animation Effect**:
- Controls grid wave amplitude
- Larger size = bigger spacetime warps
- Adds to base warp amount
- Range: 0-15 pixels additional warp

**Visual**: Move SIZE up → Grid warps more dramatically

---

### 2. DIFFUSION (Thread Density) ✨
**DSP Effect**: Stereo width / reflection density  
**Animation Effect**:
- Controls wave frequency/complexity
- Higher diffusion = faster, more complex waves
- Increases grid point size
- Enhances ripple glow visibility
- Range: 0.02-0.05 wave frequency

**Visual**: Move DIFFUSION up → Waves move faster, points get larger

---

### 3. DAMPING (Fold) 🌫️
**DSP Effect**: High-frequency absorption  
**Animation Effect**:
- Controls fade/trail intensity
- Higher damping = more motion blur/trails
- Creates "softer" visual feel
- Range: 0.05-0.10 fade intensity

**Visual**: Move DAMPING up → More trails/ghosting effect

---

### 4. PREDELAY (Time Dilation) ⏱️
**DSP Effect**: Initial delay before reverb  
**Animation Effect**:
- Slows down animation speed
- Creates "time dilation" effect
- Higher predelay = slower motion
- Range: 1.0x to 1.25x time scaling

**Visual**: Move PREDELAY up → Animation moves in slow motion

---

### 5. MIX (Warp) 🌀
**DSP Effect**: Overall reverb intensity  
**Animation Effect**:
- Master control for animation intensity
- Scales all warping effects
- Affects line thickness
- Range: 0-100% intensity multiplier

**Visual**: Move MIX up → Everything becomes more intense

---

### 6. WET/DRY 💧
**DSP Effect**: Balance between dry and reverb signal  
**Animation Effect**:
- Controls line brightness
- Affects center burst visibility
- Controls ripple opacity
- Range: 0-100% visual intensity

**Visual**: Move WET/DRY up → Grid glows brighter, ripples more visible

---

### 7. AUDIO INPUT 🎤
**Effect**: Real-time audio level  
**Animation Effect**:
- Triggers ripples on peaks
- Increases warp amplitude
- Brightens all elements
- Enlarges grid points
- Creates center energy burst

**Visual**: Speak/play audio → Ripples emanate, grid warps dramatically

---

## 🎯 Combined Effects

### Example 1: Subtle Reverb
- SIZE: 20%
- DIFFUSION: 30%
- DAMPING: 40%
- MIX: 30%
- WET/DRY: 40%

**Animation**: Gentle waves, dim grid, small warps, subtle trails

---

### Example 2: Massive Cathedral
- SIZE: 90%
- DIFFUSION: 70%
- DAMPING: 60%
- MIX: 80%
- WET/DRY: 70%

**Animation**: Dramatic warps, bright glowing grid, large points, heavy trails, intense ripples

---

### Example 3: Dry Signal (No Reverb)
- MIX: 0%
- WET/DRY: 0%

**Animation**: Minimal warping, dim grid, no center burst, subtle ripples

---

## 🌊 Audio Reactivity

### Quiet Audio (-60 to -40 dB)
- Baseline animation only
- Parameter effects visible
- No ripples
- Gentle motion

### Moderate Audio (-40 to -20 dB)
- Increased warping
- Occasional ripples
- Brighter grid
- Parameter effects amplified

### Loud Audio (-20 to 0 dB)
- **Dramatic ripples**
- **Intense warping**
- **Bright glowing grid**
- **Center energy burst**
- **All parameters at maximum visual effect**

---

## 🎨 Visual Feedback Summary

| Parameter | Visual Effect | Intensity |
|-----------|--------------|-----------|
| **SIZE** | Warp amplitude | High |
| **DIFFUSION** | Wave speed + point size | Medium |
| **DAMPING** | Trail/fade amount | Low |
| **PREDELAY** | Animation speed | Low |
| **MIX** | Overall intensity | High |
| **WET/DRY** | Brightness + ripples | High |
| **AUDIO** | Everything! | Maximum |

---

## 🎯 Design Philosophy

**Every parameter that affects the sound also affects the visuals:**

- **Spatial parameters** (Size, Diffusion) → Spatial visual changes
- **Temporal parameters** (Predelay) → Temporal visual changes
- **Mix parameters** (Mix, Wet/Dry) → Overall visual intensity
- **Damping** → Visual smoothness/trails
- **Audio** → Real-time reactive effects

**Result**: The animation is a true visual representation of what the reverb is doing to the audio!

---

*Now test it: Move each knob and watch the animation respond!* 🎨
