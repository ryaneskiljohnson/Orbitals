#!/bin/bash
# @fileoverview Run WebView migration checks and build all Orbitals plugins (Mac standalone smoke test).
# @module Orbitals/build_and_verify_all

set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
LOG_FILE="/tmp/plugin_build_results.log"

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║     BUILD AND VERIFY ALL PLUGINS - CymaSynth WebView Migration ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

echo "🔍 Running WebView migration verify..."
if ! python3 "$ROOT/tools/apply_cymasynth_webview_pattern.py" verify; then
    echo "❌ WebView migration verify failed. Fix issues before building."
    exit 1
fi
echo ""

echo "🔧 Patching Xcode projects with OrbitalsWebViewHost sources..."
python3 "$ROOT/tools/patch_xcode_webview_host.py"
echo ""

> "$LOG_FILE"

SUCCESS_COUNT=0
FAIL_COUNT=0
TOTAL_COUNT=0

build_plugin() {
    local PLUGIN_NAME=$1
    local PLUGIN_PATH=$2

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "🔨 Building: $PLUGIN_NAME"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

    if [ ! -d "$PLUGIN_PATH/Builds/MacOSX" ]; then
        echo "  ⚠️  Skipping (no Builds/MacOSX — regenerate from .jucer first)"
        echo "$PLUGIN_NAME: ⚠️ SKIPPED (no Xcode project)" >> "$LOG_FILE"
        TOTAL_COUNT=$((TOTAL_COUNT + 1))
        FAIL_COUNT=$((FAIL_COUNT + 1))
        echo ""
        return 1
    fi

    cd "$PLUGIN_PATH/Builds/MacOSX" || return 1

    xcodebuild -project "$PLUGIN_NAME.xcodeproj" \
               -scheme "$PLUGIN_NAME - Standalone Plugin" \
               -configuration Debug \
               build > "/tmp/build_${PLUGIN_NAME}.log" 2>&1

    if [ $? -eq 0 ]; then
        echo "  ✅ BUILD SUCCEEDED"
        echo "$PLUGIN_NAME: ✅ SUCCESS" >> "$LOG_FILE"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))

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
        grep "error:" "/tmp/build_${PLUGIN_NAME}.log" | head -5 | sed 's/^/    /'
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi

    TOTAL_COUNT=$((TOTAL_COUNT + 1))
    echo ""
}

echo "🧪 Pilot smoke builds (Fabric, Tidal)"
echo "=================================================================="
build_plugin "Fabric" "$ROOT/MandelbrotSet/Fabric"
build_plugin "Tidal" "$ROOT/Tidal"
echo ""

echo "📦 MANDELBROT SERIES (Audio FX)"
echo "=================================================================="
build_plugin "Entanglement" "$ROOT/MandelbrotSet/Entanglement"
build_plugin "Hadron" "$ROOT/MandelbrotSet/Hadron"
build_plugin "Mandelbrot" "$ROOT/MandelbrotSet/Mandelbrot"
build_plugin "Observer" "$ROOT/MandelbrotSet/Observer"
build_plugin "Planck" "$ROOT/MandelbrotSet/Planck"
build_plugin "Quarks" "$ROOT/MandelbrotSet/Quarks"
build_plugin "Rabbithole" "$ROOT/MandelbrotSet/Rabbithole"
build_plugin "Singularity" "$ROOT/MandelbrotSet/Singularity"
echo ""

echo "📦 ORBITAL SERIES (MIDI FX)"
echo "=================================================================="
build_plugin "Apogee" "$ROOT/Apogee"
build_plugin "Eclipse" "$ROOT/Eclipse"
build_plugin "Ion" "$ROOT/Ion"
build_plugin "Kepler" "$ROOT/Kepler"
build_plugin "Lagrange" "$ROOT/Lagrange"
build_plugin "Perihelion" "$ROOT/Perihelion"
build_plugin "Retrograde" "$ROOT/Retrograde"
build_plugin "Zenith" "$ROOT/Zenith"
echo ""

echo "📦 8-BIT KIT (Game MIDI Generators + Audio FX)"
echo "=================================================================="
build_plugin "ByteBeat" "$ROOT/8-Bit Kit/ByteBeat"
build_plugin "ChipTune" "$ROOT/8-Bit Kit/ChipTune"
build_plugin "PixelPulse" "$ROOT/8-Bit Kit/PixelPulse"
echo ""

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
    exit 1
else
    echo "🎉 ALL PLUGINS BUILT SUCCESSFULLY! ($SUCCESS_COUNT/$TOTAL_COUNT)"
fi
