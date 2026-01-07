#!/usr/bin/env python3
"""
Verify all modulation/time-based effects have wet/dry controls
"""

from pathlib import Path

EFFECTS_TO_CHECK = {
    'Fabric': 'Reverb',
    'Entanglement': 'Delay',
    'Hadron': 'Saturation',
    'Mandelbrot': 'Phaser',
    'Rabbithole': 'Flanger',
}

def check_plugin(plugin_name, effect_type):
    """Check if plugin has wet/dry control"""
    processor_file = Path(f'MandelbrotSet/{plugin_name}/Source/PluginProcessor.cpp')
    
    if not processor_file.exists():
        return False, "File not found"
    
    content = processor_file.read_text()
    
    # Check for mix or wetdry parameter
    has_mix = 'PARAM_MIX' in content or 'PARAM_WETDRY' in content
    has_wet_dry_impl = 'input * (1.0f - mix)' in content or 'input * (1.0f - wetDry)' in content
    
    # Check parameter definition
    has_param_def = f'PARAM_MIX' in content or f'PARAM_WETDRY' in content
    
    status = []
    if has_mix or has_param_def:
        status.append("✅ Has parameter")
    if has_wet_dry_impl:
        status.append("✅ Has wet/dry implementation")
    
    return len(status) == 2, " | ".join(status) if status else "❌ Missing"

def main():
    print("╔" + "="*68 + "╗")
    print("║" + " "*68 + "║")
    print("║" + "  WET/DRY CONTROL VERIFICATION".center(68) + "║")
    print("║" + " "*68 + "║")
    print("╚" + "="*68 + "╝")
    print()
    
    all_good = True
    for plugin, effect in EFFECTS_TO_CHECK.items():
        print(f"🔧 {plugin} ({effect})")
        is_ok, status = check_plugin(plugin, effect)
        print(f"   {status}")
        if not is_ok:
            all_good = False
        print()
    
    if all_good:
        print("✅ All effects have wet/dry controls!")
    else:
        print("⚠️  Some effects need wet/dry controls added")

if __name__ == '__main__':
    main()
