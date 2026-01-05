#!/usr/bin/env python3
"""
Update all .jucer files in MandelbrotSet to use correct plugin names
"""
import os
import re
from pathlib import Path

base_dir = Path(__file__).parent

# Mapping: (plugin_dir, old_name, new_name, old_id_prefix, new_id_prefix, old_code, new_code, old_bg, new_bg)
updates = [
    ("Singularity", "Tidal", "Singularity", "Tida", "Sing", "Tidl", "Sing", "tidal-background.png", "singularity.png"),
    ("Fabric", "Apogee", "Fabric", "Apog", "Fabr", "Apog", "Fabr", "apogee-background.png", "fabric.png"),
    ("Entanglement", "Lagrange", "Entanglement", "Lagn", "Enta", "Lagn", "Enta", "lagrange-background.png", "entanglement.png"),
    ("Planck", "Eclipse", "Planck", "Ecli", "Plck", "Ecli", "Plck", "eclipse-background.png", "planck.png"),
    ("Observer", "Ion", "Observer", "Ion1", "Obse", "Ion1", "Obse", "ion-background.png", "observer.png"),
    ("Hadron", "Perihelion", "Hadron", "Peri", "Hadr", "Peri", "Hadr", "perihelion-background.png", "hadron.png"),
    ("Mandelbrot", "Retrograde", "Mandelbrot", "Retr", "Mand", "Retr", "Mand", "retrograde-background.png", "mandelbrot.png"),
    ("Quarks", "Kepler", "Quarks", "Kepl", "Quar", "Kepl", "Quar", "kepler-background.png", "quarks.png"),
    ("Rabbithole", "Zenith", "Rabbithole", "Zeni", "Rabb", "Zeni", "Rabb", "zenith-background.png", "rabbithole.png"),
]

for plugin_dir, old_name, new_name, old_id, new_id, old_code, new_code, old_bg, new_bg in updates:
    jucer_file = base_dir / plugin_dir / f"{old_name}.jucer"
    new_jucer_file = base_dir / plugin_dir / f"{new_name}.jucer"
    
    if not jucer_file.exists():
        print(f"⚠️  {plugin_dir}/{old_name}.jucer not found, skipping")
        continue
    
    # Read file
    content = jucer_file.read_text(encoding='utf-8')
    
    # Replace all occurrences
    content = content.replace(f'name="{old_name}"', f'name="{new_name}"')
    content = content.replace(f'id="{old_id}1"', f'id="{new_id}1"')
    content = content.replace(f'pluginCode="{old_code}"', f'pluginCode="{new_code}"')
    content = content.replace(f'aaxIdentifier="com.nnaudio.{old_name.lower()}"', f'aaxIdentifier="com.nnaudio.{new_name.lower()}"')
    content = content.replace(f'bundleIdentifier="com.nnaudio.{old_name.lower()}"', f'bundleIdentifier="com.nnaudio.{new_name.lower()}"')
    content = content.replace(f'targetName="{old_name}"', f'targetName="{new_name}"')
    content = content.replace(f'name="{old_name}"', f'name="{new_name}"')
    content = content.replace(old_bg, new_bg)
    
    # Change from MIDI effect to Audio effect
    content = content.replace('pluginIsMidiEffectPlugin', 'pluginWantsMidiInput')
    
    # Write new file
    new_jucer_file.write_text(content, encoding='utf-8')
    print(f"✓ Updated {plugin_dir}/{new_name}.jucer")
    
    # Optionally remove old file (uncomment if desired)
    # jucer_file.unlink()

print("\n✅ All .jucer files updated!")
