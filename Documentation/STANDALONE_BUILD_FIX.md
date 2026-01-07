# 🔧 Fix: Standalone Build Missing Executable

## Problem
The standalone app bundle exists but doesn't contain an executable because Projucer hasn't regenerated the Xcode project with the standalone wrapper code.

## Solution

### Step 1: Open in Projucer
1. Open `01_Perihelion/Perihelion.jucer` (or any plugin's .jucer file) in **JUCE Projucer**
2. You should see the standalone export format in the export formats list

### Step 2: Regenerate Project
1. In Projucer, click **"Save and Open in IDE"** (or File → Save Project)
2. This will regenerate the Xcode project with the standalone wrapper code

### Step 3: Build Standalone
1. Open the generated Xcode project
2. Select the **"Perihelion (Standalone)"** scheme from the scheme dropdown
3. Build (⌘B) or Run (⌘R)

## What Projucer Generates

When you regenerate with standalone export enabled, Projucer automatically creates:

- **Standalone wrapper code** - Wraps your plugin processor/editor in a standalone application
- **Main entry point** - Creates the JUCEApplication wrapper
- **Audio device handling** - Sets up audio I/O for standalone mode
- **Xcode target** - Creates the standalone build target

## Quick Fix Command

If you want to regenerate all plugins at once:

```bash
cd /Users/rjmacbookpro/Development/Orbitals

# Open each .jucer in Projucer and click "Save and Open in IDE"
# Or use Projucer command line (if available):
# /path/to/Projucer --resave 01_Perihelion/Perihelion.jucer
```

## Verify It Worked

After regenerating, check:

```bash
ls -la "01_Perihelion/Builds/MacOSX/build/Debug/Perihelion (Standalone).app/Contents/MacOS/"
```

You should see an executable file (the standalone app binary).

---

**Note:** The standalone export format is already configured in all .jucer files. You just need to regenerate the projects in Projucer to create the wrapper code.
