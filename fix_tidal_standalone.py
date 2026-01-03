#!/usr/bin/env python3
"""
Add standalone wrapper to Tidal's Standalone target after Projucer regeneration.
Run this after regenerating in Projucer.
"""

import re
import sys
import os

project_file = "Tidal/Builds/MacOSX/Tidal.xcodeproj/project.pbxproj"

if not os.path.exists(project_file):
    print("ERROR: Xcode project not found. Regenerate in Projucer first.")
    sys.exit(1)

with open(project_file, 'r') as f:
    content = f.read()

# Check if already added correctly
if 'include_juce_audio_plugin_client_Standalone' in content:
    build_file_match = re.search(r'(\w+) /\* include_juce_audio_plugin_client_Standalone\.cpp \*/', content)
    if build_file_match:
        build_file_id = build_file_match.group(1)
        # Check if in Standalone target
        standalone_match = re.search(r'Tidal.*?Standalone.*?buildPhases = \(([^)]+)\);', content, re.DOTALL)
        if standalone_match:
            phase_ids = [p.strip() for p in standalone_match.group(1).split(',') if p.strip()]
            for pid in phase_ids:
                phase_def = re.search(rf'{re.escape(pid)} = {{.*?isa = PBXSourcesBuildPhase;.*?files = \(([^)]+)\);', content, re.DOTALL)
                if phase_def and build_file_id in phase_def.group(1):
                    print("✓ Standalone wrapper already in Standalone target")
                    sys.exit(0)

# Find or create file reference
file_ref_id = None
file_ref_match = re.search(r'(\w+) /\* include_juce_audio_plugin_client_Standalone\.cpp \*/.*?path = "?([^";]+)"?;', content)
if file_ref_match:
    file_ref_id = file_ref_match.group(1)
    print(f"Found file reference: {file_ref_id}")
else:
    # Create file reference - use Cymasphere's ID pattern
    file_ref_id = "079C1C3EDEADAA0B139A6E43"
    file_ref_section = re.search(r'(/\* Begin PBXFileReference section \*/.*?/\* End PBXFileReference section \*/)', content, re.DOTALL)
    if file_ref_section:
        new_file_ref = f'\t\t{file_ref_id} /* include_juce_audio_plugin_client_Standalone.cpp */ /* include_juce_audio_plugin_client_Standalone.cpp */ = {{isa = PBXFileReference; lastKnownFileType = sourcecode.cpp.cpp; name = include_juce_audio_plugin_client_Standalone.cpp; path = ../../JuceLibraryCode/include_juce_audio_plugin_client_Standalone.cpp; sourceTree = SOURCE_ROOT; }};\n'
        file_ref_content = file_ref_section.group(1)
        file_ref_content = file_ref_content.replace('/* End PBXFileReference section */', new_file_ref + '\t\t/* End PBXFileReference section */')
        content = content.replace(file_ref_section.group(1), file_ref_content)
        print(f"✓ Created file reference: {file_ref_id}")

# Find or create build file
build_file_id = None
build_file_match = re.search(rf'(\w+) = {{.*?fileRef = {re.escape(file_ref_id)};', content)
if build_file_match:
    build_file_id = build_file_match.group(1)
    print(f"Found build file: {build_file_id}")
else:
    # Create build file - use Cymasphere's ID
    build_file_id = "6C1653772312D732E3796A29"
    build_file_section = re.search(r'(/\* Begin PBXBuildFile section \*/.*?/\* End PBXBuildFile section \*/)', content, re.DOTALL)
    if build_file_section:
        new_build_file = f'\t\t{build_file_id} /* include_juce_audio_plugin_client_Standalone.cpp */ = {{isa = PBXBuildFile; fileRef = {file_ref_id}; }};\n'
        build_file_content = build_file_section.group(1)
        build_file_content = build_file_content.replace('/* End PBXBuildFile section */', new_build_file + '\t\t/* End PBXBuildFile section */')
        content = content.replace(build_file_section.group(1), build_file_content)
        print(f"✓ Created build file: {build_file_id}")

# Add to Standalone target sources phase
standalone_match = re.search(r'Tidal.*?Standalone.*?=.*?{(.*?name = "Tidal - Standalone Plugin";.*?)}', content, re.DOTALL)
if standalone_match:
    target_block = standalone_match.group(1)
    phases = re.search(r'buildPhases = \(([^)]+)\);', target_block, re.DOTALL)
    if phases:
        phase_ids = [p.strip() for p in phases.group(1).split(',') if p.strip()]
        
        # Find sources phase
        sources_phase_id = None
        for pid in phase_ids:
            phase_def = re.search(rf'{re.escape(pid)} = {{.*?isa = PBXSourcesBuildPhase;', content)
            if phase_def:
                sources_phase_id = pid
                break
        
        if sources_phase_id:
            phase_match = re.search(rf'({re.escape(sources_phase_id)} = {{.*?isa = PBXSourcesBuildPhase;.*?files = \(([^)]+)\);.*?}})', content, re.DOTALL)
            if phase_match:
                phase_block = phase_match.group(1)
                current_files = phase_match.group(2)
                
                if build_file_id not in current_files:
                    new_entry = f'\t\t\t\t{build_file_id} /* include_juce_audio_plugin_client_Standalone.cpp */,\n'
                    if current_files.strip():
                        updated_files = current_files.rstrip() + ',\n' + new_entry
                    else:
                        updated_files = new_entry
                    
                    updated_phase = phase_block.replace(f'files = ({current_files});', f'files = (\n{updated_files}\t\t\t);')
                    content = content.replace(phase_block, updated_phase)
                    print(f"✓ Added to Standalone target sources phase: {sources_phase_id}")
                else:
                    print("✓ Already in Standalone target")
            else:
                print("ERROR: Could not find files section")
        else:
            print("ERROR: Could not find sources phase")

# Write back
with open(project_file, 'w') as f:
    f.write(content)

print("\n✓ Fixed Xcode project - ready to build")
