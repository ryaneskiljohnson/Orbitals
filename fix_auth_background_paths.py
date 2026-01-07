#!/usr/bin/env python3
"""
Fix auth screen background image paths in all Mandelbrot plugins.
Change from: /Users/rjmacbookpro/Development/Orbitals
To: /Users/rjmacbookpro/Development/Orbitals/MandelbrotSet
"""

import re
from pathlib import Path

MANDELBROT_PLUGINS = [
    'Entanglement', 'Hadron', 'Mandelbrot', 'Observer',
    'Planck', 'Quarks', 'Rabbithole', 'Singularity'
]

def fix_plugin(plugin_name):
    """Fix background image paths in a plugin's PluginEditor.cpp"""
    editor_file = Path(f'MandelbrotSet/{plugin_name}/Source/PluginEditor.cpp')
    
    if not editor_file.exists():
        print(f"  ⚠️  {editor_file} not found")
        return False
    
    content = editor_file.read_text()
    original = content
    
    # Fix projectRoot path in loadAuthScreen
    # Pattern 1: auto projectRoot = juce::File("/Users/rjmacbookpro/Development/Orbitals");
    pattern1 = r'auto projectRoot = juce::File\("/Users/rjmacbookpro/Development/Orbitals"\);'
    replacement1 = 'auto projectRoot = juce::File("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");'
    content = re.sub(pattern1, replacement1, content)
    
    # Pattern 2: auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals");
    pattern2 = r'auto projectRoot = juce::File \("/Users/rjmacbookpro/Development/Orbitals"\);'
    replacement2 = 'auto projectRoot = juce::File("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet");'
    content = re.sub(pattern2, replacement2, content)
    
    if content != original:
        editor_file.write_text(content)
        print(f"  ✅ Fixed projectRoot path")
        return True
    else:
        # Check if already correct
        if 'MandelbrotSet' in content:
            print(f"  ✅ Already correct")
            return True
        else:
            print(f"  ⚠️  No changes needed (pattern not found)")
            return False

def main():
    print("╔" + "="*68 + "╗")
    print("║" + " "*68 + "║")
    print("║" + "  FIX AUTH BACKGROUND IMAGE PATHS".center(68) + "║")
    print("║" + " "*68 + "║")
    print("╚" + "="*68 + "╝")
    print()
    
    base_path = Path('/Users/rjmacbookpro/Development/Orbitals')
    
    fixed = 0
    already_correct = 0
    
    for plugin in MANDELBROT_PLUGINS:
        print(f"🔧 {plugin}")
        if fix_plugin(plugin):
            fixed += 1
        else:
            already_correct += 1
        print()
    
    print("╔" + "="*68 + "╗")
    print("║" + " "*68 + "║")
    print("║" + f"  SUMMARY: {fixed} fixed, {already_correct} already correct".center(68) + "║")
    print("║" + " "*68 + "║")
    print("╚" + "="*68 + "╝")

if __name__ == '__main__':
    main()
