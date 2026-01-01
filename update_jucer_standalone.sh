#!/bin/bash

# Update all .jucer files to add standalone builds
for jucer in */ *.jucer; do
    if [ -f "$jucer" ]; then
        plugin_name=$(basename "$jucer" .jucer)
        echo "📝 Updating $jucer for standalone..."
        
        # Add juce_audio_devices module if not present
        if ! grep -q 'juce_audio_devices' "$jucer"; then
            # Insert after juce_audio_basics
            sed -i '' '/juce_audio_basics.*useGlobalPath="0"/a\
    <MODULE id="juce_audio_devices" showAllCode="1" useLocalCopy="1" useGlobalPath="0" />
' "$jucer"
        fi
        
        # Add standalone export if not present
        if ! grep -q 'Standalone' "$jucer"; then
            # Find the closing </LINUX_MAKE> tag and add standalone after it
            awk '
            /<\/LINUX_MAKE>/ {
                print
                print "    <XCODE_MAC targetFolder=\"Builds/MacOSX\" targetName=\"'$plugin_name' (Standalone)\">"
                print "      <CONFIGURATIONS>"
                print "        <CONFIGURATION isDebug=\"1\" name=\"Debug\" targetName=\"'$plugin_name' (Standalone)\" osxSDK=\"default\" osxCompatibility=\"10.15 osxLatest\" osxArchitectures=\"arm64;x86_64\" cppLanguageStandard=\"17\" cppLibType=\"0\" optimisation=\"0\" targetFolder=\"Builds/MacOSX/build/Debug\" headerPath=\"\" preprocessorDefs=\"DEBUG=1 JUCE_DEBUG=1 JUCE_WEB_BROWSER=1 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_STANDALONE_APPLICATION=1 JUCE_PROJUCER_VERSION=0x70005\" />"
                print "        <CONFIGURATION isDebug=\"0\" name=\"Release\" targetName=\"'$plugin_name' (Standalone)\" osxSDK=\"default\" osxCompatibility=\"10.15 osxLatest\" osxArchitectures=\"arm64;x86_64\" cppLanguageStandard=\"17\" cppLibType=\"0\" optimisation=\"3\" targetFolder=\"Builds/MacOSX/build/Release\" headerPath=\"\" preprocessorDefs=\"NDEBUG=1 JUCE_WEB_BROWSER=1 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_STANDALONE_APPLICATION=1 JUCE_PROJUCER_VERSION=0x70005\" />"
                print "      </CONFIGURATIONS>"
                print "      <MODULEPATHS>"
                print "        <MODULEPATH id=\"juce_audio_basics\" path=\"../../Synth/JUCE/modules\" />"
                print "        <MODULEPATH id=\"juce_audio_devices\" path=\"../../Synth/JUCE/modules\" />"
                print "        <MODULEPATH id=\"juce_audio_processors\" path=\"../../Synth/JUCE/modules\" />"
                print "        <MODULEPATH id=\"juce_core\" path=\"../../Synth/JUCE/modules\" />"
                print "        <MODULEPATH id=\"juce_data_structures\" path=\"../../Synth/JUCE/modules\" />"
                print "        <MODULEPATH id=\"juce_events\" path=\"../../Synth/JUCE/modules\" />"
                print "        <MODULEPATH id=\"juce_graphics\" path=\"../../Synth/JUCE/modules\" />"
                print "        <MODULEPATH id=\"juce_gui_basics\" path=\"../../Synth/JUCE/modules\" />"
                print "        <MODULEPATH id=\"juce_gui_extra\" path=\"../../Synth/JUCE/modules\" />"
                print "      </MODULEPATHS>"
                print "    </XCODE_MAC>"
                next
            }
            { print }
            ' "$jucer" > "${jucer}.tmp" && mv "${jucer}.tmp" "$jucer"
        fi
        
        echo "✅ Updated $jucer"
    fi
done

echo ""
echo "🎉 All .jucer files updated!"
