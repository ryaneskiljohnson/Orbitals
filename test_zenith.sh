#!/bin/bash
# Test Zenith Plugin Build

echo "Testing Zenith Plugin..."
echo "========================"
echo ""

# Check required files
echo "Checking files..."
FILES=(
  "Zenith/Zenith.jucer"
  "Zenith/CMakeLists.txt"
  "Zenith/Source/PluginProcessor.cpp"
  "Zenith/Source/PluginProcessor.h"
  "Zenith/Source/PluginEditor.cpp"
  "Zenith/Source/PluginEditor.h"
  "Zenith/UI/index.html"
  "Zenith/UI/app.js"
  "Zenith/UI/styles.css"
  "Zenith/product_id.txt"
)

for file in "${FILES[@]}"; do
  if [ -f "$file" ]; then
    echo "  ✓ $file"
  else
    echo "  ✗ $file (missing)"
  fi
done

echo ""
echo "Checking parameters in PluginProcessor.cpp..."
grep -E "PARAM_EXPANSION|PARAM_THRESHOLD|PARAM_CEILING|PARAM_CURVE" Zenith/Source/PluginProcessor.cpp | head -5

echo ""
echo "Checking UI controls in index.html..."
grep -E "expansionSlider|thresholdSlider|ceilingSlider|curveSlider" Zenith/UI/index.html | wc -l | xargs echo "  Found control references:"

echo ""
echo "Checking product ID..."
cat Zenith/product_id.txt

echo ""
echo "Next steps to test:"
echo "1. Open Zenith.jucer in Projucer"
echo "2. Click 'Save Project' to regenerate build files"
echo "3. Open in Xcode and build"
echo "4. Test in a DAW with MIDI input"

