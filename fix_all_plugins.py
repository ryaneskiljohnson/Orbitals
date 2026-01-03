#!/usr/bin/env python3
"""
Fix all Orbitals plugins to match Tidal's working structure:
1. Add juce_audio_plugin_client module to .jucer
2. Add MODULEPATH entries
3. Copy plugin_client include files
4. Add version hints to parameters
5. Add native title bar setup
6. Fix HTML loading to use file-based approach
"""

import os
import re
import shutil
import xml.etree.ElementTree as ET
from pathlib import Path

PLUGINS = ["Apogee", "Kepler", "Lagrange", "Eclipse", "Ion", "Perihelion", "Retrograde"]
TIDAL_DIR = "Tidal"
TIDAL_JL = f"{TIDAL_DIR}/JuceLibraryCode"

def add_module_to_jucer(jucer_path, module_id):
    """Add juce_audio_plugin_client module to .jucer file"""
    tree = ET.parse(jucer_path)
    root = tree.getroot()
    
    modules = root.find('MODULES')
    if modules is None:
        return False
    
    # Check if already exists
    for mod in modules.findall('MODULE'):
        if mod.get('id') == module_id:
            return False  # Already exists
    
    # Find insertion point (after juce_audio_formats, before juce_audio_processors)
    insert_pos = None
    for i, mod in enumerate(modules.findall('MODULE')):
        mod_id = mod.get('id')
        if mod_id == 'juce_audio_formats':
            insert_pos = i + 1
        elif mod_id == 'juce_audio_processors' and insert_pos is None:
            insert_pos = i
    
    # Create new module element
    new_mod = ET.Element('MODULE')
    new_mod.set('id', module_id)
    new_mod.set('showAllCode', '1')
    new_mod.set('useLocalCopy', '1')
    new_mod.set('useGlobalPath', '0')
    
    if insert_pos is not None:
        modules.insert(insert_pos, new_mod)
    else:
        modules.append(new_mod)
    
    tree.write(jucer_path, encoding='utf-8', xml_declaration=True)
    return True

def add_modulepath_to_jucer(jucer_path, module_id):
    """Add MODULEPATH entries for juce_audio_plugin_client"""
    with open(jucer_path, 'r') as f:
        content = f.read()
    
    # Pattern to find MODULEPATH sections
    pattern = r'(<MODULEPATH id="juce_audio_formats" path="JUCE/modules" />)'
    replacement = r'\1\n        <MODULEPATH id="juce_audio_plugin_client" path="JUCE/modules" />'
    
    if f'<MODULEPATH id="{module_id}"' in content:
        return False  # Already exists
    
    content = re.sub(pattern, replacement, content)
    
    with open(jucer_path, 'w') as f:
        f.write(content)
    
    return True

def copy_plugin_client_files(source_jl, dest_jl):
    """Copy plugin_client include files"""
    if not os.path.exists(source_jl) or not os.path.exists(dest_jl):
        return 0
    
    plugin_client_files = [f for f in os.listdir(source_jl) if 'plugin_client' in f]
    copied = 0
    for f in plugin_client_files:
        src = os.path.join(source_jl, f)
        dst = os.path.join(dest_jl, f)
        if os.path.exists(src):
            shutil.copy2(src, dst)
            copied += 1
    return copied

def add_version_hints_to_processor(processor_path):
    """Add version hints to parameters in PluginProcessor.cpp"""
    with open(processor_path, 'r') as f:
        content = f.read()
    
    # Pattern to match AudioParameterFloat/Choice/Int/Bool without ParameterID
    patterns = [
        (r'std::make_unique<juce::AudioParameterFloat>\s*\(\s*([A-Z_]+),', 
         r'std::make_unique<juce::AudioParameterFloat> (\n        juce::ParameterID (\1, {version}),'),
        (r'std::make_unique<juce::AudioParameterChoice>\s*\(\s*([A-Z_]+),',
         r'std::make_unique<juce::AudioParameterChoice> (\n        juce::ParameterID (\1, {version}),'),
        (r'std::make_unique<juce::AudioParameterInt>\s*\(\s*([A-Z_]+),',
         r'std::make_unique<juce::AudioParameterInt> (\n        juce::ParameterID (\1, {version}),'),
        (r'std::make_unique<juce::AudioParameterBool>\s*\(\s*([A-Z_]+),',
         r'std::make_unique<juce::AudioParameterBool> (\n        juce::ParameterID (\1, {version}),'),
    ]
    
    # This is complex - let's do it manually per plugin
    return False

def add_native_title_bar(editor_path):
    """Add native title bar setup to PluginEditor.cpp"""
    with open(editor_path, 'r') as f:
        content = f.read()
    
    if 'TopLevelWindow::getTopLevelWindow' in content:
        return False  # Already added
    
    # Find constructor
    pattern = r'(AudioProcessorEditor::AudioProcessorEditor\s*\([^)]+\)\s*:\s*AudioProcessorEditor\s*\([^)]+\),\s*audioProcessor\s*\([^)]+\)\s*\{)'
    replacement = r'\1\n    // Enable native title bar on the top-level window (for standalone builds)\n    if (auto* top_level = juce::TopLevelWindow::getTopLevelWindow(0))\n        top_level->setUsingNativeTitleBar(true);\n'
    
    if re.search(pattern, content):
        content = re.sub(pattern, replacement, content)
        with open(editor_path, 'w') as f:
            f.write(content)
        return True
    
    return False

def fix_html_loading(editor_path):
    """Fix HTML loading to use file-based approach"""
    with open(editor_path, 'r') as f:
        content = f.read()
    
    if 'tempDir.getChildFile' in content and 'file://' in content:
        return False  # Already fixed
    
    # Replace data URL approach with file-based
    pattern = r'webView->goToURL\("data:text/html;charset=utf-8,"\s*\+\s*juce::URL::addEscapeChars\(htmlContent,\s*true\)\);'
    replacement = '''    // Load HTML using temporary file approach (avoids data URL encoding issues)
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("PLUGIN_NAMEUI_" + juce::String(juce::Time::currentTimeMillis()));
    tempDir.createDirectory();
    
    auto tempFile = tempDir.getChildFile("index.html");
    tempFile.replaceWithText(htmlContent);
    
    auto filePath = tempFile.getFullPathName().replace(" ", "%20");
    juce::String fileURL = "file://" + filePath;
    webView->goToURL(fileURL);'''
    
    if re.search(pattern, content):
        # Get plugin name from path
        plugin_name = os.path.basename(os.path.dirname(os.path.dirname(editor_path)))
        replacement = replacement.replace("PLUGIN_NAME", plugin_name)
        content = re.sub(pattern, replacement, content)
        with open(editor_path, 'w') as f:
            f.write(content)
        return True
    
    return False

def main():
    print("Fixing all Orbitals plugins...")
    
    for plugin in PLUGINS:
        print(f"\n=== {plugin} ===")
        
        jucer_path = f"{plugin}/{plugin}.jucer"
        processor_path = f"{plugin}/Source/PluginProcessor.cpp"
        editor_path = f"{plugin}/Source/PluginEditor.cpp"
        jl_path = f"{plugin}/JuceLibraryCode"
        
        # 1. Copy plugin_client files
        if os.path.exists(TIDAL_JL) and os.path.exists(jl_path):
            copied = copy_plugin_client_files(TIDAL_JL, jl_path)
            print(f"  ✓ Copied {copied} plugin_client files")
        
        # 2. Add module to .jucer (manual - complex XML)
        # 3. Add version hints (manual - need to count parameters)
        # 4. Add native title bar
        if os.path.exists(editor_path):
            if add_native_title_bar(editor_path):
                print(f"  ✓ Added native title bar setup")
        
        # 5. Fix HTML loading
        if os.path.exists(editor_path):
            if fix_html_loading(editor_path):
                print(f"  ✓ Fixed HTML loading")
    
    print("\n✓ Basic fixes applied!")
    print("NOTE: .jucer module additions and version hints need manual application")
    print("      See Tidal as reference for exact patterns")

if __name__ == "__main__":
    main()
