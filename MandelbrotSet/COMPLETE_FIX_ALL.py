#!/usr/bin/env python3
"""
COMPLETE FIX - Finish all .jucer files and update all C++ files
"""
import os
import re
from pathlib import Path

base_dir = Path("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet")

# Remaining .jucer files to fix
jucer_fixes = [
    ("Hadron", "Perihelion.jucer", "Hadron.jucer", "Peri1", "Hadr1", "Peri", "Hadr", "perihelion-background.png", "hadron.png", "Perihelion", "Hadron", "com.nnaudio.perihelion", "com.nnaudio.hadron"),
    ("Mandelbrot", "Retrograde.jucer", "Mandelbrot.jucer", "Retr1", "Mand1", "Retr", "Mand", "retrograde-background.png", "mandelbrot.png", "Retrograde", "Mandelbrot", "com.nnaudio.retrograde", "com.nnaudio.mandelbrot"),
    ("Quarks", "Kepler.jucer", "Quarks.jucer", "Kepl1", "Quar1", "Kepl", "Quar", "kepler-background.png", "quarks.png", "Kepler", "Quarks", "com.nnaudio.kepler", "com.nnaudio.quarks"),
    ("Rabbithole", "Zenith.jucer", "Rabbithole.jucer", "Lagr1", "Rabb1", "Lagr", "Rabb", "zenith-background.png", "rabbithole.png", "Zenith", "Rabbithole", "com.nnaudio.lagrange", "com.nnaudio.rabbithole"),
]

# C++ class mappings
cpp_mappings = [
    ("Singularity", "Tidal", "Singularity"),
    ("Fabric", "Apogee", "Fabric"),
    ("Entanglement", "Lagrange", "Entanglement"),
    ("Planck", "Eclipse", "Planck"),
    ("Observer", "Ion", "Observer"),
    ("Hadron", "Perihelion", "Hadron"),
    ("Mandelbrot", "Retrograde", "Mandelbrot"),
    ("Quarks", "Kepler", "Quarks"),
    ("Rabbithole", "Zenith", "Rabbithole"),
]

print("🔧 COMPLETE FIX - Mandelbrot Set\n")

# 1. Fix remaining .jucer files
print("📦 Fixing .jucer files...")
for plugin_dir, old_name, new_name, old_id, new_id, old_code, new_code, old_bg, new_bg, old_display, new_display, old_bundle, new_bundle in jucer_fixes:
    old_path = base_dir / plugin_dir / old_name
    new_path = base_dir / plugin_dir / new_name
    
    if old_path.exists():
        content = old_path.read_text(encoding='utf-8', errors='ignore')
        
        # Replace all
        content = content.replace(f'id="{old_id}"', f'id="{new_id}"')
        content = content.replace(f'name="{old_display}"', f'name="{new_display}"')
        content = content.replace(f'pluginCode="{old_code}"', f'pluginCode="{new_code}"')
        content = content.replace(f'targetName="{old_display}"', f'targetName="{new_display}"')
        content = content.replace(f'aaxIdentifier="{old_bundle}"', f'aaxIdentifier="{new_bundle}"')
        content = content.replace(f'bundleIdentifier="{old_bundle}"', f'bundleIdentifier="{new_bundle}"')
        content = content.replace(old_bg, new_bg)
        content = content.replace(f'<MAINGROUP id="{old_id}" name="{old_display}">', f'<MAINGROUP id="{new_id}" name="{new_display}">')
        content = content.replace('pluginIsMidiEffectPlugin', 'pluginWantsMidiInput')
        content = content.replace(f'id="{old_display.lower()}_bg"', f'id="{new_display.lower()}_bg"')
        content = content.replace(f'id="{old_name.split(".")[0].lower()}_bg"', f'id="{new_display.lower()}_bg"')
        
        new_path.write_text(content, encoding='utf-8')
        old_path.unlink()
        print(f"  ✓ {plugin_dir}/{old_name} → {new_name}")

# 2. Update all C++ files
print("\n🔨 Updating C++ files...")
for plugin_dir, old_name, new_name in cpp_mappings:
    source_dir = base_dir / plugin_dir / "Source"
    if not source_dir.exists():
        continue
    
    for cpp_file in source_dir.glob("*.cpp"):
        content = cpp_file.read_text(encoding='utf-8', errors='ignore')
        original = content
        
        # Replace class names
        content = content.replace(f'{old_name}AudioProcessor', f'{new_name}AudioProcessor')
        content = content.replace(f'{old_name}AudioProcessorEditor', f'{new_name}AudioProcessorEditor')
        
        # Replace file references
        content = content.replace('orbitals-design-system.css', 'mandelbrot-design-system.css')
        content = content.replace('orbitals-components.js', 'mandelbrot-components.js')
        content = content.replace('orbitals-animations.js', 'mandelbrot-animations.js')
        content = content.replace('orbitals-particles.js', 'mandelbrot-particles.js')
        
        # Replace background images
        old_bg_map = {
            "Tidal": "tidal-background.png",
            "Apogee": "apogee-background.png",
            "Lagrange": "lagrange-background.png",
            "Eclipse": "eclipse-background.png",
            "Ion": "ion-background.png",
            "Perihelion": "perihelion-background.png",
            "Retrograde": "retrograde-background.png",
            "Kepler": "kepler-background.png",
            "Zenith": "zenith-background.png",
        }
        new_bg_map = {
            "Singularity": "singularity.png",
            "Fabric": "fabric.png",
            "Entanglement": "entanglement.png",
            "Planck": "planck.png",
            "Observer": "observer.png",
            "Hadron": "hadron.png",
            "Mandelbrot": "mandelbrot.png",
            "Quarks": "quarks.png",
            "Rabbithole": "rabbithole.png",
        }
        if old_name in old_bg_map and new_name in new_bg_map:
            content = content.replace(old_bg_map[old_name], new_bg_map[new_name])
        
        # Replace paths
        content = content.replace(f'getChildFile ("{old_name}")', f'getChildFile ("MandelbrotSet").getChildFile ("{new_name}")')
        content = content.replace(f'{old_name}UI_', f'{new_name}UI_')
        content = content.replace(f'{old_name} -', f'{new_name} -')
        content = content.replace('MIDI FX', 'Audio FX')
        
        if content != original:
            cpp_file.write_text(content, encoding='utf-8')
            print(f"  ✓ Updated {plugin_dir}/Source/{cpp_file.name}")
    
    for h_file in source_dir.glob("*.h"):
        content = h_file.read_text(encoding='utf-8', errors='ignore')
        original = content
        
        content = content.replace(f'{old_name}AudioProcessor', f'{new_name}AudioProcessor')
        content = content.replace(f'{old_name}AudioProcessorEditor', f'{new_name}AudioProcessorEditor')
        content = content.replace(f'{old_name} -', f'{new_name} -')
        content = content.replace('MIDI FX', 'Audio FX')
        
        if content != original:
            h_file.write_text(content, encoding='utf-8')
            print(f"  ✓ Updated {plugin_dir}/Source/{h_file.name}")

print("\n✅ All fixes complete!")
