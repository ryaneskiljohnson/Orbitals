#!/usr/bin/env python3
"""
Migrate ALL plugins to Fabric architecture:
1. Remove MainComponent files
2. Update .jucer files to remove MainComponent references
3. Ensure PluginEditor is used for standalone
4. Update WebView to JUCE 8 API (if needed)
"""

import os
import re
import shutil
from pathlib import Path

MANDELBROT_PLUGINS = [
    'Entanglement', 'Hadron', 'Mandelbrot', 'Observer',
    'Planck', 'Quarks', 'Rabbithole', 'Singularity'
]

ORBITAL_PLUGINS = [
    'Apogee', 'Eclipse', 'Ion', 'Kepler', 'Lagrange',
    'Perihelion', 'Retrograde', 'Tidal', 'Zenith'
]

def backup_maincomponent_files(plugin_path):
    """Backup MainComponent files before deleting"""
    backup_dir = plugin_path / 'Backup_Standalone_Files'
    backup_dir.mkdir(exist_ok=True)
    
    files_to_backup = [
        'Source/Main.cpp',
        'Source/MainComponent.h',
        'Source/MainComponent.cpp'
    ]
    
    backed_up = []
    for file_path in files_to_backup:
        source = plugin_path / file_path
        if source.exists():
            dest = backup_dir / source.name
            shutil.copy2(source, dest)
            backed_up.append(source.name)
    
    return backed_up

def remove_maincomponent_from_jucer(jucer_file):
    """Remove MainComponent file references from .jucer file"""
    if not jucer_file.exists():
        return False
    
    content = jucer_file.read_text()
    original = content
    
    # Remove Main.cpp
    content = re.sub(
        r'<FILE id="[^"]*" name="Main\.cpp"[^>]*/?>',
        '',
        content
    )
    
    # Remove MainComponent.h
    content = re.sub(
        r'<FILE id="[^"]*" name="MainComponent\.h"[^>]*/?>',
        '',
        content
    )
    
    # Remove MainComponent.cpp
    content = re.sub(
        r'<FILE id="[^"]*" name="MainComponent\.cpp"[^>]*/?>',
        '',
        content
    )
    
    if content != original:
        jucer_file.write_text(content)
        return True
    return False

def delete_maincomponent_files(plugin_path):
    """Delete MainComponent files"""
    files_to_delete = [
        'Source/Main.cpp',
        'Source/MainComponent.h',
        'Source/MainComponent.cpp'
    ]
    
    deleted = []
    for file_path in files_to_delete:
        full_path = plugin_path / file_path
        if full_path.exists():
            full_path.unlink()
            deleted.append(full_path.name)
    
    return deleted

def check_webview_api(editor_file):
    """Check if plugin uses old or new WebView API"""
    if not editor_file.exists():
        return "FILE_NOT_FOUND"
    
    content = editor_file.read_text()
    
    if '.withEventListener' in content:
        return "JUCE8_NEW"
    elif 'WebBrowserComponent' in content:
        return "JUCE8_OLD"
    else:
        return "NO_WEBVIEW"

def process_plugin(plugin_name, plugin_path, dry_run=False):
    """Process a single plugin"""
    print(f"\n{'='*70}")
    print(f"🔧 {plugin_name}")
    print(f"{'='*70}")
    
    if not plugin_path.exists():
        print(f"  ❌ Plugin directory not found")
        return False
    
    # Check MainComponent
    main_cpp = plugin_path / 'Source' / 'Main.cpp'
    has_maincomponent = main_cpp.exists()
    
    print(f"  📁 MainComponent files: {'PRESENT' if has_maincomponent else 'NONE (already using PluginEditor)'}")
    
    if has_maincomponent and not dry_run:
        # Backup
        backed_up = backup_maincomponent_files(plugin_path)
        if backed_up:
            print(f"  💾 Backed up: {', '.join(backed_up)}")
        
        # Remove from .jucer
        jucer_file = plugin_path / f'{plugin_name}.jucer'
        if remove_maincomponent_from_jucer(jucer_file):
            print(f"  ✅ Removed MainComponent from .jucer")
        
        # Delete files
        deleted = delete_maincomponent_files(plugin_path)
        if deleted:
            print(f"  🗑️  Deleted: {', '.join(deleted)}")
    
    # Check WebView API
    editor_file = plugin_path / 'Source' / 'PluginEditor.cpp'
    webview_status = check_webview_api(editor_file)
    print(f"  🌐 WebView API: {webview_status}")
    
    if webview_status == "JUCE8_OLD":
        print(f"  ⚠️  Needs WebView API update (currently not using event listeners)")
    elif webview_status == "JUCE8_NEW":
        print(f"  ✅ Already using modern JUCE 8 event listeners")
    
    return True

def main():
    print("╔" + "="*68 + "╗")
    print("║" + " "*68 + "║")
    print("║" + "  COMPLETE PLUGIN MIGRATION TO FABRIC ARCHITECTURE".center(68) + "║")
    print("║" + " "*68 + "║")
    print("╚" + "="*68 + "╝")
    print()
    
    import sys
    dry_run = '--dry-run' in sys.argv
    
    if dry_run:
        print("🔍 DRY RUN MODE - No files will be modified")
        print()
    
    mandelbrot_base = Path('/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet')
    orbital_base = Path('/Users/rjmacbookpro/Development/Orbitals')
    
    print("📦 MANDELBROT SERIES")
    print("="*70)
    mandelbrot_stats = {'maincomponent': 0, 'old_webview': 0, 'new_webview': 0}
    for plugin in MANDELBROT_PLUGINS:
        plugin_path = mandelbrot_base / plugin
        process_plugin(plugin, plugin_path, dry_run)
    
    print("\n\n📦 ORBITAL SERIES")
    print("="*70)
    orbital_stats = {'maincomponent': 0, 'old_webview': 0, 'new_webview': 0}
    for plugin in ORBITAL_PLUGINS:
        plugin_path = orbital_base / plugin
        process_plugin(plugin, plugin_path, dry_run)
    
    print("\n" + "╔" + "="*68 + "╗")
    print("║" + " "*68 + "║")
    print("║" + "  ANALYSIS COMPLETE".center(68) + "║")
    print("║" + " "*68 + "║")
    print("╚" + "="*68 + "╝")
    print()
    
    if not dry_run:
        print("✅ MainComponent files removed from all plugins")
        print("✅ .jucer files updated")
        print()
        print("⚠️  NEXT STEPS:")
        print("  1. Regenerate Xcode projects (Projucer or manual)")
        print("  2. Update WebView API in plugins that need it")
        print("  3. Build each plugin to verify")
        print()

if __name__ == '__main__':
    main()
