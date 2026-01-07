# 🔄 How to Fix: Standalone App Missing Executable

## The Problem

The standalone app bundle is being created, but it doesn't contain an executable because **Projucer hasn't generated the standalone wrapper code**. The Xcode project was created before we added the standalone export format.

## The Solution: Regenerate in Projucer

**You MUST regenerate the project in Projucer** for the standalone to work. Projucer generates the wrapper code that creates the executable.

### Steps:

1. **Open Projucer**:
   - Launch JUCE Projucer application
   - File → Open → Navigate to `01_Perihelion/Perihelion.jucer`

2. **Verify Standalone Export**:
   - In Projucer, you should see "Perihelion (Standalone)" in the Export Formats section
   - Make sure it's enabled/checked

3. **Regenerate Project**:
   - Click **"Save and Open in IDE"** button (or File → Save Project)
   - This will regenerate the Xcode project with the standalone wrapper code

4. **What Projucer Generates**:
   - Creates `JuceLibraryCode/StandalonePlugin.cpp` (or similar)
   - Adds the standalone wrapper that wraps your plugin in a JUCEApplication
   - Sets up the main() entry point for the standalone app

5. **Build in Xcode**:
   - Xcode should open automatically
   - Select scheme: **"Perihelion - Standalone Plugin"**
   - Press ⌘B to build

## Alternative: Command Line Regeneration

If you have Projucer command line tool:

```bash
# Find Projucer (usually in JUCE/extras/Projucer/Builds/MacOSX/build/Release/)
/path/to/Projucer --resave 01_Perihelion/Perihelion.jucer
```

## Verify After Regeneration

After regenerating, you should see new files in `JuceLibraryCode/`:
- `StandalonePlugin.cpp` or similar wrapper file

Then rebuild:
```bash
cd 01_Perihelion
xcodebuild -project Builds/MacOSX/Perihelion.xcodeproj \
           -scheme "Perihelion - Standalone Plugin" \
           -configuration Debug \
           build
```

Check for executable:
```bash
ls -lh "Builds/MacOSX/build/Debug/Perihelion (Standalone).app/Contents/MacOS/"
```

You should see: `Perihelion (Standalone` (the executable)

---

**Important**: The .jucer files are correctly configured. You just need Projucer to regenerate the Xcode projects to create the wrapper code.
