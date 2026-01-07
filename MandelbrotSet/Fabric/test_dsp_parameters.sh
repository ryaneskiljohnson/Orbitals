#!/bin/bash

echo "=== Fabric DSP Parameter Verification Test ==="
echo ""
echo "This test verifies that all DSP parameters affect the audio output."
echo "The app will run for 30 seconds generating a test tone through reverb."
echo "Watch for 'DSP Parameters' and 'Audio after reverb' output."
echo ""
echo "Expected behavior:"
echo "  - Input signal: 0.5 amplitude"
echo "  - Output should vary as you move controls in the UI"
echo "  - Wet/Dry: 0% should output ~0.375 (75% dry), 100% should be higher (more reverb)"
echo "  - Size affects reverb tail length"
echo "  - Damping affects high frequencies"
echo ""
echo "Starting app..."
echo ""

cd /Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Fabric
./Builds/MacOSX/build/Debug/Fabric.app/Contents/MacOS/Fabric 2>&1 | grep -E "(DSP Parameters|Audio after reverb|Mapped Values)" &

APP_PID=$!
sleep 30
kill $APP_PID 2>/dev/null

echo ""
echo "=== Test complete ==="
