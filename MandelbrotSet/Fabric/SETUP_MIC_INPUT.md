# Setting Up Microphone Input for Fabric

## ✅ Fixed: Microphone Permissions Added

The app now has the required microphone permission entry in Info.plist.

---

## 🎤 Steps to Enable Microphone Input

### 1. Grant Microphone Permission (macOS will prompt you)

When you open Fabric, macOS will show a dialog:

```
"Fabric" would like to access the microphone.
Fabric needs microphone access to process audio input for reverb effects.

[Don't Allow]  [OK]
```

**Click OK** to allow microphone access.

---

### 2. Configure Audio in Fabric

1. **Click the ⚙️ Settings button** (top-right corner of Fabric window)

2. **In the Audio Settings dialog**:
   - **Input Device**: Select "Built-in Microphone" or "MacBook Pro Microphone"
   - **Output Device**: Select "Built-in Output" or "MacBook Pro Speakers"
   - **Sample Rate**: 44100 Hz (recommended)
   - **Buffer Size**: 512 samples (default) or 256 for lower latency
   
3. **Click OK**

---

### 3. Test the Microphone

1. **Speak into your MacBook's microphone** (near the keyboard)
2. **Watch for Input check messages** in the console (if running from Terminal)
3. **You should hear your voice with reverb** coming from the speakers

**Expected:** Your voice → Reverb processing → Speakers output

---

## 🔧 If Mic Still Doesn't Work

### Check macOS Permissions

1. Open **System Settings**
2. Go to **Privacy & Security**
3. Click **Microphone**
4. Find **Fabric** in the list  
5. Ensure the toggle is **ON** (enabled)

### Check Audio Settings

1. In Fabric, click **⚙️ Settings**
2. Verify:
   - Input device is **NOT** "No device"
   - Input device is **Built-in Microphone**  
   - Sample rate matches (44100 Hz)
   - Active input channels > 0

### Test Your Microphone

1. Open **System Settings → Sound → Input**
2. Select **Built-in Microphone**
3. **Speak** and watch the input level meters
4. If meters don't move, your mic has a hardware/system issue

---

## 📊 Debug Mode

To see detailed input monitoring:

```bash
/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Fabric/Builds/MacOSX/build/Debug/Fabric.app/Contents/MacOS/Fabric
```

**Look for these messages:**
```
Input: RMS=0.123 Max=0.456 | Channels=2
```

- If RMS and Max are both 0.0: No mic input is being received
- If RMS and Max are > 0.0: Mic is working!

---

## ✅ What's Been Fixed

1. **Microphone permission** added to Info.plist ✅
2. **Input bus layout** fixed to accept mono and stereo ✅
3. **Debug logging** added to verify input levels ✅
4. **All test signals** removed (now uses real input) ✅

---

## 🎚️ Expected Behavior

When mic input is working correctly:

1. **Speak into mic** → Input levels > 0
2. **Reverb processes** → Output levels > Input levels (due to reflections)
3. **Hear from speakers** → Your voice with beautiful reverb
4. **Move controls** → Reverb sound changes in real-time

**Input: 0.5** → **Reverb Processing** → **Output: 0.69** (typical)

---

**If mic input still doesn't work after following all steps, there may be a macOS audio system issue. Try restarting your Mac.**
