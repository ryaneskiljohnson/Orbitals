#!/usr/bin/env python3
"""
Rename .jucer files in MandelbrotSet to match new plugin names
"""
import os
from pathlib import Path

base_dir = Path(__file__).parent

renames = [
    ("Singularity", "Tidal.jucer", "Singularity.jucer"),
    ("Fabric", "Apogee.jucer", "Fabric.jucer"),
    ("Entanglement", "Lagrange.jucer", "Entanglement.jucer"),
    ("Planck", "Eclipse.jucer", "Planck.jucer"),
    ("Observer", "Ion.jucer", "Observer.jucer"),
    ("Hadron", "Perihelion.jucer", "Hadron.jucer"),
    ("Mandelbrot", "Retrograde.jucer", "Mandelbrot.jucer"),
    ("Quarks", "Kepler.jucer", "Quarks.jucer"),
    ("Rabbithole", "Zenith.jucer", "Rabbithole.jucer"),
]

for plugin_dir, old_name, new_name in renames:
    old_path = base_dir / plugin_dir / old_name
    new_path = base_dir / plugin_dir / new_name
    
    if old_path.exists():
        old_path.rename(new_path)
        print(f"✓ Renamed {plugin_dir}/{old_name} → {plugin_dir}/{new_name}")
    else:
        print(f"✗ Not found: {plugin_dir}/{old_name}")

print("\nAll .jucer files renamed!")
