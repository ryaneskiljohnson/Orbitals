/*
  @fileoverview Shared CymaSynth-style WebView host for Orbitals plugin editors.
  @module Orbitals/_Shared/Source/WebView
*/

#pragma once

#include <JuceHeader.h>
#include <functional>
#include <optional>

#include "OrbitalsWebBrowserCallbacks.h"
#include "OrbitalsWebViewState.h"
#include "PluginModulePath.h"

/**
 * @brief Reusable WebView lifecycle, loading, and bridge host for Orbitals plugins.
 * @note Simplified CymaSynth architecture without synth bootstrap/hydration gates.
 */
class OrbitalsWebViewHost
{
public:
    OrbitalsWebViewHost() = default;

    /**
     * @brief Initializes WebView on the editor component.
     * @param editor Parent AudioProcessorEditor.
     * @param config Plugin-specific paths and display name.
     * @param onJavaScriptMessage Callback for JS messages (already on message thread).
     * @returns true when WebView was created successfully.
     */
    bool initialize(juce::AudioProcessorEditor& editor,
                    const OrbitalsWebViewHostConfig& config,
                    std::function<void(const juce::var&)> onJavaScriptMessage);

    /** @brief Releases WebView callbacks before destruction. */
    void shutdown();

    /**
     * @brief Updates WebView bounds to fill the editor.
     * @param editor Parent component.
     */
    void resized(juce::Component& editor);

    /**
     * @brief Loads dark placeholder, then dispatches auth or main UI once.
     * @param authorized Whether license check passed.
     */
    void dispatchInitialNavigation(bool authorized);

    /** @brief Loads the main plugin UI (index.html). */
    void loadWebUI();

    /** @brief Loads auth or main UI after license state changes. */
    void reloadSurface(bool authorized);

    /**
     * @brief Loads the auth screen using UI/plugin-background.png and UI/nnaudio-logo.png.
     */
    void loadAuthScreen();

    /**
     * @brief Called from WebView pageFinishedLoading (via async dispatch).
     * @param url Loaded URL.
     * @param editor Parent component for repaint.
     */
    void handlePageFinished(const juce::String& url, juce::Component& editor);

    /** @brief Navigates to a minimal dark data: placeholder. */
    void navigateToDarkPlaceholder();

    /**
     * @brief Evaluates JavaScript when WebView is visible.
     * @param script JavaScript source.
     */
    void evaluateJavascript(const juce::String& script) const;

    /** @returns Raw WebView pointer or nullptr. */
    OrbitalsWebBrowserCallbacks* getWebView() const { return webView.get(); }

    /** @returns true when WebView initialized successfully. */
    bool isInitialized() const { return webViewInitialized; }

    /** @returns true when WebView is visible. */
    bool isVisible() const;

    /** @returns Reference to optional error label (may be null until init failure). */
    juce::Label* getErrorLabel() const { return errorLabel.get(); }

    /**
     * @brief Unwraps JUCE native-integration message envelope and forwards to handler.
     * @param message Raw message from withEventListener.
     */
    static void dispatchJavaScriptMessage(const juce::var& message,
                                          const std::function<void(const juce::var&)>& handler);

    /**
     * @brief Resolves index.html for dev or bundled plugin.
     * @param config Plugin configuration.
     * @returns Path to index.html or invalid file if not found.
     */
    static juce::File resolveIndexHtmlFile(const OrbitalsWebViewHostConfig& config);

    /**
     * @brief Resolves the directory containing Web UI assets (dev UI/, bundle Resources/, or Resources/web/).
     * @param config Plugin configuration.
     * @returns UI asset root directory.
     */
    static juce::File resolveUiDirectory(const OrbitalsWebViewHostConfig& config);

    /**
     * @brief Serves a resource URL for withResourceProvider (non-Windows).
     * @param url Request URL from embedded browser.
     * @param state Mutable WebView state holding injected HTML and resource root.
     * @returns Resource bytes and MIME type, or nullopt.
     */
    static std::optional<juce::WebBrowserComponent::Resource> getResourceForUrl(
        const juce::String& url,
        OrbitalsWebViewState& state);

private:
    enum class Surface { none, webUi, auth };

    void beginNewNavigation();
    void scheduleLoadFallback(uint32_t generation, juce::Component& editor);
    void showWebViewAfterLoad(uint32_t generation, juce::Component& editor);
    bool loadHtmlContent(const juce::File& htmlFile, Surface surface);
    juce::String prepareHtmlFromFile(const juce::File& htmlFile) const;
    juce::String buildLoadUrl(const juce::File& tempIndexFile) const;
    void navigateToInjectedHtml(const juce::String& htmlContent, Surface surface, const juce::File& webDir);
    void cleanupTransientWebState();
#if JUCE_WINDOWS
    static juce::File createUniqueWebView2UserDataFolder();
    static bool copyWebFolderToTempBundle(const juce::File& sourceWebDir, const juce::File& destBundleDir);
#endif

    OrbitalsWebViewHostConfig config;
    OrbitalsWebViewState state;
    std::function<void(const juce::var&)> jsMessageHandler;
    juce::Component::SafePointer<juce::Component> editorComponent;
    std::unique_ptr<OrbitalsWebBrowserCallbacks> webView;
    std::unique_ptr<juce::Label> errorLabel;
    bool webViewInitialized = false;
    Surface lastSurface = Surface::none;
};
