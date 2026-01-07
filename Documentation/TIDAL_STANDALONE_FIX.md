# Tidal Standalone Executable Fix

## 🔍 Root Cause Analysis

### The Problem
Tidal.app bundle was created but contained **no executable binary**, causing the error:
```
The bundle located at ".../Tidal.app" doesn't contain an executable.
```

### Forensic Comparison: Tidal vs Kepler

**Key Finding:** Tidal's standalone target had an **empty `PBXSourcesBuildPhase`** in the Xcode project, meaning no source files were being compiled for the standalone app.

**Evidence:**
```bash
# Tidal's standalone sources build phase:
EFCF3544C31FFE7FB4DB305F = {
    isa = PBXSourcesBuildPhase;
    files = (
    );  # ← EMPTY!
};

# Kepler's standalone sources build phase:
7C3B50D76D741C49F2835C68 = {
    isa = PBXSourcesBuildPhase;
    files = (
    );  # Also empty, but Kepler works...
};
```

### The Real Issue

**Tidal.jucer was missing `Main.cpp` and `MainComponent` files** in the Source group. These files are required for standalone builds:

- `Main.cpp` - Contains `START_JUCE_APPLICATION` macro that generates the `main()` entry point
- `MainComponent.h` / `MainComponent.cpp` - The main UI component for the standalone app

### Why Kepler Works

Kepler's Xcode project was likely generated at a different time or with different Projucer settings that automatically included the standalone wrapper, even though `Main.cpp` isn't explicitly listed in `Kepler.jucer` either.

## ✅ The Fix

Added `Main.cpp` and `MainComponent` files to `Tidal.jucer`:

```xml
<GROUP id="{D7F8A864-8ECA-4FE0-8FEC-20B87407D899}" name="Source">
  <FILE id="Main.cpp" name="Main.cpp" compile="1" resource="0" file="Source/Main.cpp" />
  <FILE id="MainComponent.h" name="MainComponent.h" compile="0" resource="0" file="Source/MainComponent.h" />
  <FILE id="MainComponent.cpp" name="MainComponent.cpp" compile="1" resource="0" file="Source/MainComponent.cpp" />
  <!-- ... existing PluginProcessor/PluginEditor files ... -->
</GROUP>
```

## 🔧 Next Steps

**You MUST regenerate the Xcode project in Projucer:**

1. Open `Tidal.jucer` in JUCE Projucer
2. Click **"Save and Open in IDE"** (or File → Save Project)
3. This will regenerate the Xcode project with the correct source file references
4. Build the "Tidal - Standalone Plugin" target in Xcode

After regeneration, the standalone target's `PBXSourcesBuildPhase` should include:
- `Main.cpp`
- `MainComponent.cpp`
- `PluginProcessor.cpp`
- `PluginEditor.cpp`
- And the JUCE-generated standalone wrapper

## Verification

After regenerating and building, verify the executable exists:

```bash
ls -la "Tidal/Builds/MacOSX/build/Debug/Tidal.app/Contents/MacOS/"
# Should show: Tidal (executable binary)

file "Tidal/Builds/MacOSX/build/Debug/Tidal.app/Contents/MacOS/Tidal"
# Should show: Mach-O 64-bit executable arm64
```

## Summary

- **Issue:** Missing `Main.cpp` and `MainComponent` files in `Tidal.jucer`
- **Symptom:** Empty sources build phase → no executable created
- **Fix:** Added files to `.jucer`, regenerate in Projucer
- **Status:** ✅ Fixed in `.jucer` file, awaiting Projucer regeneration
