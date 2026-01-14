#!/usr/bin/env python3
"""
Script to rename plugins from old names to new retro gaming names
ByteBeat: Rabbithole -> ByteBeat
ChipTune: Singularity -> ChipTune
"""

import os
import re
from pathlib import Path

# Mapping: old_name -> new_name
RENAMES = {
    'Rabbithole': 'ByteBeat',
    'rabbithole': 'bytebeat',
    'RabbitholeAudioProcessor': 'ByteBeatAudioProcessor',
    'RabbitholeAudioProcessorEditor': 'ByteBeatAudioProcessorEditor',
    'Singularity': 'ChipTune',
    'singularity': 'chiptune',
    'SingularityAudioProcessor': 'ChipTuneAudioProcessor',
    'SingularityAudioProcessorEditor': 'ChipTuneAudioProcessorEditor',
}

def rename_in_file(filepath, old_name, new_name):
    """Replace all occurrences of old_name with new_name in a file"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Replace all occurrences
        new_content = content.replace(old_name, new_name)
        
        if new_content != content:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(new_content)
            return True
    except Exception as e:
        print(f"Error processing {filepath}: {e}")
    return False

def process_directory(directory):
    """Process all files in directory"""
    for root, dirs, files in os.walk(directory):
        # Skip .git and build directories
        dirs[:] = [d for d in dirs if d not in ['.git', 'Builds', 'build', 'JUCE']]
        
        for file in files:
            filepath = os.path.join(root, file)
            
            # Skip binary files
            if file.endswith(('.png', '.jpg', '.jpeg', '.gif', '.ico', '.pdf')):
                continue
            
            # Process text files
            if file.endswith(('.cpp', '.h', '.js', '.html', '.css', '.jucer', '.txt', '.md', '.cmake', 'CMakeLists.txt')):
                for old_name, new_name in RENAMES.items():
                    if rename_in_file(filepath, old_name, new_name):
                        print(f"Renamed {old_name} -> {new_name} in {filepath}")

if __name__ == '__main__':
    base_dir = Path(__file__).parent
    
    # Process ByteBeat
    bytebeat_dir = base_dir / 'ByteBeat'
    if bytebeat_dir.exists():
        print(f"Processing {bytebeat_dir}...")
        process_directory(bytebeat_dir)
    
    # Process ChipTune
    chiptune_dir = base_dir / 'ChipTune'
    if chiptune_dir.exists():
        print(f"Processing {chiptune_dir}...")
        process_directory(chiptune_dir)
    
    print("Done!")
