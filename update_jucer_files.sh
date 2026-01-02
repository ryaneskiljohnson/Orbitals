#!/bin/bash

# Script to update all .jucer files to use PluginProcessor/PluginEditor instead of Main/MainComponent

PLUGINS=("Tidal" "Apogee" "Lagrange" "Eclipse" "Ion" "Perihelion" "Retrograde" "Kepler")

for plugin in "${PLUGINS[@]}"; do
    echo "Updating $plugin.jucer..."
    
    jucer_file="$plugin/$plugin.jucer"
    
    if [ -f "$jucer_file" ]; then
        # Update Source group to use PluginProcessor and PluginEditor files
        sed -i.bak 's|<FILE id="Main.cpp" name="Main.cpp" compile="1" resource="0" file="Source/Main.cpp" />|<FILE id="PluginProcessor.cpp" name="PluginProcessor.cpp" compile="1" resource="0" file="Source/PluginProcessor.cpp" />|g' "$jucer_file"
        sed -i.bak 's|<FILE id="MainComponent.h" name="MainComponent.h" compile="0" resource="0" file="Source/MainComponent.h" />|<FILE id="PluginProcessor.h" name="PluginProcessor.h" compile="0" resource="0" file="Source/PluginProcessor.h" />|g' "$jucer_file"
        sed -i.bak 's|<FILE id="MainComponent.cpp" name="MainComponent.cpp" compile="1" resource="0" file="Source/MainComponent.cpp" />|<FILE id="PluginEditor.h" name="PluginEditor.h" compile="0" resource="0" file="Source/PluginEditor.h" />\n      <FILE id="PluginEditor.cpp" name="PluginEditor.cpp" compile="1" resource="0" file="Source/PluginEditor.cpp" />|g' "$jucer_file"
        
        rm -f "$jucer_file.bak"
        echo "✓ Updated $plugin"
    else
        echo "✗ $jucer_file not found"
    fi
done

echo "Done updating .jucer files"
