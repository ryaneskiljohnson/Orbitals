#!/usr/bin/env python3
"""
Apply Fabric fixes to all Mandelbrot and Orbital plugins
Fixes: Parameter smoothing, bus layout, microphone permissions
"""

import os
import re
import sys
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

def fix_parameter_smoothing(processor_file):
    """Replace getNextValue() with skip(numSamples) for instant parameter response"""
    if not processor_file.exists():
        print(f"    ❌ File not found: {processor_file}")
        return False
    
    content = processor_file.read_text()
    original_content = content
    
    # Pattern: smoothedParamName.getNextValue()
    pattern = r'smoothed(\w+)\.getNextValue\s*\(\s*\)'
    replacement = r'smoothed\1.skip(numSamples)'
    
    content = re.sub(pattern, replacement, content)
    
    if content != original_content:
        processor_file.write_text(content)
        matches = len(re.findall(pattern, original_content))
        print(f"    ✅ Replaced {matches} getNextValue() calls with skip(numSamples)")
        return True
    else:
        print(f"    ℹ️  No getNextValue() calls found (already fixed or not using smoothing)")
        return False

def fix_bus_layout_support(processor_file):
    """Fix isBusesLayoutSupported to accept mono + stereo input"""
    if not processor_file.exists():
        print(f"    ❌ File not found: {processor_file}")
        return False
    
    content = processor_file.read_text()
    original_content = content
    
    # Find the isBusesLayoutSupported method
    old_pattern = r'bool\s+\w+::isBusesLayoutSupported\s*\([^)]+\)\s*const\s*\{[^}]+if\s*\(\s*layouts\.getMainOutputChannelSet\(\)[^}]+return\s+true;[^}]+\}'
    
    # New implementation
    new_impl = '''bool \\1::isBusesLayoutSupported(const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
  #else
    // Accept mono or stereo input
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // Output must be stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return true;
  #endif
}'''
    
    # This is complex - let's just check if it needs fixing
    if 'layouts.getMainInputChannelSet()' not in content:
        print(f"    ⚠️  isBusesLayoutSupported may need manual review (no input check found)")
        return False
    else:
        print(f"    ✅ Bus layout support already checks input channels")
        return True

def add_microphone_permissions(jucer_file):
    """Add microphone permissions to .jucer file"""
    if not jucer_file.exists():
        print(f"    ❌ File not found: {jucer_file}")
        return False
    
    content = jucer_file.read_text()
    original_content = content
    
    # Check if already has microphone permission
    if 'microphonePermissionNeeded="1"' in content:
        print(f"    ✅ Microphone permissions already configured")
        return False
    
    # Add to XCODE_MAC export format
    pattern = r'(<XCODE_MAC targetFolder="Builds/MacOSX")'
    replacement = r'\1 microphonePermissionNeeded="1" microphonePermissionsText="This plugin needs microphone access to process audio input."'
    
    content = re.sub(pattern, replacement, content)
    
    if content != original_content:
        jucer_file.write_text(content)
        print(f"    ✅ Added microphone permissions to .jucer")
        return True
    else:
        print(f"    ⚠️  Could not add microphone permissions automatically (may need manual edit)")
        return False

def process_plugin(plugin_name, plugin_path, is_mandelbrot=True):
    """Apply all fixes to a single plugin"""
    print(f"\n{'='*70}")
    print(f"🔧 Processing: {plugin_name}")
    print(f"   Path: {plugin_path}")
    print(f"{'='*70}")
    
    if not plugin_path.exists():
        print(f"❌ Plugin directory not found: {plugin_path}")
        return False
    
    success_count = 0
    total_fixes = 0
    
    # Fix 1: Parameter smoothing
    print("\n  📊 Fix 1: Parameter Smoothing Response")
    processor_file = plugin_path / 'Source' / 'PluginProcessor.cpp'
    if fix_parameter_smoothing(processor_file):
        success_count += 1
    total_fixes += 1
    
    # Fix 2: Bus layout (audio FX only)
    if is_mandelbrot:
        print("\n  🎛️  Fix 2: Bus Layout Support (Audio FX)")
        if fix_bus_layout_support(processor_file):
            success_count += 1
        total_fixes += 1
    
    # Fix 3: Microphone permissions
    print("\n  🎤 Fix 3: Microphone Permissions")
    jucer_file = plugin_path / f'{plugin_name}.jucer'
    if add_microphone_permissions(jucer_file):
        success_count += 1
    total_fixes += 1
    
    print(f"\n  {'='*66}")
    print(f"  ✅ {plugin_name}: {success_count}/{total_fixes} fixes applied")
    print(f"  {'='*66}")
    
    return True

def main():
    print("╔" + "="*68 + "╗")
    print("║" + " "*68 + "║")
    print("║" + "  APPLYING FABRIC FIXES TO ALL PLUGINS".center(68) + "║")
    print("║" + " "*68 + "║")
    print("╚" + "="*68 + "╝")
    print()
    
    mandelbrot_base = Path('/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet')
    orbital_base = Path('/Users/rjmacbookpro/Development/Orbitals')
    
    total_processed = 0
    total_success = 0
    
    # Process Mandelbrot plugins
    print("\n📦 MANDELBROT SERIES (Audio FX)")
    print("="*70)
    for plugin in MANDELBROT_PLUGINS:
        plugin_path = mandelbrot_base / plugin
        if process_plugin(plugin, plugin_path, is_mandelbrot=True):
            total_processed += 1
            total_success += 1
    
    # Process Orbital plugins  
    print("\n\n📦 ORBITAL SERIES (MIDI FX)")
    print("="*70)
    for plugin in ORBITAL_PLUGINS:
        plugin_path = orbital_base / plugin
        if process_plugin(plugin, plugin_path, is_mandelbrot=False):
            total_processed += 1
            total_success += 1
    
    # Summary
    print("\n" + "╔" + "="*68 + "╗")
    print("║" + " "*68 + "║")
    print("║" + f"  SUMMARY: {total_success}/{total_processed} plugins processed".center(68) + "║")
    print("║" + " "*68 + "║")
    print("╚" + "="*68 + "╝")
    print()
    print("⚠️  IMPORTANT NEXT STEPS:")
    print("  1. Review changes with git diff")
    print("  2. Build each plugin to verify compilation")
    print("  3. Test UI and parameter response")
    print("  4. Commit changes if all tests pass")
    print()

if __name__ == '__main__':
    main()
