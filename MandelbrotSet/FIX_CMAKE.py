#!/usr/bin/env python3
"""
Fix all CMakeLists.txt files for Mandelbrot Set plugins
"""
import re
from pathlib import Path

base_dir = Path("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet")

# Plugin mappings: (old_name, new_name, old_code, new_code, old_bundle, new_bundle, old_bg, new_bg)
plugin_mappings = [
    ("Tidal", "Singularity", "Tidl", "Sing", "com.nnaudio.tidal", "com.nnaudio.singularity", "tidal-background.png", "singularity.png"),
    ("Apogee", "Fabric", "Apog", "Fabr", "com.nnaudio.apogee", "com.nnaudio.fabric", "apogee-background.png", "fabric.png"),
    ("Lagrange", "Entanglement", "Lagr", "Enta", "com.nnaudio.lagrange", "com.nnaudio.entanglement", "lagrange-background.png", "entanglement.png"),
    ("Eclipse", "Planck", "Ecli", "Plck", "com.nnaudio.eclipse", "com.nnaudio.planck", "eclipse-background.png", "planck.png"),
    ("Ion", "Observer", "Ionx", "Obse", "com.nnaudio.ion", "com.nnaudio.observer", "ion-background.png", "observer.png"),
    ("Perihelion", "Hadron", "Peri", "Hadr", "com.nnaudio.perihelion", "com.nnaudio.hadron", "perihelion-background.png", "hadron.png"),
    ("Retrograde", "Mandelbrot", "Retr", "Mand", "com.nnaudio.retrograde", "com.nnaudio.mandelbrot", "retrograde-background.png", "mandelbrot.png"),
    ("Kepler", "Quarks", "Kepl", "Quar", "com.nnaudio.kepler", "com.nnaudio.quarks", "kepler-background.png", "quarks.png"),
    ("Zenith", "Rabbithole", "Zeni", "Rabb", "com.nnaudio.zenith", "com.nnaudio.rabbithole", "zenith-background.png", "rabbithole.png"),
]

for old_name, new_name, old_code, new_code, old_bundle, new_bundle, old_bg, new_bg in plugin_mappings:
    cmake_file = base_dir / new_name / "CMakeLists.txt"
    
    if not cmake_file.exists():
        print(f"⚠️  {cmake_file} not found, skipping...")
        continue
    
    content = cmake_file.read_text()
    
    # Replace project name
    content = re.sub(rf'project\({old_name}', f'project({new_name}', content)
    
    # Replace plugin name in juce_add_plugin
    content = re.sub(rf'juce_add_plugin\({old_name}', f'juce_add_plugin({new_name}', content)
    content = re.sub(rf'PRODUCT_NAME "{old_name}"', f'PRODUCT_NAME "{new_name}"', content)
    
    # Replace plugin code
    content = re.sub(rf'PLUGIN_CODE "{old_code}"', f'PLUGIN_CODE "{new_code}"', content)
    
    # Replace bundle ID
    content = re.sub(rf'BUNDLE_ID "{old_bundle}"', f'BUNDLE_ID "{new_bundle}"', content)
    
    # Change from MIDI effect to Audio effect
    content = re.sub(r'IS_MIDI_EFFECT TRUE', 'IS_MIDI_EFFECT FALSE', content)
    content = re.sub(r'NEEDS_MIDI_INPUT TRUE', 'NEEDS_MIDI_INPUT FALSE', content)
    content = re.sub(r'NEEDS_MIDI_OUTPUT TRUE', 'NEEDS_MIDI_OUTPUT FALSE', content)
    
    # Replace resource target name
    content = re.sub(rf'{old_name}Resources', f'{new_name}Resources', content)
    content = re.sub(rf'target_link_libraries\({old_name} PRIVATE', f'target_link_libraries({new_name} PRIVATE', content)
    content = re.sub(rf'target_sources\({old_name} PRIVATE', f'target_sources({new_name} PRIVATE', content)
    content = re.sub(rf'target_compile_definitions\({old_name} PUBLIC', f'target_compile_definitions({new_name} PUBLIC', content)
    
    # Replace orbitals-* with mandelbrot-*
    content = content.replace('orbitals-design-system.css', 'mandelbrot-design-system.css')
    content = content.replace('orbitals-animations.js', 'mandelbrot-animations.js')
    content = content.replace('orbitals-particles.js', 'mandelbrot-particles.js')
    content = content.replace('orbitals-components.js', 'mandelbrot-components.js')
    
    # Replace background image
    content = content.replace(old_bg, new_bg)
    
    cmake_file.write_text(content)
    print(f"✅ Fixed {cmake_file.name}")

print("\n🎉 All CMakeLists.txt files updated!")
