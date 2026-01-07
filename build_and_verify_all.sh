#!/bin/bash

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║  BUILD AND VERIFY ALL PLUGINS - FABRIC ARCHITECTURE MIGRATION  ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

LOG_FILE="/tmp/plugin_build_results.log"
> "$LOG_FILE"  # Clear log file

SUCCESS_COUNT=0
FAIL_COUNT=0
TOTAL_COUNT=0

build_plugin() {
    local PLUGIN_NAME=$1
    local PLUGIN_PATH=$2
    
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "🔨 Building: $PLUGIN_NAME"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    cd "$PLUGIN_PATH/Builds/MacOSX" || return 1
    
    # Build standalone only for speed
    xcodebuild -project "$PLUGIN_NAME.xcodeproj" \
               -scheme "$PLUGIN_NAME - Standalone Plugin" \
               -configuration Debug \
               build > /tmp/build_${PLUGIN_NAME}.log 2>&1
    
    if [ $? -eq 0 ]; then
        echo "  ✅ BUILD SUCCEEDED"
        echo "$PLUGIN_NAME: ✅ SUCCESS" >> "$LOG_FILE"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
        
        # Check if .app was created
        APP_PATH="build/Debug/$PLUGIN_NAME.app"
        if [ -d "$APP_PATH" ]; then
            echo "  ✅ Standalone app created: $APP_PATH"
            echo "  📦 Size: $(du -sh "$APP_PATH" | cut -f1)"
        fi
    else
        echo "  ❌ BUILD FAILED"
        echo "$PLUGIN_NAME: ❌ FAILED" >> "$LOG_FILE"
        echo ""
        echo "  Error details:"
        grep "error:" /tmp/build_${PLUGIN_NAME}.log | head -5 | sed 's/^/    /'
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
    
    TOTAL_COUNT=$((TOTAL_COUNT + 1))
    echo ""
}

echo "📦 MANDELBROT SERIES (Audio FX)"
echo "=================================================================="
echo ""

build_plugin "Fabric" "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Fabric"
build_plugin "Entanglement" "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Entanglement"
build_plugin "Hadron" "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Hadron"
build_plugin "Mandelbrot" "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Mandelbrot"
build_plugin "Observer" "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Observer"
build_plugin "Planck" "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Planck"
build_plugin "Quarks" "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Quarks"
build_plugin "Rabbithole" "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Rabbithole"
build_plugin "Singularity" "/Users/rjmacbookpro/Development/Orbitals/MandelbrotSet/Singularity"

echo ""
echo "📦 ORBITAL SERIES (MIDI FX)"
echo "=================================================================="
echo ""

build_plugin "Apogee" "/Users/rjmacbookpro/Development/Orbitals/Apogee"
build_plugin "Eclipse" "/Users/rjmacbookpro/Development/Orbitals/Eclipse"
build_plugin "Ion" "/Users/rjmacbookpro/Development/Orbitals/Ion"
build_plugin "Kepler" "/Users/rjmacbookpro/Development/Orbitals/Kepler"
build_plugin "Lagrange" "/Users/rjmacbookpro/Development/Orbitals/Lagrange"
build_plugin "Perihelion" "/Users/rjmacbookpro/Development/Orbitals/Perihelion"
build_plugin "Retrograde" "/Users/rjmacbookpro/Development/Orbitals/Retrograde"
build_plugin "Tidal" "/Users/rjmacbookpro/Development/Orbitals/Tidal"
build_plugin "Zenith" "/Users/rjmacbookpro/Development/Orbitals/Zenith"

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║                        BUILD SUMMARY                           ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""
echo "  ✅ Successful: $SUCCESS_COUNT"
echo "  ❌ Failed: $FAIL_COUNT"
echo "  📊 Total: $TOTAL_COUNT"
echo ""
echo "Full results saved to: $LOG_FILE"
echo ""

if [ $FAIL_COUNT -gt 0 ]; then
    echo "⚠️  Some plugins failed to build. Check individual log files in /tmp/"
    echo ""
    cat "$LOG_FILE"
else
    echo "🎉 ALL PLUGINS BUILT SUCCESSFULLY!"
fi
echo ""
