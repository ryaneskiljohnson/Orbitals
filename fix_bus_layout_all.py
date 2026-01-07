#!/usr/bin/env python3
"""
Fix isBusesLayoutSupported for all Mandelbrot plugins
Adds support for mono input (microphone) + stereo input (audio interface)
"""

from pathlib import Path

MANDELBROT_PLUGINS = [
    'Entanglement', 'Hadron', 'Mandelbrot', 'Observer',
    'Planck', 'Quarks', 'Rabbithole', 'Singularity'
]

OLD_BUS_LAYOUT = '''bool {plugin}AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
  #endif
}}'''

NEW_BUS_LAYOUT = '''bool {plugin}AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Accept mono or stereo input
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // Output must be stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return true;
  #endif
}}'''

def fix_plugin_bus_layout(plugin_name, base_path):
    """Fix bus layout for a single plugin"""
    plugin_path = base_path / plugin_name
    processor_file = plugin_path / 'Source' / 'PluginProcessor.cpp'
    
    if not processor_file.exists():
        print(f"  ❌ {plugin_name}: PluginProcessor.cpp not found")
        return False
    
    content = processor_file.read_text()
    
    old_code = OLD_BUS_LAYOUT.format(plugin=plugin_name)
    new_code = NEW_BUS_LAYOUT.format(plugin=plugin_name)
    
    if old_code in content:
        content = content.replace(old_code, new_code)
        processor_file.write_text(content)
        print(f"  ✅ {plugin_name}: Bus layout fixed (mono + stereo input support added)")
        return True
    elif 'layouts.getMainInputChannelSet()' in content:
        print(f"  ℹ️  {plugin_name}: Bus layout already checks input (already fixed)")
        return False
    else:
        print(f"  ⚠️  {plugin_name}: Bus layout method not found or different format")
        return False

def main():
    print("="*70)
    print("FIXING BUS LAYOUT FOR ALL MANDELBROT PLUGINS")
    print("="*70)
    print()
    
    base_path = Path('/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet')
    
    fixed_count = 0
    for plugin in MANDELBROT_PLUGINS:
        if fix_plugin_bus_layout(plugin, base_path):
            fixed_count += 1
    
    print()
    print("="*70)
    print(f"✅ Fixed {fixed_count}/{len(MANDELBROT_PLUGINS)} plugins")
    print("="*70)
    print()
    print("Next steps:")
    print("  1. Build each plugin to verify compilation")
    print("  2. Test with microphone input")
    print("  3. Verify mono and stereo inputs work")
    print()

if __name__ == '__main__':
    main()
