#!/usr/bin/env python3
import os
from pathlib import Path

base = Path("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet")

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
    old_path = base / plugin_dir / old_name
    new_path = base / plugin_dir / new_name
    
    if old_path.exists():
        old_path.rename(new_path)
        print(f"✓ {plugin_dir}/{old_name} → {new_name}")
    else:
        print(f"✗ {plugin_dir}/{old_name} not found")

print("\n✅ Done!")
