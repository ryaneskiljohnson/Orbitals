#!/usr/bin/env python3
"""
Fix malformed sendMeteringData implementations
"""

from pathlib import Path
import re

PLUGINS = ['Mandelbrot', 'Observer', 'Planck', 'Quarks', 'Rabbithole', 'Singularity']

def fix_plugin(plugin_name):
    """Fix malformed code in plugin"""
    cpp_file = Path(f'MandelbrotSet/{plugin_name}/Source/PluginEditor.cpp')
    
    if not cpp_file.exists():
        return False
    
    content = cpp_file.read_text()
    original = content
    
    # Remove the extra closing braces after sendMeteringData
    # Pattern: }
    #         }
    #     }
    # }
    # Should be just: }
    
    pattern = r'(\}\n)\s+\}\n\s+\}\n\}\n\nvoid ' + plugin_name + r'AudioProcessorEditor::notifyMIDINote'
    replacement = r'\1\nvoid ' + plugin_name + r'AudioProcessorEditor::notifyMIDINote'
    
    content = re.sub(pattern, replacement, content)
    
    if content != original:
        cpp_file.write_text(content)
        print(f"  ✅ Fixed {plugin_name}")
        return True
    else:
        print(f"  ⚠️  No changes for {plugin_name}")
        return False

def main():
    print("Fixing malformed code...")
    for plugin in PLUGINS:
        print(f"🔧 {plugin}")
        fix_plugin(plugin)

if __name__ == '__main__':
    main()
