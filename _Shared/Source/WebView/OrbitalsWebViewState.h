/*
  @fileoverview Shared runtime state for Orbitals WebView loading.
  @module Orbitals/_Shared/Source/WebView
*/

#pragma once

#include <JuceHeader.h>
#include <atomic>

/**
 * @brief Platform-specific Web UI asset state used by OrbitalsWebViewHost.
 */
struct OrbitalsWebViewState
{
    juce::File webUiResourceDirectory;
    juce::String webUiInjectedIndexHtml;
#if JUCE_WINDOWS
    juce::File webUiWindowsTempBundle;
    juce::File webView2UserDataFolder;
#endif
    std::atomic<uint32_t> navigationGeneration { 0 };
    std::atomic<bool> initialNavigationDispatched { false };
    std::atomic<bool> loadFailureUiShown { false };
    bool fallbackPending = false;
    uint32_t fallbackPendingGeneration = 0;
};

/**
 * @brief Configuration for OrbitalsWebViewHost per plugin.
 */
struct OrbitalsWebViewHostConfig
{
    juce::String pluginDisplayName;
    juce::File devPluginDirectory;
};
