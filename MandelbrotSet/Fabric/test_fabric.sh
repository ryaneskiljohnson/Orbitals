#!/bin/bash

echo "=== Starting Fabric Standalone with Console Logging ==="
echo "=== Press Ctrl+C to stop ==="
echo ""

# Run the app directly to capture console output
/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Fabric/Builds/MacOSX/build/Debug/Fabric.app/Contents/MacOS/Fabric 2>&1

echo ""
echo "=== App closed ==="
