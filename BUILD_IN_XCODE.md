# 🛠️ Building Standalone in Xcode

## ⚠️ Important: Regenerate Project First

The standalone app bundle exists but is missing the executable because the Xcode project needs to be regenerated in Projucer.

## Steps to Build Standalone

### Option 1: Regenerate in Projucer (Recommended)

1. **Open Projucer**:
   ```bash
   # If you have Projucer installed, open it and load:
   open /Users/rjmacbookpro/Development/Orbitals/01_Perihelion/Perihelion.jucer
   ```

2. **In Projucer**:
   - Verify "Perihelion (Standalone)" appears in the Export Formats list
   - Click **"Save and Open in IDE"** (or File → Save Project)
   - This regenerates the Xcode project with the standalone wrapper code

3. **In Xcode** (should open automatically):
   - Select scheme: **"Perihelion - Standalone Plugin"**
   - Select configuration: **Debug** or **Release**
   - Press **⌘B** to build, or **⌘R** to run

### Option 2: Build from Command Line (After Regeneration)

```bash
cd /Users/rjmacbookpro/Development/Orbitals/01_Perihelion

# Build standalone
xcodebuild -project Builds/MacOSX/Perihelion.xcodeproj \
           -scheme "Perihelion - Standalone Plugin" \
           -configuration Debug \
           build

# Or Release
xcodebuild -project Builds/MacOSX/Perihelion.xcodeproj \
           -scheme "Perihelion - Standalone Plugin" \
           -configuration Release \
           build
```

### Option 3: Build All Plugins

```bash
cd /Users/rjmacbookpro/Development/Orbitals

for plugin_dir in 0*_*/; do
    plugin=$(basename "$plugin_dir" | sed 's/^[0-9]*_//')
    echo "Building $plugin standalone..."
    cd "$plugin_dir"
    xcodebuild -project "Builds/MacOSX/${plugin}.xcodeproj" \
               -scheme "${plugin} - Standalone Plugin" \
               -configuration Debug \
               build
    cd ..
done
```

## Verify Build Success

After building, check for the executable:

```bash
ls -lh "01_Perihelion/Builds/MacOSX/build/Debug/Perihelion (Standalone).app/Contents/MacOS/"
```

You should see: `Perihelion (Standalone` (the executable file)

## Run the Standalone App

```bash
open "01_Perihelion/Builds/MacOSX/build/Debug/Perihelion (Standalone).app"
```

Or double-click the `.app` file in Finder.

---

**Current Status**: Xcode project is open, but you need to regenerate in Projucer first to create the standalone wrapper code.
