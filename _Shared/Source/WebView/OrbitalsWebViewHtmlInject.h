/*
  @fileoverview HTML head/body injection helpers for Orbitals WebView pages.
  @module Orbitals/_Shared/Source/WebView
*/

#pragma once

#include <JuceHeader.h>

namespace OrbitalsWebViewHtmlInject
{
/**
 * @brief Injects black background styles immediately after the opening head tag.
 * @param htmlContent HTML document string to modify in place.
 */
inline void injectBlackFlashStyles(juce::String& htmlContent)
{
    const juce::String blackStyles = R"(<style>
        html, body {
            background-color: #000000 !important;
            margin: 0;
            padding: 0;
        }
    </style>)";

    if (htmlContent.contains("<head>"))
    {
        htmlContent = htmlContent.replace("<head>", "<head>\n    " + blackStyles);
        return;
    }

    const int headStart = htmlContent.indexOf("<head");
    if (headStart < 0)
        return;

    for (int i = headStart; i < htmlContent.length(); ++i)
    {
        if (htmlContent[i] == '>')
        {
            htmlContent = htmlContent.substring(0, i + 1) + "\n    " + blackStyles + htmlContent.substring(i + 1);
            break;
        }
    }
}

/**
 * @brief Appends standalone flag and context-menu disable script before closing body.
 * @param htmlContent HTML document string to modify in place.
 */
inline void injectBodyScripts(juce::String& htmlContent)
{
    juce::String script = R"(<script>
        document.addEventListener('contextmenu', function(e) { e.preventDefault(); return false; });
        document.addEventListener('selectstart', function(e) { e.preventDefault(); return false; });
        window.isStandaloneMode = )";

#if JucePlugin_Build_Standalone
    script += "true";
#else
    script += "false";
#endif

    script += R"(;
    </script>)";

    if (htmlContent.contains("</body>"))
        htmlContent = htmlContent.replace("</body>", script + "</body>");
    else if (htmlContent.contains("</html>"))
        htmlContent = htmlContent.replace("</html>", script + "</html>");
    else
        htmlContent += script;
}

/**
 * @brief Builds the inline auth screen HTML used when license is missing.
 * @param pluginDisplayName Plugin title shown on the auth screen.
 * @param backgroundDataUrl CSS url value for background (data URL or "none").
 * @param logoDataUrl img src value for logo (data URL or empty).
 * @returns Complete auth HTML document.
 */
inline juce::String buildAuthHtml(const juce::String& pluginDisplayName,
                                  const juce::String& backgroundDataUrl,
                                  const juce::String& logoDataUrl)
{
    juce::String html = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>__PLUGIN__ - Authentication Required</title>
    <style>
        html, body { background-color: #000000 !important; margin: 0; padding: 0; }
    </style>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        html, body {
            width: 1200px; height: 750px; overflow: hidden;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
            background: #0a0a0f; color: #e8e8f0; position: relative;
        }
        body::before {
            content: ''; position: absolute; top: 0; left: 0; width: 100%; height: 100%;
            background-image: url('__BG__'); background-size: cover;
            background-position: center; background-repeat: no-repeat;
            opacity: 0.6; z-index: 0;
        }
        .auth-container {
            position: relative; z-index: 1; text-align: center; padding: 40px;
            display: flex; flex-direction: column; align-items: center;
            justify-content: center; height: 100%; width: 100%;
        }
        .logo-container { position: relative; z-index: 2; margin-bottom: 40px; }
        .logo-container img { max-width: 300px; height: auto; }
        .text-content {
            position: relative; z-index: 2; background: rgba(10, 10, 15, 0.85);
            padding: 50px 60px; border-radius: 20px; max-width: 700px;
        }
        .auth-title {
            font-size: 48px; font-weight: bold; color: #00d4ff; margin-bottom: 30px;
            font-family: 'Orbitron', monospace; letter-spacing: 4px;
        }
        .auth-message { font-size: 20px; line-height: 1.8; color: #e8e8f0; }
        .auth-status { margin-top: 30px; font-size: 14px; color: #888; }
    </style>
</head>
<body>
    <div class="auth-container">
        <div class="logo-container">
            <img src="__LOGO__" alt="NNAudio Logo">
        </div>
        <div class="text-content">
            <h1 class="auth-title">__PLUGIN__</h1>
            <p class="auth-message">
                Your plugin's authentication needs to be refreshed.<br><br>
                Please launch the NNAudio Access app to continue.
            </p>
            <p class="auth-status">Checking license...</p>
        </div>
    </div>
</body>
</html>)";

    html = html.replace("__PLUGIN__", pluginDisplayName.toUpperCase());
    html = html.replace("__BG__", backgroundDataUrl.isNotEmpty() ? backgroundDataUrl : "none");
    html = html.replace("__LOGO__", logoDataUrl);
    return html;
}

/**
 * @brief Converts a PNG file to a data URL, or returns empty string if missing.
 * @param imageFile Path to PNG on disk.
 * @returns data:image/png;base64,... or empty.
 */
inline juce::String fileToDataUrl(const juce::File& imageFile)
{
    if (!imageFile.existsAsFile())
        return {};

    juce::MemoryBlock imageData;
    if (!imageFile.loadFileAsData(imageData))
        return {};

    return "data:image/png;base64," + juce::Base64::toBase64(imageData.getData(), imageData.getSize());
}

} // namespace OrbitalsWebViewHtmlInject
