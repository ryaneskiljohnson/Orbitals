# Test Fabric Audio-Reactive Animation

## 🧪 Testing Steps

### 1. Launch Fabric
```bash
./MandelbrotSet/Fabric/launch_fabric.sh
```

### 2. Open Browser Console
- Right-click anywhere in the plugin window
- Select "Inspect Element" or "Inspect"
- Go to the "Console" tab

### 3. Check for Debug Messages

You should see:
```
🎵 Audio levels: Input=-XX.XdB, Output=-XX.XdB
🎨 Animation: audioLevel=-XX.XdB, normalized=0.XXX, ripples=X
```

If you see these messages, the animation is receiving audio data.

### 4. Test Audio Response

**Speak into microphone** and watch console:
- Audio levels should change
- Normalized level should increase (0 to 1)
- When level jumps, you should see: `🌊 RIPPLE TRIGGERED!`

### 5. Visual Verification

**What to look for**:
- Grid should warp more with louder audio
- Lines should get brighter
- Grid points should pulse larger
- **Ripples should expand from center on peaks**
- Center should glow during loud audio

---

## 🔍 Troubleshooting

### If No Console Messages

**Problem**: Timer not running or JavaScript not loaded

**Check**:
1. Is the WebView visible?
2. Did the page finish loading?
3. Is `window.receiveAudioData` defined?

**Test in console**:
```javascript
console.log('receiveAudioData exists:', typeof window.receiveAudioData);
console.log('Current state:', state);
```

### If Audio Levels Always -100dB

**Problem**: No audio input configured

**Fix**:
1. Click ⚙️ settings button
2. Select "Built-in Microphone" as input
3. Allow microphone permissions if prompted
4. Speak into microphone

### If Levels Update But No Ripples

**Problem**: Audio not loud enough or threshold too high

**Check**:
```javascript
// In console, check normalized level
console.log('Normalized level:', (state.inputLevel + 60) / 60);
```

Should be > 0.3 for ripples to trigger.

**Lower threshold temporarily**:
```javascript
// In console
peakTriggerThreshold = 0.1;
```

### If Animation Not Running

**Check**:
```javascript
// In console
console.log('Canvas exists:', !!document.getElementById('fabricCanvas'));
console.log('Animation initialized:', typeof initializeFabricAnimation);
```

---

## ✅ Expected Behavior

### Quiet (-60 to -40 dB)
- Gentle wave motion
- Dim grid
- No ripples
- Baseline animation

### Moderate (-40 to -20 dB)
- More warping
- Brighter lines
- Occasional ripples
- Increased brightness

### Loud (-20 to 0 dB)
- **Dramatic warping**
- **Multiple ripples**
- **Bright glowing grid**
- **Center energy burst**
- **Large pulsing points**

---

## 🎯 Quick Test

**In the browser console, force a ripple**:
```javascript
// Manually trigger ripple to test visual
state.inputLevel = -10; // Simulate loud audio
```

You should immediately see the grid warp and brighten.

---

## 📊 Debug Info to Check

1. **Audio data arriving?**
   ```javascript
   console.log('Input level:', state.inputLevel);
   ```

2. **Animation running?**
   ```javascript
   console.log('Animation frame count:', window.animDebugCounter);
   ```

3. **Ripples being created?**
   ```javascript
   console.log('Ripple count:', ripples.length);
   ```

4. **Normalized level correct?**
   ```javascript
   const normalized = (state.inputLevel + 60) / 60;
   console.log('Normalized:', normalized);
   ```

---

*Test and report back what you see in the console!*
