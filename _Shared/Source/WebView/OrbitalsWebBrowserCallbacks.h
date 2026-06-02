/*
  @fileoverview WebBrowserComponent subclass with lifecycle hooks and WebView2 error suppression.
  @module Orbitals/_Shared/Source/WebView
  @note Ported from CymaSynth WebBrowserWithCallbacks.
*/

#pragma once

#include <JuceHeader.h>

/**
 * @brief WebBrowserComponent with page-finished and network-error callbacks.
 */
class OrbitalsWebBrowserCallbacks : public juce::WebBrowserComponent
{
public:
    /**
     * @brief Constructs the WebView with JUCE native-integration options.
     * @param options Preconfigured WebBrowserComponent::Options.
     */
    explicit OrbitalsWebBrowserCallbacks(const juce::WebBrowserComponent::Options& options)
        : juce::WebBrowserComponent(options)
    {
    }

    std::function<void(const juce::String&)> onPageFinishedLoading;
    std::function<void(const juce::String&)> onPageLoadHadNetworkError;

    void pageFinishedLoading(const juce::String& url) override
    {
        juce::WebBrowserComponent::pageFinishedLoading(url);
        if (onPageFinishedLoading)
            onPageFinishedLoading(url);
    }

    /**
     * @brief Suppresses benign WebView2 superseded-navigation errors (codes 9 and 14).
     * @param errorInfo Platform error string from WebView2.
     * @returns false to skip JUCE adhoc error page for suppressed codes.
     */
    bool pageLoadHadNetworkError(const juce::String& errorInfo) override
    {
#if JUCE_WINDOWS
        if (errorInfo.contains("Error code: 9") || errorInfo.contains("Error code: 14"))
        {
            if (onPageLoadHadNetworkError && errorInfo.isNotEmpty())
                onPageLoadHadNetworkError("suppressed: " + errorInfo);
            return false;
        }
#endif
        const bool allow = juce::WebBrowserComponent::pageLoadHadNetworkError(errorInfo);
        if (onPageLoadHadNetworkError && errorInfo.isNotEmpty())
            onPageLoadHadNetworkError(errorInfo);
        return allow;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrbitalsWebBrowserCallbacks)
};
