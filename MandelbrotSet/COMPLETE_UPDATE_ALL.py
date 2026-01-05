#!/usr/bin/env python3
"""
Complete update script for all MandelbrotSet plugins
Updates .jucer files, C++ files, and all references
"""
import os
import re
from pathlib import Path

base_dir = Path(__file__).parent

# Complete mapping for all 9 plugins
plugins = [
    {
        "dir": "Singularity",
        "old_name": "Tidal",
        "new_name": "Singularity",
        "old_id": "Tida",
        "new_id": "Sing",
        "old_code": "Tidl",
        "new_code": "Sing",
        "old_bg": "tidal-background.png",
        "new_bg": "singularity.png",
        "product_id": "300001"
    },
    {
        "dir": "Fabric",
        "old_name": "Apogee",
        "new_name": "Fabric",
        "old_id": "Apog",
        "new_id": "Fabr",
        "old_code": "Apog",
        "new_code": "Fabr",
        "old_bg": "apogee-background.png",
        "new_bg": "fabric.png",
        "product_id": "300002"
    },
    {
        "dir": "Entanglement",
        "old_name": "Lagrange",
        "new_name": "Entanglement",
        "old_id": "Lagn",
        "new_id": "Enta",
        "old_code": "Lagn",
        "new_code": "Enta",
        "old_bg": "lagrange-background.png",
        "new_bg": "entanglement.png",
        "product_id": "300003"
    },
    {
        "dir": "Planck",
        "old_name": "Eclipse",
        "new_name": "Planck",
        "old_id": "Ecli",
        "new_id": "Plck",
        "old_code": "Ecli",
        "new_code": "Plck",
        "old_bg": "eclipse-background.png",
        "new_bg": "planck.png",
        "product_id": "300004"
    },
    {
        "dir": "Observer",
        "old_name": "Ion",
        "new_name": "Observer",
        "old_id": "Ion1",
        "new_id": "Obse",
        "old_code": "Ion1",
        "new_code": "Obse",
        "old_bg": "ion-background.png",
        "new_bg": "observer.png",
        "product_id": "300005"
    },
    {
        "dir": "Hadron",
        "old_name": "Perihelion",
        "new_name": "Hadron",
        "old_id": "Peri",
        "new_id": "Hadr",
        "old_code": "Peri",
        "new_code": "Hadr",
        "old_bg": "perihelion-background.png",
        "new_bg": "hadron.png",
        "product_id": "300006"
    },
    {
        "dir": "Mandelbrot",
        "old_name": "Retrograde",
        "new_name": "Mandelbrot",
        "old_id": "Retr",
        "new_id": "Mand",
        "old_code": "Retr",
        "new_code": "Mand",
        "old_bg": "retrograde-background.png",
        "new_bg": "mandelbrot.png",
        "product_id": "300007"
    },
    {
        "dir": "Quarks",
        "old_name": "Kepler",
        "new_name": "Quarks",
        "old_id": "Kepl",
        "new_id": "Quar",
        "old_code": "Kepl",
        "new_code": "Quar",
        "old_bg": "kepler-background.png",
        "new_bg": "quarks.png",
        "product_id": "300008"
    },
    {
        "dir": "Rabbithole",
        "old_name": "Zenith",
        "new_name": "Rabbithole",
        "old_id": "Zeni",
        "new_id": "Rabb",
        "old_code": "Zeni",
        "new_code": "Rabb",
        "old_bg": "zenith-background.png",
        "new_bg": "rabbithole.png",
        "product_id": "300009"
    },
]

def update_file(file_path, replacements):
    """Update file with replacements"""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        original = content
        for old, new in replacements:
            content = content.replace(old, new)
        
        if content != original:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
    except Exception as e:
        print(f"  ⚠️  Error: {e}")
    return False

print("🌀 MANDELBROT SET - Complete Update Script\n")

for plugin in plugins:
    plugin_path = base_dir / plugin["dir"]
    if not plugin_path.exists():
        print(f"⚠️  {plugin['dir']} not found")
        continue
    
    print(f"📦 Updating {plugin['dir']}...")
    
    # 1. Update .jucer file
    old_jucer = plugin_path / f"{plugin['old_name']}.jucer"
    new_jucer = plugin_path / f"{plugin['new_name']}.jucer"
    
    if old_jucer.exists():
        replacements = [
            (f'id="{plugin["old_id"]}1"', f'id="{plugin["new_id"]}1"'),
            (f'name="{plugin["old_name"]}"', f'name="{plugin["new_name"]}"'),
            (f'pluginCode="{plugin["old_code"]}"', f'pluginCode="{plugin["new_code"]}"'),
            (f'targetName="{plugin["old_name"]}"', f'targetName="{plugin["new_name"]}"'),
            (f'aaxIdentifier="com.nnaudio.{plugin["old_name"].lower()}"', f'aaxIdentifier="com.nnaudio.{plugin["new_name"].lower()}"'),
            (f'bundleIdentifier="com.nnaudio.{plugin["old_name"].lower()}"', f'bundleIdentifier="com.nnaudio.{plugin["new_name"].lower()}"'),
            (plugin["old_bg"], plugin["new_bg"]),
            ('pluginIsMidiEffectPlugin', 'pluginWantsMidiInput'),
        ]
        update_file(old_jucer, replacements)
        if old_jucer != new_jucer:
            old_jucer.rename(new_jucer)
        print(f"  ✓ Updated {plugin['new_name']}.jucer")
    
    # 2. Update C++ source files
    source_dir = plugin_path / "Source"
    if source_dir.exists():
        for cpp_file in source_dir.glob("*.cpp"):
            replacements = [
                (f'{plugin["old_name"]}AudioProcessor', f'{plugin["new_name"]}AudioProcessor'),
                (f'{plugin["old_name"]}AudioProcessorEditor', f'{plugin["new_name"]}AudioProcessorEditor'),
                ('orbitals-design-system.css', 'mandelbrot-design-system.css'),
                ('orbitals-components.js', 'mandelbrot-components.js'),
                ('orbitals-animations.js', 'mandelbrot-animations.js'),
                ('orbitals-particles.js', 'mandelbrot-particles.js'),
                (plugin["old_bg"], plugin["new_bg"]),
                (f'getChildFile ("{plugin["old_name"]}")', f'getChildFile ("MandelbrotSet").getChildFile ("{plugin["new_name"]}")'),
                (f'{plugin["old_name"]}UI_', f'{plugin["new_name"]}UI_'),
                (f'{plugin["old_name"]} -', f'{plugin["new_name"]} -'),
            ]
            if update_file(cpp_file, replacements):
                print(f"  ✓ Updated {cpp_file.name}")
        
        for h_file in source_dir.glob("*.h"):
            replacements = [
                (f'{plugin["old_name"]}AudioProcessor', f'{plugin["new_name"]}AudioProcessor'),
                (f'{plugin["old_name"]}AudioProcessorEditor', f'{plugin["new_name"]}AudioProcessorEditor'),
                (f'{plugin["old_name"]} -', f'{plugin["new_name"]} -'),
                ('MIDI FX', 'Audio FX'),
            ]
            if update_file(h_file, replacements):
                print(f"  ✓ Updated {h_file.name}")
    
    # 3. Update product_id.txt
    product_id_file = plugin_path / "product_id.txt"
    if product_id_file.exists():
        product_id_file.write_text(plugin["product_id"])
        print(f"  ✓ Updated product_id.txt → {plugin['product_id']}")
    
    # 4. Update CMakeLists.txt
    cmake_file = plugin_path / "CMakeLists.txt"
    if cmake_file.exists():
        replacements = [
            (f'project({plugin["old_name"]}', f'project({plugin["new_name"]}'),
            (f'PROJECT_NAME {plugin["old_name"]}', f'PROJECT_NAME {plugin["new_name"]}'),
            (f'PRODUCT_NAME "{plugin["old_name"]}"', f'PRODUCT_NAME "{plugin["new_name"]}"'),
            (f'PLUGIN_CODE {plugin["old_code"]}', f'PLUGIN_CODE {plugin["new_code"]}'),
            (plugin["old_bg"], plugin["new_bg"]),
        ]
        if update_file(cmake_file, replacements):
            print(f"  ✓ Updated CMakeLists.txt")
    
    print()

print("✅ All plugins updated!")
print("\n📋 Next steps:")
print("1. Verify the changes look correct")
print("2. Test building one plugin")
print("3. Update any remaining references if needed")
