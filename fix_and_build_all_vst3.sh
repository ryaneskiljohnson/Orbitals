#!/bin/bash

# Script to fix header paths and build all plugins as VST3

set -e

ORBITAL_PLUGINS=("Tidal" "Apogee" "Lagrange" "Eclipse" "Ion" "Perihelion" "Retrograde" "Kepler" "Zenith")
MANDELBROT_PLUGINS=("Singularity" "Entanglement" "Hadron" "Mandelbrot" "Observer" "Planck" "Quarks" "Rabbithole")

BASE_DIR="/Users/rjmacbookpro/Development/Orbitals"

fix_project_file() {
    local project_file="$1"
    local plugin_name="$2"
    
    echo "Fixing $plugin_name..."
    
    # Remove global JUCE path from HEADER_SEARCH_PATHS
    if grep -q '\$(HOME)/JUCE/modules/juce_audio_processors_headless/format_types/VST3_SDK' "$project_file"; then
        # Remove the line with global JUCE path
        sed -i '' '/\$(HOME)\/JUCE\/modules\/juce_audio_processors_headless\/format_types\/VST3_SDK/d' "$project_file"
        echo "  ✓ Removed global JUCE path from HEADER_SEARCH_PATHS"
    fi
    
    # Remove global JUCE path from MTL_HEADER_SEARCH_PATHS
    if grep -q '\$(HOME)/JUCE/modules/juce_audio_processors_headless/format_types/VST3_SDK' "$project_file"; then
        sed -i '' 's/\$(HOME)\/JUCE\/modules\/juce_audio_processors_headless\/format_types\/VST3_SDK //g' "$project_file"
        echo "  ✓ Removed global JUCE path from MTL_HEADER_SEARCH_PATHS"
    fi
    
    # Add local VST3 SDK path if not present
    if ! grep -q 'JuceLibraryCode/modules/juce_audio_processors/format_types/VST3_SDK' "$project_file"; then
        # Add after JuceLibraryCode/modules line
        sed -i '' 's|"$(SRCROOT)/../../JuceLibraryCode/modules",|"$(SRCROOT)/../../JuceLibraryCode/modules",\n\t\t\t\t\t"$(SRCROOT)/../../JuceLibraryCode/modules/juce_audio_processors/format_types/VST3_SDK",|g' "$project_file"
        echo "  ✓ Added local VST3 SDK path to HEADER_SEARCH_PATHS"
    fi
    
    # Add to MTL_HEADER_SEARCH_PATHS if not present
    if ! grep -q 'JuceLibraryCode/modules/juce_audio_processors/format_types/VST3_SDK' "$project_file" || ! grep -q 'MTL_HEADER_SEARCH_PATHS.*VST3_SDK' "$project_file"; then
        sed -i '' 's|$(SRCROOT)/../../JuceLibraryCode/modules |$(SRCROOT)/../../JuceLibraryCode/modules $(SRCROOT)/../../JuceLibraryCode/modules/juce_audio_processors/format_types/VST3_SDK |g' "$project_file"
        echo "  ✓ Added local VST3 SDK path to MTL_HEADER_SEARCH_PATHS"
    fi
}

build_vst3() {
    local plugin_dir="$1"
    local plugin_name="$2"
    local project_path="$plugin_dir/Builds/MacOSX/${plugin_name}.xcodeproj"
    
    if [ ! -d "$project_path" ]; then
        echo "  ⚠ Project not found: $project_path"
        return 1
    fi
    
    echo "Building $plugin_name VST3..."
    cd "$plugin_dir/Builds/MacOSX"
    
    if xcodebuild -project "${plugin_name}.xcodeproj" -scheme "${plugin_name} - VST3" -configuration Release clean build > /tmp/${plugin_name}_build.log 2>&1; then
        echo "  ✅ $plugin_name VST3 built successfully"
        return 0
    else
        echo "  ❌ $plugin_name VST3 build failed"
        tail -20 /tmp/${plugin_name}_build.log
        return 1
    fi
}

# Fix and build Orbital plugins
echo "=========================================="
echo "Fixing and Building Orbital Plugins"
echo "=========================================="

for plugin in "${ORBITAL_PLUGINS[@]}"; do
    plugin_dir="$BASE_DIR/$plugin"
    project_file="$plugin_dir/Builds/MacOSX/${plugin}.xcodeproj/project.pbxproj"
    
    if [ -f "$project_file" ]; then
        fix_project_file "$project_file" "$plugin"
        build_vst3 "$plugin_dir" "$plugin"
    else
        echo "⚠ Skipping $plugin - project file not found"
    fi
    echo ""
done

# Fix and build Mandelbrot plugins
echo "=========================================="
echo "Fixing and Building Mandelbrot Plugins"
echo "=========================================="

for plugin in "${MANDELBROT_PLUGINS[@]}"; do
    plugin_dir="$BASE_DIR/MandelbrotSet/$plugin"
    project_file="$plugin_dir/Builds/MacOSX/${plugin}.xcodeproj/project.pbxproj"
    
    if [ -f "$project_file" ]; then
        fix_project_file "$project_file" "$plugin"
        build_vst3 "$plugin_dir" "$plugin"
    else
        echo "⚠ Skipping $plugin - project file not found"
    fi
    echo ""
done

echo "=========================================="
echo "Build Summary"
echo "=========================================="
echo "Checking built VST3 plugins..."

for plugin in "${ORBITAL_PLUGINS[@]}"; do
    vst3_path="$BASE_DIR/$plugin/Builds/MacOSX/build/Release/${plugin}.vst3"
    if [ -d "$vst3_path" ]; then
        echo "  ✅ $plugin.vst3"
    else
        echo "  ❌ $plugin.vst3 (not found)"
    fi
done

for plugin in "${MANDELBROT_PLUGINS[@]}"; do
    vst3_path="$BASE_DIR/MandelbrotSet/$plugin/Builds/MacOSX/build/Release/${plugin}.vst3"
    if [ -d "$vst3_path" ]; then
        echo "  ✅ $plugin.vst3"
    else
        echo "  ❌ $plugin.vst3 (not found)"
    fi
done
