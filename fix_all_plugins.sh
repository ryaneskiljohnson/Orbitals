#!/bin/bash
# Fix all Orbitals plugins to match Tidal's working structure

set -e

PLUGINS=("Apogee" "Kepler" "Lagrange" "Eclipse" "Ion" "Perihelion" "Retrograde")
TIDAL_JL="Tidal/JuceLibraryCode"

echo "Applying Tidal fixes to all Orbitals plugins..."

for plugin in "${PLUGINS[@]}"; do
    echo ""
    echo "=== Processing $plugin ==="
    
    # Copy plugin_client files
    if [ -d "$plugin/JuceLibraryCode" ] && [ -d "$TIDAL_JL" ]; then
        plugin_client_files=$(find "$TIDAL_JL" -name "*plugin_client*" -type f)
        for file in $plugin_client_files; do
            filename=$(basename "$file")
            cp "$file" "$plugin/JuceLibraryCode/$filename"
        done
        echo "✓ Copied plugin_client files to $plugin"
    fi
done

echo ""
echo "✓ All plugins updated!"
echo "Next: Update .jucer files and source code manually (see Python script)"
