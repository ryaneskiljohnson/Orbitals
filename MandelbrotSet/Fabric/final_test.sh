#!/bin/bash

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║     FABRIC REVERB PLUGIN - COMPREHENSIVE VERIFICATION TEST     ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""
echo "Testing all audio functionality..."
echo ""

cd /Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Fabric

# Run the app and capture all relevant output
(./Builds/MacOSX/build/Debug/Fabric.app/Contents/MacOS/Fabric 2>&1 & 
APP_PID=$!
sleep 30
kill $APP_PID 2>/dev/null
wait $APP_PID) 2>&1 | tee /tmp/fabric_test_output.log | grep -E "(===|✅|Test mode|AUDIO DEVICE|Sample rate|Device is playing|Audio INPUT|Audio after reverb|DSP Parameters|Mapped Values|WetLevel|DryLevel)" | head -50

echo ""
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║                     VERIFICATION CHECKLIST                      ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""
echo "□ Constructor called"
echo "□ PrepareToPlay called with sample rate 44100 Hz"
echo "□ Audio device: MacBook Pro Speakers"
echo "□ Device is playing: YES"
echo "□ Test signal generated (220Hz tone)"
echo "□ Audio INPUT shows ~0.5 amplitude"
echo "□ Audio after reverb shows 0.4-0.9 amplitude"
echo "□ DSP parameters change during test"
echo "□ Mapped values (WetLevel, DryLevel) are calculated"
echo ""
echo "Full log saved to: /tmp/fabric_test_output.log"
echo ""
