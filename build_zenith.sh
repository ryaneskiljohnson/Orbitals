#!/bin/bash
# Build Zenith Plugin

echo "Building Zenith Plugin..."
echo "========================"
echo ""

# Check if Xcode project exists
if [ -f "Zenith/Builds/MacOSX/Zenith.xcodeproj/project.pbxproj" ]; then
  echo "✓ Xcode project found"
  echo ""
  echo "Building VST3..."
  xcodebuild -project Zenith/Builds/MacOSX/Zenith.xcodeproj \
             -scheme "Zenith - VST3" \
             -configuration Release \
             build 2>&1 | tee zenith_build.log | grep -E "(error|warning|BUILD SUCCEEDED|BUILD FAILED)"
else
  echo "✗ Xcode project not found"
  echo ""
  echo "Please open Zenith.jucer in Projucer and click 'Save Project' first"
  echo "Command: open Zenith/Zenith.jucer"
fi

