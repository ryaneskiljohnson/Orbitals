#!/bin/bash

echo "=== FABRIC STANDALONE TEST ==="
echo ""
echo "Starting Fabric standalone app..."
echo "The app will run in the background."
echo "Console output will be captured to test_output.log"
echo ""

cd Builds/MacOSX

# Kill any existing instance
pkill -f "Fabric.app/Contents/MacOS/Fabric" 2>/dev/null

# Run the app and capture output
./build/Debug/Fabric.app/Contents/MacOS/Fabric > test_output.log 2>&1 &
PID=$!

echo "App started with PID: $PID"
echo "Waiting 5 seconds for app to initialize..."
sleep 5

echo ""
echo "=== CONSOLE OUTPUT (first 100 lines) ==="
head -100 test_output.log

echo ""
echo "=== INSTRUCTIONS ==="
echo "1. The Fabric window should be open"
echo "2. Try moving a knob or slider"
echo "3. Try clicking the settings button"
echo "4. Then run: tail -f Builds/MacOSX/test_output.log"
echo "   to see live output as you interact with controls"
echo ""
echo "To stop the app: kill $PID"
echo "Or: pkill -f Fabric"
