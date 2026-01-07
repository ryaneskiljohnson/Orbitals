# Auth Background Image Fix

## Issue
Background images were not showing in the authentication screens for Mandelbrot plugins.

## Root Cause
The `loadAuthScreen()` method in `PluginEditor.cpp` was using an incorrect path:
- **Wrong**: `/Users/rjmacbookpro/Development/Orbitals/_Shared/Assets/backgrounds/`
- **Correct**: `/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/_Shared/Assets/backgrounds/`

The path was missing the `MandelbrotSet` directory.

## Fix Applied
Updated `projectRoot` path in `loadAuthScreen()` method for all 8 Mandelbrot plugins:
- ✅ Entanglement
- ✅ Hadron (already correct)
- ✅ Mandelbrot
- ✅ Observer
- ✅ Planck
- ✅ Quarks
- ✅ Rabbithole
- ✅ Singularity

## Files Modified
- `MandelbrotSet/[Plugin]/Source/PluginEditor.cpp` (8 files)

## Verification
All plugins now correctly reference:
```cpp
auto projectRoot = juce::File("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");
auto backgroundImage = projectRoot.getChildFile("_Shared/Assets/backgrounds/[plugin].png");
```

## Result
✅ Background images should now display correctly in all authentication screens.

---

*Fixed: January 6, 2026*
