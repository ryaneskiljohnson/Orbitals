#!/usr/bin/env python3
"""
Update all PluginProcessor parameter definitions to match UI
"""
import re
from pathlib import Path

base_dir = Path("/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet")

# Parameter mappings for each plugin
plugin_params = {
    "Singularity": {
        "params": ["threshold", "ratio", "attack", "release", "makeup", "bypass"],
        "old_params": ["amplitude", "phase", "rate", "shape", "bypass"],
        "old_identifier": "Tidal"
    },
    "Fabric": {
        "params": ["size", "diffusion", "damping", "predelay", "mix", "bypass"],
        "old_params": ["lift", "curve", "ceiling", "momentum", "bypass"],
        "old_identifier": "Apogee"
    },
    "Entanglement": {
        "params": ["time", "feedback", "mix", "damping", "bypass"],
        "old_params": ["distance", "coupling", "coherence", "decay", "bypass"],
        "old_identifier": "Lagrange"
    },
    "Planck": {
        "params": ["lowfreq", "lowgain", "midfreq", "midgain", "highfreq", "highgain", "q", "bypass"],
        "old_params": ["lowfreq", "lowgain", "midfreq", "midgain", "highfreq", "highgain", "q", "bypass"],
        "old_identifier": "Eclipse"
    },
    "Observer": {
        "params": ["threshold", "frequency", "ratio", "attack", "release", "bypass"],
        "old_params": ["measurement", "precision", "collapseratio", "observationtime", "uncertainty", "bypass"],
        "old_identifier": "Ion"
    },
    "Hadron": {
        "params": ["drive", "tone", "bias", "mix", "output", "bypass"],
        "old_params": ["collisionenergy", "particlemass", "acceleration", "luminosity", "beamfocus", "bypass"],
        "old_identifier": "Perihelion"
    },
    "Mandelbrot": {
        "params": ["rate", "depth", "feedback", "stages", "mix", "bypass"],
        "old_params": ["zoom", "boundary", "selfsimilarity", "iterations", "dimension", "bypass"],
        "old_identifier": "Retrograde"
    },
    "Quarks": {
        "params": ["threshold", "ratio", "attack", "release", "range", "bypass"],
        "old_params": ["strongforce", "colorcharge", "binding", "decay", "confinement", "bypass"],
        "old_identifier": "Kepler"
    },
    "Rabbithole": {
        "params": ["rate", "depth", "feedback", "stereo", "mix", "bypass"],
        "old_params": ["descentrate", "tunneldepth", "gravitywell", "duality", "wonderland", "bypass"],
        "old_identifier": "Zenith"
    }
}

def update_processor_h(plugin_dir, params, old_params):
    h_file = base_dir / plugin_dir / "Source" / "PluginProcessor.h"
    if not h_file.exists():
        return False
    
    content = h_file.read_text(encoding='utf-8', errors='ignore')
    original = content
    
    # Update parameter declarations
    param_decls = []
    for i, param in enumerate(params, 1):
        param_upper = param.upper()
        param_decls.append(f"    static const juce::String PARAM_{param_upper};")
    
    # Find and replace parameter declarations
    param_pattern = r"    // Parameter IDs\s*\n(?:    static const juce::String PARAM_\w+;\s*\n)+"
    replacement = "    // Parameter IDs\n" + "\n".join(param_decls) + "\n"
    content = re.sub(param_pattern, replacement, content)
    
    if content != original:
        h_file.write_text(content, encoding='utf-8')
        print(f"  ✓ Updated {plugin_dir}/Source/PluginProcessor.h")
        return True
    return False

def update_processor_cpp(plugin_dir, params, old_params, old_identifier):
    cpp_file = base_dir / plugin_dir / "Source" / "PluginProcessor.cpp"
    if not cpp_file.exists():
        return False
    
    content = cpp_file.read_text(encoding='utf-8', errors='ignore')
    original = content
    
    # Update parameter ID definitions
    param_defs = []
    for i, param in enumerate(params, 1):
        param_upper = param.upper()
        param_defs.append(f'const juce::String {plugin_dir}AudioProcessor::PARAM_{param_upper} = "{param}";')
    
    # Find and replace parameter definitions
    param_pattern = r"// Parameter IDs\s*\n(?:const juce::String \w+AudioProcessor::PARAM_\w+ = \"\w+\";\s*\n)+"
    replacement = "// Parameter IDs\n" + "\n".join(param_defs) + "\n"
    content = re.sub(param_pattern, replacement, content)
    
    # Update identifier
    content = content.replace(f'juce::Identifier ("{old_identifier}")', f'juce::Identifier ("{plugin_dir}")')
    
    # Update comment
    content = content.replace("MIDI FX Plugin Processor", "Audio FX Plugin Processor")
    content = content.replace("MIDI FX Plugin Processor Implementation", "Audio FX Plugin Processor Implementation")
    
    if content != original:
        cpp_file.write_text(content, encoding='utf-8')
        print(f"  ✓ Updated {plugin_dir}/Source/PluginProcessor.cpp")
        return True
    return False

print("🔧 Updating all parameter definitions...\n")

for plugin_dir, config in plugin_params.items():
    print(f"📦 Processing {plugin_dir}...")
    update_processor_h(plugin_dir, config["params"], config["old_params"])
    update_processor_cpp(plugin_dir, config["params"], config["old_params"], config["old_identifier"])

print("\n✅ All parameter definitions updated!")
