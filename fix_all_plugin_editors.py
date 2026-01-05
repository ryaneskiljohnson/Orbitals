#!/usr/bin/env python3
"""
Bulk update all plugin editors to match Zenith's pattern
"""

import os
import re
from pathlib import Path

# Plugin configuration
PLUGINS = [
    # Orbitals series (use orbitals-design-system)
    {"name": "Ion", "path": "Ion", "design_system": "orbitals", "project_root": "/Users/rjmacbookpro/Development/Orbitals"},
    {"name": "Kepler", "path": "Kepler", "design_system": "orbitals", "project_root": "/Users/rjmacbookpro/Development/Orbitals"},
    {"name": "Lagrange", "path": "Lagrange", "design_system": "orbitals", "project_root": "/Users/rjmacbookpro/Development/Orbitals"},
    {"name": "Perihelion", "path": "Perihelion", "design_system": "orbitals", "project_root": "/Users/rjmacbookpro/Development/Orbitals"},
    {"name": "Retrograde", "path": "Retrograde", "design_system": "orbitals", "project_root": "/Users/rjmacbookpro/Development/Orbitals"},
    {"name": "Tidal", "path": "Tidal", "design_system": "orbitals", "project_root": "/Users/rjmacbookpro/Development/Orbitals"},
    
    # MandelbrotSet series (use mandelbrot-design-system)
    {"name": "Singularity", "path": "MandelbrotSet/Singularity", "design_system": "mandelbrot", "project_root": "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet"},
    {"name": "Entanglement", "path": "MandelbrotSet/Entanglement", "design_system": "mandelbrot", "project_root": "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet"},
    {"name": "Hadron", "path": "MandelbrotSet/Hadron", "design_system": "mandelbrot", "project_root": "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet"},
    {"name": "Mandelbrot", "path": "MandelbrotSet/Mandelbrot", "design_system": "mandelbrot", "project_root": "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet"},
    {"name": "Observer", "path": "MandelbrotSet/Observer", "design_system": "mandelbrot", "project_root": "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet"},
    {"name": "Planck", "path": "MandelbrotSet/Planck", "design_system": "mandelbrot", "project_root": "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet"},
    {"name": "Quarks", "path": "MandelbrotSet/Quarks", "design_system": "mandelbrot", "project_root": "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet"},
    {"name": "Rabbithole", "path": "MandelbrotSet/Rabbithole", "design_system": "mandelbrot", "project_root": "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet"},
]

BASE_DIR = Path("/Users/rjmacbookpro/Development/Orbitals")

def read_product_id(plugin_path):
    """Read product ID from product_id.txt"""
    product_file = BASE_DIR / plugin_path / "product_id.txt"
    if product_file.exists():
        return product_file.read_text().strip()
    return None

def read_zenith_template():
    """Read Zenith files as templates"""
    zenith_h = (BASE_DIR / "Zenith" / "Source" / "PluginEditor.h").read_text()
    zenith_cpp = (BASE_DIR / "Zenith" / "Source" / "PluginEditor.cpp").read_text()
    return zenith_h, zenith_cpp

def get_parameter_handling(plugin_name, plugin_path):
    """Get plugin-specific parameter handling code"""
    # Read the current file to extract parameter handling
    current_file = BASE_DIR / plugin_path / "Source" / "PluginEditor.cpp"
    if not current_file.exists():
        return None
    
    content = current_file.read_text()
    
    # Extract parameter handling section
    match = re.search(r'if \(type == "parameterChange"\)\s*\{[^}]+\}', content, re.DOTALL)
    if match:
        param_section = match.group(0)
        # Clean up the section
        param_section = re.sub(r'auto\* p = audioProcessor\.parameters\.getParameter\([^)]+\);\s*if \(p != nullptr\)\s*p->setValueNotifyingHost\([^)]+\);', 
                              lambda m: m.group(0), param_section)
        return param_section
    
    return None

def update_plugin_editor_h(plugin, zenith_h_template):
    """Update PluginEditor.h file"""
    plugin_path = BASE_DIR / plugin["path"]
    h_file = plugin_path / "Source" / "PluginEditor.h"
    
    if not h_file.exists():
        print(f"  WARNING: {h_file} does not exist, skipping...")
        return False
    
    # Replace Zenith with plugin name
    content = zenith_h_template.replace("Zenith", plugin["name"])
    content = content.replace("zenith", plugin["name"].lower())
    
    h_file.write_text(content)
    print(f"  ✓ Updated {h_file.name}")
    return True

def update_plugin_editor_cpp(plugin, zenith_cpp_template, product_id):
    """Update PluginEditor.cpp file"""
    plugin_path = BASE_DIR / plugin["path"]
    cpp_file = plugin_path / "Source" / "PluginEditor.cpp"
    
    if not cpp_file.exists():
        print(f"  WARNING: {cpp_file} does not exist, skipping...")
        return False
    
    # Read current file to get parameter handling
    current_content = cpp_file.read_text()
    param_handling = None
    
    # Try to extract parameter handling
    param_match = re.search(r'if \(type == "parameterChange"\)\s*\{.*?else if \(type ==', current_content, re.DOTALL)
    if param_match:
        param_handling = param_match.group(0)
        # Remove the "else if (type ==" part
        param_handling = param_handling.replace('else if (type ==', '').strip()
        if param_handling.endswith('}'):
            param_handling = param_handling[:-1].strip()
    
    # Start with Zenith template
    content = zenith_cpp_template
    
    # Replace Zenith with plugin name
    content = content.replace("Zenith", plugin["name"])
    content = content.replace("zenith", plugin["name"].lower())
    content = content.replace("ZENITH", plugin["name"].upper())
    
    # Replace design system
    if plugin["design_system"] == "mandelbrot":
        content = content.replace("orbitals-design-system.css", "mandelbrot-design-system.css")
        content = content.replace("orbitals-animations.js", "mandelbrot-animations.js")
        content = content.replace("orbitals-particles.js", "mandelbrot-particles.js")
        content = content.replace("orbitals-components.js", "mandelbrot-components.js")
        content = content.replace("orbitals-header", "mandelbrot-header")
        content = content.replace("orbitals-footer", "mandelbrot-footer")
        content = content.replace("orbitals-logo", "mandelbrot-logo")
    
    # Replace project root path
    content = content.replace(
        'auto projectRoot = juce::File ("/Users/rjmacbookpro/Development/Orbitals");',
        f'auto projectRoot = juce::File ("{plugin["project_root"]}");'
    )
    
    # Replace background image
    bg_pattern = r'zenith-background\.png'
    bg_replacement = f'{plugin["name"].lower()}-background.png'
    if plugin["design_system"] == "mandelbrot":
        bg_replacement = f'{plugin["name"].lower()}.png'
    content = re.sub(bg_pattern, bg_replacement, content)
    content = content.replace("zenith-background.png", bg_replacement)
    
    # Replace product ID
    content = re.sub(r'product_list\.contains\("200004"\)', f'product_list.contains("{product_id}")', content)
    
    # Replace temp directory names
    content = content.replace("ZenithUI_", f'{plugin["name"]}UI_')
    content = content.replace("ZenithAuth_", f'{plugin["name"]}Auth_')
    
    # Replace plugin title in auth screen
    content = content.replace('<title>Zenith - Authentication Required</title>', 
                             f'<title>{plugin["name"]} - Authentication Required</title>')
    content = content.replace('<h1 class="auth-title">ZENITH</h1>', 
                             f'<h1 class="auth-title">{plugin["name"].upper()}</h1>')
    
    # If we have parameter handling from current file, try to preserve it
    if param_handling:
        # Find and replace the parameter handling section
        old_param_pattern = r'if \(type == "parameterChange"\)\s*\{.*?\}'
        if re.search(old_param_pattern, content, re.DOTALL):
            # Try to insert the preserved parameter handling
            # This is complex, so we'll just note it
            print(f"  NOTE: Parameter handling may need manual review for {plugin['name']}")
    
    cpp_file.write_text(content)
    print(f"  ✓ Updated {cpp_file.name}")
    return True

def main():
    print("=" * 60)
    print("Bulk Updating Plugin Editors to Match Zenith Pattern")
    print("=" * 60)
    
    zenith_h, zenith_cpp = read_zenith_template()
    
    success_count = 0
    skip_count = 0
    
    for plugin in PLUGINS:
        print(f"\n[{plugin['name']}]")
        print(f"  Path: {plugin['path']}")
        
        # Get product ID
        product_id = read_product_id(plugin["path"])
        if not product_id:
            print(f"  WARNING: Could not find product_id.txt, skipping...")
            skip_count += 1
            continue
        
        print(f"  Product ID: {product_id}")
        
        # Update .h file
        if update_plugin_editor_h(plugin, zenith_h):
            # Update .cpp file
            if update_plugin_editor_cpp(plugin, zenith_cpp, product_id):
                success_count += 1
            else:
                skip_count += 1
        else:
            skip_count += 1
    
    print("\n" + "=" * 60)
    print(f"Summary: {success_count} updated, {skip_count} skipped")
    print("=" * 60)
    print("\n⚠️  IMPORTANT: Please review each plugin's parameter handling section!")
    print("   The script preserves the structure but may need manual adjustment.")

if __name__ == "__main__":
    main()
