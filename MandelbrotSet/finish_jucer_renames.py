#!/usr/bin/env python3
"""Finish renaming remaining .jucer files"""
import os
from pathlib import Path

base = Path("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet")

# Remaining plugins to rename
renames = [
    ("Planck", "Eclipse.jucer", "Planck.jucer", "Ecli1", "Plck1", "Ecli", "Plck", "eclipse-background.png", "planck.png", "Eclipse", "Planck"),
    ("Observer", "Ion.jucer", "Observer.jucer", "Ion1", "Obse1", "Ionx", "Obse", "ion-background.png", "observer.png", "Ion", "Observer"),
    ("Hadron", "Perihelion.jucer", "Hadron.jucer", "Peri1", "Hadr1", "Peri", "Hadr", "perihelion-background.png", "hadron.png", "Perihelion", "Hadron"),
    ("Mandelbrot", "Retrograde.jucer", "Mandelbrot.jucer", "Retr1", "Mand1", "Retr", "Mand", "retrograde-background.png", "mandelbrot.png", "Retrograde", "Mandelbrot"),
    ("Quarks", "Kepler.jucer", "Quarks.jucer", "Kepl1", "Quar1", "Kepl", "Quar", "kepler-background.png", "quarks.png", "Kepler", "Quarks"),
    ("Rabbithole", "Zenith.jucer", "Rabbithole.jucer", "Lagr1", "Rabb1", "Lagr", "Rabb", "zenith-background.png", "rabbithole.png", "Zenith", "Rabbithole"),
]

for plugin_dir, old_name, new_name, old_id, new_id, old_code, new_code, old_bg, new_bg, old_display, new_display in renames:
    old_path = base / plugin_dir / old_name
    new_path = base / plugin_dir / new_name
    
    if old_path.exists():
        content = old_path.read_text(encoding='utf-8', errors='ignore')
        
        # Replace all occurrences
        content = content.replace(f'id="{old_id}"', f'id="{new_id}"')
        content = content.replace(f'name="{old_display}"', f'name="{new_display}"')
        content = content.replace(f'pluginCode="{old_code}"', f'pluginCode="{new_code}"')
        content = content.replace(f'targetName="{old_display}"', f'targetName="{new_display}"')
        content = content.replace(f'aaxIdentifier="com.nnaudio.{old_display.lower()}"', f'aaxIdentifier="com.nnaudio.{new_display.lower()}"')
        content = content.replace(f'bundleIdentifier="com.nnaudio.{old_display.lower()}"', f'bundleIdentifier="com.nnaudio.{new_display.lower()}"')
        content = content.replace(old_bg, new_bg)
        content = content.replace(f'<MAINGROUP id="{old_id}" name="{old_display}">', f'<MAINGROUP id="{new_id}" name="{new_display}">')
        content = content.replace('pluginIsMidiEffectPlugin', 'pluginWantsMidiInput')
        content = content.replace(f'id="{old_display.lower()}_bg"', f'id="{new_display.lower()}_bg"')
        
        new_path.write_text(content, encoding='utf-8')
        old_path.unlink()
        print(f"✓ {plugin_dir}/{old_name} → {new_name}")
    else:
        print(f"✗ {plugin_dir}/{old_name} not found")

print("\n✅ Done!")
