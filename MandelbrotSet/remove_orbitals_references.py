#!/usr/bin/env python3
"""
Remove all Orbitals references from MandelbrotSet plugins
Replace with Mandelbrot equivalents
"""
import os
import re
from pathlib import Path

base_dir = Path(__file__).parent

# Mapping: (plugin_dir, old_class_prefix, new_class_prefix, old_name, new_name, old_bg, new_bg)
plugin_mappings = [
    ("Singularity", "Tidal", "Singularity", "Tidal", "Singularity", "tidal-background.png", "singularity.png"),
    ("Fabric", "Apogee", "Fabric", "Apogee", "Fabric", "apogee-background.png", "fabric.png"),
    ("Entanglement", "Lagrange", "Entanglement", "Lagrange", "Entanglement", "lagrange-background.png", "entanglement.png"),
    ("Planck", "Eclipse", "Planck", "Eclipse", "Planck", "eclipse-background.png", "planck.png"),
    ("Observer", "Ion", "Observer", "Ion", "Observer", "ion-background.png", "observer.png"),
    ("Hadron", "Perihelion", "Hadron", "Perihelion", "Hadron", "perihelion-background.png", "hadron.png"),
    ("Mandelbrot", "Retrograde", "Mandelbrot", "Retrograde", "Mandelbrot", "retrograde-background.png", "mandelbrot.png"),
    ("Quarks", "Kepler", "Quarks", "Kepler", "Quarks", "kepler-background.png", "quarks.png"),
    ("Rabbithole", "Zenith", "Rabbithole", "Zenith", "Rabbithole", "zenith-background.png", "rabbithole.png"),
]

def replace_in_file(file_path, replacements):
    """Replace all occurrences in a file"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        for old, new in replacements:
            content = content.replace(old, new)
        
        if content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
    except Exception as e:
        print(f"  ⚠️  Error processing {file_path}: {e}")
    return False

# Process each plugin
for plugin_dir, old_class, new_class, old_name, new_name, old_bg, new_bg in plugin_mappings:
    plugin_path = base_dir / plugin_dir
    if not plugin_path.exists():
        print(f"⚠️  {plugin_dir} not found, skipping")
        continue
    
    print(f"\n📦 Processing {plugin_dir}...")
    
    # Find all C++ and header files
    cpp_files = list(plugin_path.rglob("*.cpp")) + list(plugin_path.rglob("*.h"))
    
    for cpp_file in cpp_files:
        # Skip if in Builds or JUCE directories
        if "Builds" in str(cpp_file) or "JUCE" in str(cpp_file) or "JuceLibraryCode" in str(cpp_file):
            continue
        
        replacements = [
            # Class names
            (f"{old_class}AudioProcessor", f"{new_class}AudioProcessor"),
            (f"{old_class}AudioProcessorEditor", f"{new_class}AudioProcessorEditor"),
            (f"{old_class}AudioProcessor::", f"{new_class}AudioProcessor::"),
            (f"{old_class}AudioProcessorEditor::", f"{new_class}AudioProcessorEditor::"),
            
            # File references
            ("orbitals-design-system.css", "mandelbrot-design-system.css"),
            ("orbitals-components.js", "mandelbrot-components.js"),
            ("orbitals-animations.js", "mandelbrot-animations.js"),
            ("orbitals-particles.js", "mandelbrot-particles.js"),
            
            # Background images
            (old_bg, new_bg),
            
            # Path references (keep Orbitals path but update file names)
            (f'getChildFile ("{old_name}")', f'getChildFile ("{new_name}")'),
            (f'getChildFile("{old_name}")', f'getChildFile("{new_name}")'),
            (f'/{old_name}/', f'/{new_name}/'),
            
            # String replacements in code
            (f'"{old_name}"', f'"{new_name}"'),
            (f"'{old_name}'", f"'{new_name}'"),
            
            # Comments
            (f"{old_name} -", f"{new_name} -"),
            (f"{old_name} ", f"{new_name} "),
        ]
        
        if replace_in_file(cpp_file, replacements):
            print(f"  ✓ Updated {cpp_file.relative_to(base_dir)}")
    
    # Update .jucer files
    jucer_files = list(plugin_path.glob("*.jucer"))
    for jucer_file in jucer_files:
        replacements = [
            (f'name="{old_name}"', f'name="{new_name}"'),
            (f'targetName="{old_name}"', f'targetName="{new_name}"'),
            (old_bg, new_bg),
            ("orbitals-design-system.css", "mandelbrot-design-system.css"),
            ("orbitals-components.js", "mandelbrot-components.js"),
            ("orbitals-animations.js", "mandelbrot-animations.js"),
            ("orbitals-particles.js", "mandelbrot-particles.js"),
        ]
        if replace_in_file(jucer_file, replacements):
            print(f"  ✓ Updated {jucer_file.name}")

# Update shared UI files if they exist
shared_ui = base_dir / "_Shared" / "UI"
if shared_ui.exists():
    print(f"\n📦 Processing shared UI files...")
    for js_file in shared_ui.glob("*.js"):
        if "mandelbrot" not in js_file.name.lower():
            # This should already be renamed, but check for any remaining references
            replacements = [
                ("Orbitals", "Mandelbrot"),
                ("orbitals", "mandelbrot"),
            ]
            if replace_in_file(js_file, replacements):
                print(f"  ✓ Updated {js_file.name}")

print("\n✅ All Orbitals references removed from MandelbrotSet!")
