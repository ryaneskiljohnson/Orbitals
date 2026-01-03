#!/bin/bash
# Complete fix for Tidal standalone - run after regenerating in Projucer

cd "$(dirname "$0")"

echo "Checking if project exists..."
if [ ! -f "Tidal/Builds/MacOSX/Tidal.xcodeproj/project.pbxproj" ]; then
    echo "ERROR: Xcode project not found!"
    echo "1. Open Tidal.jucer in Projucer"
    echo "2. Click 'Save and Open in IDE'"
    echo "3. Then run this script again: ./fix_tidal_complete.sh"
    exit 1
fi

echo "Running fix script..."
python3 fix_tidal_standalone.py

echo ""
echo "Building standalone target..."
xcodebuild -project Tidal/Builds/MacOSX/Tidal.xcodeproj \
           -target "Tidal - Standalone Plugin" \
           -configuration Debug \
           clean build 2>&1 | tail -20

echo ""
echo "Checking for executable..."
if [ -f "Tidal/Builds/MacOSX/build/Debug/Tidal.app/Contents/MacOS/Tidal" ]; then
    echo "✓ SUCCESS! Executable created!"
    ls -lh "Tidal/Builds/MacOSX/build/Debug/Tidal.app/Contents/MacOS/Tidal"
else
    echo "✗ Executable still missing - check build errors above"
fi
