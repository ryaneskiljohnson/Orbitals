#!/usr/bin/env python3
"""
Unify all parameter handlers to use the Fabric/Eclipse pattern:
- Use range.convertTo0to1() instead of manual divisions
- Works with any parameter range automatically
"""

import re
from pathlib import Path

UNIFIED_HANDLER = '''    if (type == "parameterChange")
    {
        auto param = obj->getProperty("parameter").toString();
        auto value = obj->getProperty("value");
        
        auto* p = audioProcessor.parameters.getParameter(param);
        if (p != nullptr)
        {
            auto& range = p->getNormalisableRange();
            float rawValue = static_cast<float>(value);
            rawValue = juce::jlimit(range.start, range.end, rawValue);
            float normalizedValue = range.convertTo0to1(rawValue);
            p->setValueNotifyingHost(normalizedValue);
        }
    }'''

PLUGINS_TO_UPDATE = [
    # Mandelbrot Series
    ('MandelbrotSet/Entanglement', 'EntanglementAudioProcessorEditor'),
    ('MandelbrotSet/Hadron', 'HadronAudioProcessorEditor'),
    ('MandelbrotSet/Mandelbrot', 'MandelbrotAudioProcessorEditor'),
    ('MandelbrotSet/Observer', 'ObserverAudioProcessorEditor'),
    ('MandelbrotSet/Planck', 'PlanckAudioProcessorEditor'),
    ('MandelbrotSet/Quarks', 'QuarksAudioProcessorEditor'),
    ('MandelbrotSet/Rabbithole', 'RabbitholeAudioProcessorEditor'),
    ('MandelbrotSet/Singularity', 'SingularityAudioProcessorEditor'),
    
    # Orbital Series
    ('Apogee', 'ApogeeAudioProcessorEditor'),
    ('Ion', 'IonAudioProcessorEditor'),
    ('Kepler', 'KeplerAudioProcessorEditor'),
    ('Lagrange', 'LagrangeAudioProcessorEditor'),
    ('Perihelion', 'PerihelionAudioProcessorEditor'),
    ('Retrograde', 'RetrogradeAudioProcessorEditor'),
    ('Tidal', 'TidalAudioProcessorEditor'),
    ('Zenith', 'ZenithAudioProcessorEditor'),
]

def find_parameter_handler_section(content):
    """Find the parameterChange handler section - find from 'if (type == "parameterChange")' to closing brace"""
    start_marker = 'if (type == "parameterChange")'
    start_pos = content.find(start_marker)
    
    if start_pos == -1:
        return None, None
    
    # Find the matching closing brace
    brace_count = 0
    pos = start_pos
    start_brace = content.find('{', pos)
    if start_brace == -1:
        return None, None
    
    pos = start_brace + 1
    brace_count = 1
    
    while pos < len(content) and brace_count > 0:
        if content[pos] == '{':
            brace_count += 1
        elif content[pos] == '}':
            brace_count -= 1
        pos += 1
    
    if brace_count == 0:
        return start_pos, pos
    
    return None, None

def update_plugin(plugin_path, editor_class):
    """Update a single plugin's parameter handler"""
    editor_file = Path(plugin_path) / 'Source' / 'PluginEditor.cpp'
    
    if not editor_file.exists():
        print(f"  ⚠️  {editor_file} not found")
        return False
    
    content = editor_file.read_text()
    
    # Check if already using unified handler
    if 'range.convertTo0to1(rawValue)' in content:
        print(f"  ✅ Already using unified handler")
        return True
    
    # Find the parameterChange section
    start, end = find_parameter_handler_section(content)
    
    if start is None or end is None:
        print(f"  ⚠️  Could not find parameterChange handler")
        return False
    
    # Extract the old handler
    old_handler = content[start:end]
    
    # Replace with unified handler
    new_content = content[:start] + UNIFIED_HANDLER + content[end:]
    
    # Write back
    editor_file.write_text(new_content)
    print(f"  ✅ Updated parameter handler")
    return True

def main():
    print("╔" + "="*68 + "╗")
    print("║" + " "*68 + "║")
    print("║" + "  UNIFY PARAMETER HANDLERS - FABRIC PATTERN".center(68) + "║")
    print("║" + " "*68 + "║")
    print("╚" + "="*68 + "╝")
    print()
    
    base_path = Path('/Users/rjmacbookpro/Development/Orbitals')
    
    updated = 0
    skipped = 0
    failed = 0
    
    for plugin_path, editor_class in PLUGINS_TO_UPDATE:
        full_path = base_path / plugin_path
        plugin_name = Path(plugin_path).name
        
        print(f"🔧 {plugin_name}")
        print(f"   Path: {full_path}")
        
        if update_plugin(full_path, editor_class):
            updated += 1
        else:
            failed += 1
        
        print()
    
    print("╔" + "="*68 + "╗")
    print("║" + " "*68 + "║")
    print("║" + f"  SUMMARY: {updated} updated, {failed} failed".center(68) + "║")
    print("║" + " "*68 + "║")
    print("╚" + "="*68 + "╝")

if __name__ == '__main__':
    main()
