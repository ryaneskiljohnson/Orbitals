#!/bin/bash
# Script to update all plugins with WebBrowserWithCallbacks fix

PLUGINS=(
    "Retrograde"
    "Perihelion"
    "MandelbrotSet/Fabric"
    "MandelbrotSet/Hadron"
    "MandelbrotSet/Observer"
    "MandelbrotSet/Planck"
    "MandelbrotSet/Entanglement"
    "MandelbrotSet/Singularity"
    "MandelbrotSet/Rabbithole"
    "MandelbrotSet/Quarks"
    "MandelbrotSet/Mandelbrot"
)

for plugin in "${PLUGINS[@]}"; do
    echo "Updating $plugin..."
    
    HEADER_FILE="$plugin/Source/PluginEditor.h"
    CPP_FILE="$plugin/Source/PluginEditor.cpp"
    
    if [ ! -f "$HEADER_FILE" ]; then
        echo "  WARNING: $HEADER_FILE not found, skipping..."
        continue
    fi
    
    # Check if already has WebBrowserWithCallbacks
    if grep -q "WebBrowserWithCallbacks" "$HEADER_FILE"; then
        echo "  Already has WebBrowserWithCallbacks in header"
    else
        echo "  Adding WebBrowserWithCallbacks to header..."
        # This will be done manually via search_replace
    fi
    
    if [ ! -f "$CPP_FILE" ]; then
        echo "  WARNING: $CPP_FILE not found, skipping..."
        continue
    fi
    
    # Check if already has the fix
    if grep -q "withKeepPageLoadedWhenBrowserIsHidden" "$CPP_FILE"; then
        echo "  Already has withKeepPageLoadedWhenBrowserIsHidden"
    else
        echo "  Needs update in cpp"
    fi
done

echo "Done checking plugins."
