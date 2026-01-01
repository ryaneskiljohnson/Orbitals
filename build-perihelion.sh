#!/bin/bash

# Build script for Perihelion plugin
# Usage: ./build-perihelion.sh

set -e  # Exit on error

echo "════════════════════════════════════════════════════════════════════"
echo "           Building PERIHELION - Velocity Sculptor"
echo "════════════════════════════════════════════════════════════════════"
echo ""

PLUGIN_DIR="/Users/rjmacbookpro/Development/Orbitals/01_Perihelion"
cd "$PLUGIN_DIR"

echo "📁 Plugin directory: $PLUGIN_DIR"
echo ""

# Check for JUCE
JUCE_PATH="$PLUGIN_DIR/../../Synth/JUCE"
if [ ! -d "$JUCE_PATH" ]; then
    echo "❌ ERROR: JUCE not found at $JUCE_PATH"
    echo "   Please ensure JUCE is in Synth/JUCE directory"
    exit 1
fi
echo "✅ JUCE found at: $JUCE_PATH"
echo ""

# Create build directory
echo "📦 Creating build directory..."
mkdir -p build
cd build
echo ""

# Configure with CMake
echo "⚙️  Configuring with CMake..."
cmake .. || {
    echo "❌ CMake configuration failed!"
    exit 1
}
echo ""

# Build
echo "🔨 Building plugin..."
cmake --build . --config Release || {
    echo "❌ Build failed!"
    exit 1
}
echo ""

# Find built plugin
echo "🔍 Looking for built plugin..."
if [ -d "Perihelion_artefacts/VST3" ]; then
    VST3_PATH=$(find Perihelion_artefacts/VST3 -name "*.vst3" -type d | head -1)
    echo "✅ VST3 built: $VST3_PATH"
fi

if [ -d "Perihelion_artefacts/AU" ]; then
    AU_PATH=$(find Perihelion_artefacts/AU -name "*.component" -type d | head -1)
    echo "✅ AU built: $AU_PATH"
fi

echo ""
echo "════════════════════════════════════════════════════════════════════"
echo "                    ✅ BUILD COMPLETE!"
echo "════════════════════════════════════════════════════════════════════"
echo ""
echo "📋 Next steps:"
echo ""
echo "1. Copy plugin to system location:"
echo "   VST3: cp -R $VST3_PATH ~/Library/Audio/Plug-Ins/VST3/"
echo "   AU:   cp -R $AU_PATH ~/Library/Audio/Plug-Ins/Components/"
echo ""
echo "2. Rescan plugins in your DAW"
echo ""
echo "3. Load Perihelion on a MIDI track!"
echo ""
echo "════════════════════════════════════════════════════════════════════"
