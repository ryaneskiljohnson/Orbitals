# Applying Fabric Fixes to All Plugins

## Plugins to Update

### Mandelbrot Series (8 plugins)
1. ✅ Fabric - COMPLETE (reference implementation)
2. ⏳ Entanglement
3. ⏳ Hadron
4. ⏳ Mandelbrot
5. ⏳ Observer
6. ⏳ Planck
7. ⏳ Quarks
8. ⏳ Rabbithole
9. ⏳ Singularity

### Orbital Series (9 plugins)
1. ⏳ Apogee
2. ⏳ Eclipse
3. ⏳ Ion
4. ⏳ Kepler
5. ⏳ Lagrange
6. ⏳ Perihelion
7. ⏳ Retrograde
8. ⏳ Tidal
9. ⏳ Zenith

**Total**: 17 plugins to update

---

## Critical Fixes to Apply

### 1. Parameter Smoothing Response (All Plugins)

**File**: `PluginProcessor.cpp` - `processBlock()` method

**Change**:
```cpp
// OLD (laggy):
float param = smoothedParam.getNextValue();

// NEW (instant):
float param = smoothedParam.skip(numSamples);
```

**Applies to**: ALL smoothed parameters in ALL plugins

---

### 2. Bus Layout Support (Audio FX Plugins Only)

**File**: `PluginProcessor.cpp` - `isBusesLayoutSupported()` method

**Change**:
```cpp
bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
  #else
    // Accept mono or stereo input (for microphone/audio interface)
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // Output must be stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return true;
  #endif
}
```

**Applies to**: Only Mandelbrot plugins (audio FX)  
**Skip**: Orbital plugins (MIDI FX - already correct)

---

### 3. Microphone Permissions (All Standalone Plugins)

**File**: `.jucer` file

**Add**:
```xml
<XCODE_MAC targetFolder="Builds/MacOSX" microphonePermissionNeeded="1" microphonePermissionsText="[Plugin] needs microphone access to process audio input.">
```

**Or** manually add to `Info.plist` after build:
```bash
plutil -insert NSMicrophoneUsageDescription -string "[Plugin] needs microphone access..." Info.plist
```

**Applies to**: ALL plugins with standalone builds

---

### 4. Component Class Names

**Mandelbrot Plugins** (`app.js`):
```javascript
// Use:
new MandelbrotKnob(...)
new MandelbrotSlider(...)
```

**Orbital Plugins** (`app.js`):
```javascript
// Use:
new OrbitalsKnob(...)
new OrbitalsSlider(...)
```

**HTML References**:
- Mandelbrot: `../../_Shared/UI/mandelbrot-*.js`
- Orbital: `../../_Shared/UI/orbitals-*.js`

---

### 5. Resource Inlining Paths

**Mandelbrot Plugins** (`PluginEditor.cpp`):
```cpp
auto projectRoot = juce::File("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");
auto sharedDir = projectRoot.getChildFile("_Shared").getChildFile("UI");

// Files to inline:
sharedDir.getChildFile("mandelbrot-components.js")
sharedDir.getChildFile("mandelbrot-animations.js")
sharedDir.getChildFile("mandelbrot-particles.js")
sharedDir.getChildFile("mandelbrot-design-system.css")
```

**Orbital Plugins** (`PluginEditor.cpp`):
```cpp
auto projectRoot = juce::File("/Users/rjmacbookpro/Development/Orbitals");
auto sharedDir = projectRoot.getChildFile("_Shared").getChildFile("UI");

// Files to inline:
sharedDir.getChildFile("orbitals-components.js")
sharedDir.getChildFile("orbitals-animations.js")
sharedDir.getChildFile("orbitals-particles.js")
sharedDir.getChildFile("orbitals-design-system.css")
```

---

## Automation Script Structure

### Python Script: `apply_fabric_fixes.py`

```python
#!/usr/bin/env python3
"""
Apply Fabric fixes to all Mandelbrot and Orbital plugins
"""

import os
import re
from pathlib import Path

# Plugin lists
MANDELBROT_PLUGINS = [
    'Entanglement', 'Hadron', 'Mandelbrot', 'Observer',
    'Planck', 'Quarks', 'Rabbithole', 'Singularity'
]

ORBITAL_PLUGINS = [
    'Apogee', 'Eclipse', 'Ion', 'Kepler', 'Lagrange',
    'Perihelion', 'Retrograde', 'Tidal', 'Zenith'
]

def fix_parameter_smoothing(plugin_path):
    """Replace getNextValue() with skip(numSamples) for all smoothed parameters"""
    processor_file = plugin_path / 'Source' / 'PluginProcessor.cpp'
    
    content = processor_file.read_text()
    
    # Replace smoothed parameter access
    pattern = r'smoothed(\w+)\.getNextValue\(\)'
    replacement = r'smoothed\1.skip(numSamples)'
    content = re.sub(pattern, replacement, content)
    
    processor_file.write_text(content)
    print(f"  ✅ Fixed parameter smoothing in {plugin_path.name}")

def fix_bus_layout_support(plugin_path):
    """Fix isBusesLayoutSupported to accept mono + stereo input"""
    processor_file = plugin_path / 'Source' / 'PluginProcessor.cpp'
    
    # ... implementation
    
def fix_microphone_permissions(plugin_path):
    """Add microphone permissions to .jucer file"""
    jucer_file = plugin_path / f'{plugin_path.name}.jucer'
    
    # ... implementation
    
def apply_all_fixes(plugins, base_path, is_mandelbrot=True):
    """Apply all fixes to a list of plugins"""
    for plugin_name in plugins:
        plugin_path = base_path / plugin_name
        print(f"\n🔧 Processing {plugin_name}...")
        
        fix_parameter_smoothing(plugin_path)
        if is_mandelbrot:
            fix_bus_layout_support(plugin_path)
        fix_microphone_permissions(plugin_path)
        
        print(f"  ✅ {plugin_name} complete")

if __name__ == '__main__':
    mandelbrot_path = Path('/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet')
    orbital_path = Path('/Users/rjmacbookpro/Development/Orbitals')
    
    print("=" * 70)
    print("APPLYING FABRIC FIXES TO ALL PLUGINS")
    print("=" * 70)
    
    print("\n📦 MANDELBROT SERIES")
    apply_all_fixes(MANDELBROT_PLUGINS, mandelbrot_path, is_mandelbrot=True)
    
    print("\n📦 ORBITAL SERIES")  
    apply_all_fixes(ORBITAL_PLUGINS, orbital_path, is_mandelbrot=False)
    
    print("\n" + "=" * 70)
    print("✅ ALL FIXES APPLIED")
    print("=" * 70)
```

---

## Manual Application Process

### For Each Plugin:

#### Step 1: Open Files
```bash
# Open in editor
code [Plugin]/Source/PluginProcessor.cpp
code [Plugin]/Source/PluginEditor.cpp
code [Plugin]/UI/app.js
```

#### Step 2: Apply Fixes
1. Search for `getNextValue()` → Replace with `skip(numSamples)`
2. Check `isBusesLayoutSupported()` - ensure accepts mono + stereo
3. Verify component classes match (Mandelbrot* or Orbitals*)
4. Check resource inlining paths are correct

#### Step 3: Build & Test
```bash
cd [Plugin]/Builds/MacOSX
xcodebuild -project [Plugin].xcodeproj -scheme "[Plugin] - All" -configuration Debug build
```

#### Step 4: Verify
- [ ] Build succeeds
- [ ] UI displays correctly
- [ ] All controls visible
- [ ] Parameters affect audio
- [ ] Wet/dry responds instantly (if audio FX)

---

## Priority Order

### High Priority (Audio FX with UI)
1. Hadron
2. Entanglement  
3. Mandelbrot
4. Singularity

### Medium Priority (MIDI FX with UI)
5. Eclipse
6. Ion
7. Kepler
8. Lagrange

### Low Priority (Already Working)
9. Apogee
10. Perihelion
11. Retrograde
12. Others

---

## Verification Checklist

For each plugin, verify:

- [ ] Builds without errors (VST3, AU, Standalone)
- [ ] UI displays correctly (background, controls, meters)
- [ ] JavaScript bridge working (parameter changes reach C++)
- [ ] Parameters respond instantly (no lag)
- [ ] Wet/dry control works (if audio FX)
- [ ] Bypass works
- [ ] Settings button works (standalone only)
- [ ] Microphone input works (if audio FX standalone)
- [ ] Authentication works (correct product ID)

---

## Estimated Time

- Per plugin: 15-30 minutes
- Total (17 plugins): 4-8 hours
- With automation script: 2-3 hours

---

## Next Steps

1. **Review this document** - Understand all fixes
2. **Create automation script** - Or apply manually
3. **Test Fabric one more time** - Ensure it's the reference
4. **Apply to next plugin** - Start with Hadron
5. **Build and verify** - Test thoroughly
6. **Repeat** - Continue through all plugins
7. **Final verification** - Test all 18 plugins

---

**This implementation guide is the definitive reference for all NNAudio plugins going forward.**
