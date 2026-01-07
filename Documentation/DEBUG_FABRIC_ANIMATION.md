# Debug Fabric Animation

## 🔍 Debugging Steps

### 1. Launch Fabric
```bash
open /Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Fabric/Builds/MacOSX/build/Debug/Fabric.app
```

### 2. Open Browser Console
- **Right-click** anywhere in the plugin window
- Select **"Inspect Element"**
- Go to **"Console"** tab

### 3. Check Console Messages

You should see these messages in order:

```
🔵 DOMContentLoaded fired
🧪 Starting automatic bridge test...
🔵 Initializing controls...
🎨 Initializing Fabric animation...
✅ Canvas found: [object HTMLCanvasElement]
✅ Canvas configured: 700x700
✅ Starting animation loop...
🎬 Frame 0: time=0.02, inputLevel=-100
🎬 Frame 1: time=0.04, inputLevel=-100
🎬 Frame 2: time=0.06, inputLevel=-100
...
✅ Animation started, calling updateSpaceInfo...
✅ Fabric animation fully initialized
🔵 All controls initialized
```

### 4. Check Audio Data

After a few seconds, you should see:
```
🎵 Audio levels: Input=-XX.XdB, Output=-XX.XdB
🎨 Animation: audioLevel=-XX.XdB, normalized=0.XXX, ripples=X
```

### 5. Test Audio Input

**Speak into microphone**, then check console:
- Input level should change from -100dB to higher (e.g., -30dB)
- Normalized level should increase
- If loud enough, you'll see: `🌊 RIPPLE TRIGGERED!`

---

## 🐛 If Animation is Frozen

### Check 1: Is canvas element present?
```javascript
// In console:
document.getElementById('fabricCanvas')
```
Should return: `<canvas id="fabricCanvas" width="700" height="700"></canvas>`

### Check 2: Is animation loop running?
```javascript
// In console:
window.frameCount
```
Should be increasing (check multiple times)

### Check 3: Is requestAnimationFrame working?
```javascript
// In console:
let testCount = 0;
function testAnim() {
    testCount++;
    if (testCount < 10) requestAnimationFrame(testAnim);
}
testAnim();
setTimeout(() => console.log('Test frames:', testCount), 1000);
```
Should show ~60 frames after 1 second

### Check 4: Are there JavaScript errors?
Look for red error messages in console

---

## 🔧 Manual Test

If animation is frozen, try manually triggering it:

```javascript
// In console:
initializeFabricAnimation();
```

This will reinitialize the animation.

---

## 📊 Expected Console Output

### On Load:
```
🔵 DOMContentLoaded fired
🎨 Initializing Fabric animation...
✅ Canvas found
✅ Canvas configured: 700x700
✅ Starting animation loop...
🎬 Frame 0: time=0.02, inputLevel=-100
🎬 Frame 1: time=0.04, inputLevel=-100
...
✅ Fabric animation fully initialized
```

### During Playback:
```
🎵 Audio levels: Input=-25.3dB, Output=-22.1dB
🎨 Animation: audioLevel=-25.3dB, normalized=0.578, ripples=0
🌊 RIPPLE TRIGGERED! Level=0.65, Diff=0.18
```

---

## 🎯 What to Report

Please share:
1. **All console messages** you see
2. **Any red errors**
3. **Value of `window.frameCount`** after 5 seconds
4. **Result of `document.getElementById('fabricCanvas')`**

This will help identify the issue!
