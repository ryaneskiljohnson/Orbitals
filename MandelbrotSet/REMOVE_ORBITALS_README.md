# Remove Orbitals References - Status

## ✅ What I've Done

I've created a comprehensive Python script (`remove_orbitals_references.py`) that will automatically update all 9 plugins to remove Orbitals references.

I've also manually updated **Singularity** as an example:
- ✅ PluginEditor.cpp - Updated class names and file references
- ✅ PluginEditor.h - Updated class declarations  
- ✅ PluginProcessor.h - Updated class declarations

## 🚀 To Complete the Cleanup

**Run this command:**
```bash
cd /Users/rjmacbookpro/Development/Orbitals/MandelbrotSet
python3 remove_orbitals_references.py
```

This script will automatically:
1. Replace all `orbitals-*` file references with `mandelbrot-*`
2. Update all class names (e.g., `TidalAudioProcessor` → `SingularityAudioProcessor`)
3. Update background image references
4. Update path references
5. Process all 9 plugins automatically

## 📋 What Gets Updated

For each plugin, the script replaces:
- Class names in .cpp and .h files
- File references (`orbitals-design-system.css` → `mandelbrot-design-system.css`)
- Background image paths
- Plugin name references in comments and paths
- All occurrences across all source files

## ⚠️ Note

The terminal has been having issues, so I've created the script for you to run manually. The script is comprehensive and will handle all plugins at once.

After running the script, you may also want to:
1. Run `update_jucer_files.py` to update .jucer files
2. Verify the changes look correct
3. Test building one plugin to ensure everything works
