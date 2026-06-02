/*
  @fileoverview Resolves the loaded plugin module binary path for Web UI resource discovery.
  @module Orbitals/_Shared/Source/WebView
  @note Ported from CymaSynth PluginModulePath.h — avoids using the host DAW executable in AU/VST3.
*/

#pragma once

#include <JuceHeader.h>

#if JUCE_MAC
#include <dlfcn.h>
#endif

#if JUCE_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace OrbitalsPluginPaths
{
/**
 * @brief Returns the on-disk path of this plugin module.
 * @returns Plugin binary in hosted builds; current executable in standalone.
 */
inline juce::File getModuleBinaryFileForResourceDiscovery()
{
#if JUCE_STANDALONE_APPLICATION
    return juce::File::getSpecialLocation(juce::File::currentExecutableFile);
#elif JUCE_MAC
    Dl_info info{};
    if (dladdr(reinterpret_cast<const void*>(&getModuleBinaryFileForResourceDiscovery), &info) != 0
        && info.dli_fname != nullptr && *info.dli_fname != '\0')
    {
        return juce::File(juce::String::fromUTF8(info.dli_fname));
    }
#elif JUCE_WINDOWS
    HMODULE moduleHandle = nullptr;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCWSTR>(&getModuleBinaryFileForResourceDiscovery),
                           &moduleHandle) != 0
        && moduleHandle != nullptr)
    {
        wchar_t path[MAX_PATH]{};
        const DWORD len = GetModuleFileNameW(moduleHandle, path, MAX_PATH);
        if (len > 0 && len < MAX_PATH)
            return juce::File(juce::String(path));
    }
#endif

    return juce::File::getSpecialLocation(juce::File::currentExecutableFile);
}

#if JUCE_MAC
/**
 * @brief Resolves Contents/Resources for the bundle containing exe.
 * @param exe Mach-O path inside the plugin bundle.
 * @param outResourcesDir Set to Contents/Resources when found.
 * @returns true when outResourcesDir is a directory.
 */
inline bool tryResolveMacBundleResourcesDirFromExecutable(const juce::File& exe, juce::File& outResourcesDir)
{
    juce::File dir = exe.isDirectory() ? exe : exe.getParentDirectory();
    const juce::String exeFull = exe.getFullPathName();
    constexpr int kMaxAncestors = 32;
    const auto sep = juce::File::getSeparatorString();

    for (int i = 0; i < kMaxAncestors && dir.exists(); ++i)
    {
        const auto contents = dir.getChildFile("Contents");
        const auto resources = contents.getChildFile("Resources");

        if (contents.isDirectory() && resources.isDirectory())
        {
            const auto contentsFull = contents.getFullPathName();
            const bool underContents = exeFull.startsWithIgnoreCase(contentsFull + sep)
                                       || exeFull.equalsIgnoreCase(contentsFull);
            if (underContents)
            {
                outResourcesDir = resources;
                return true;
            }
        }

        auto parent = dir.getParentDirectory();
        if (parent.getFullPathName() == dir.getFullPathName())
            break;

        dir = parent;
    }

    return false;
}
#endif
} // namespace OrbitalsPluginPaths
