#!/bin/bash

# Helper function to add standalone to a plugin
add_standalone() {
    folder=$1
    name=$2
    jucer="${folder}/${name}.jucer"
    
    if [ ! -f "$jucer" ]; then
        echo "⚠️  $jucer not found"
        return
    fi
    
    echo "📝 Updating $jucer..."
    
    # Add juce_audio_devices module
    sed -i '' '/<MODULE id="juce_audio_basics"/a\
    <MODULE id="juce_audio_devices" showAllCode="1" useLocalCopy="1" useGlobalPath="0" />
' "$jucer"
    
    # Add juce_audio_devices to XCODE_MAC module paths
    sed -i '' '/<MODULEPATH id="juce_audio_basics" path="..\/..\/Synth\/JUCE\/modules" \/>/a\
        <MODULEPATH id="juce_audio_devices" path="../../Synth/JUCE/modules" />
' "$jucer"
    
    # Add standalone export before </EXPORTFORMATS>
    standalone_block="    <XCODE_MAC targetFolder=\"Builds/MacOSX\" targetName=\"${name} (Standalone)\">
      <CONFIGURATIONS>
        <CONFIGURATION isDebug=\"1\" name=\"Debug\" targetName=\"${name} (Standalone)\" osxSDK=\"default\" osxCompatibility=\"10.15 osxLatest\" osxArchitectures=\"arm64;x86_64\" cppLanguageStandard=\"17\" cppLibType=\"0\" optimisation=\"0\" targetFolder=\"Builds/MacOSX/build/Debug\" headerPath=\"\" preprocessorDefs=\"DEBUG=1 JUCE_DEBUG=1 JUCE_WEB_BROWSER=1 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_STANDALONE_APPLICATION=1 JUCE_PROJUCER_VERSION=0x70005\" />
        <CONFIGURATION isDebug=\"0\" name=\"Release\" targetName=\"${name} (Standalone)\" osxSDK=\"default\" osxCompatibility=\"10.15 osxLatest\" osxArchitectures=\"arm64;x86_64\" cppLanguageStandard=\"17\" cppLibType=\"0\" optimisation=\"3\" targetFolder=\"Builds/MacOSX/build/Release\" headerPath=\"\" preprocessorDefs=\"NDEBUG=1 JUCE_WEB_BROWSER=1 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_STANDALONE_APPLICATION=1 JUCE_PROJUCER_VERSION=0x70005\" />
      </CONFIGURATIONS>
      <MODULEPATHS>
        <MODULEPATH id=\"juce_audio_basics\" path=\"../../Synth/JUCE/modules\" />
        <MODULEPATH id=\"juce_audio_devices\" path=\"../../Synth/JUCE/modules\" />
        <MODULEPATH id=\"juce_audio_processors\" path=\"../../Synth/JUCE/modules\" />
        <MODULEPATH id=\"juce_core\" path=\"../../Synth/JUCE/modules\" />
        <MODULEPATH id=\"juce_data_structures\" path=\"../../Synth/JUCE/modules\" />
        <MODULEPATH id=\"juce_events\" path=\"../../Synth/JUCE/modules\" />
        <MODULEPATH id=\"juce_graphics\" path=\"../../Synth/JUCE/modules\" />
        <MODULEPATH id=\"juce_gui_basics\" path=\"../../Synth/JUCE/modules\" />
        <MODULEPATH id=\"juce_gui_extra\" path=\"../../Synth/JUCE/modules\" />
      </MODULEPATHS>
    </XCODE_MAC>"
    
    # Only add if not already present
    if ! grep -q "(Standalone)" "$jucer"; then
        sed -i '' "s|  </EXPORTFORMATS>|${standalone_block}\n  </EXPORTFORMATS>|" "$jucer"
    fi
    
    # Add to global MODULEPATHS
    sed -i '' '/<MODULEPATH id="juce_audio_basics" path="..\/..\/Synth\/JUCE\/modules" \/>/a\
    <MODULEPATH id="juce_audio_devices" path="../../Synth/JUCE/modules" />
' "$jucer"
    
    echo "✅ Updated $jucer"
}

# Update remaining plugins
add_standalone "03_Apogee" "Apogee"
add_standalone "04_Retrograde" "Retrograde"
add_standalone "05_Tidal" "Tidal"
add_standalone "06_Ion" "Ion"
add_standalone "07_Eclipse" "Eclipse"
add_standalone "08_Kepler" "Kepler"

echo ""
echo "🎉 All plugins updated with standalone support!"
