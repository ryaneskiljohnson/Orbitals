#!/usr/bin/env python3
"""
@fileoverview Migrate PluginEditor files to OrbitalsWebViewHost (CymaSynth pattern).
@module Orbitals/tools
"""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

WEBVIEW_INCLUDE = {
    "orbitals": "../../_Shared/Source/WebView/OrbitalsWebViewHost.h",
    "mandelbrot": "../../../_Shared/Source/WebView/OrbitalsWebViewHost.h",
    "eightbit": "../../../_Shared/Source/WebView/OrbitalsWebViewHost.h",
}

PLUGINS: list[tuple[str, Path, str, int]] = []
for name in ["Apogee", "Eclipse", "Ion", "Kepler", "Lagrange", "Perihelion", "Retrograde", "Tidal", "Zenith"]:
    PLUGINS.append((name, ROOT / name, "orbitals", 1000 * 60 * 15))
for name in ["Entanglement", "Fabric", "Hadron", "Mandelbrot", "Observer", "Planck", "Quarks", "Rabbithole", "Singularity"]:
    PLUGINS.append((name, ROOT / "MandelbrotSet" / name, "mandelbrot", 50))
for name in ["ByteBeat", "ChipTune", "PixelPulse"]:
    PLUGINS.append((name, ROOT / "8-Bit Kit" / name, "eightbit", 50))


def patch_header(path: Path, cls: str, include_key: str) -> None:
    text = path.read_text(encoding="utf-8")
    include = WEBVIEW_INCLUDE[include_key]

    text = re.sub(
        r"//==============================================================================\n"
        r"// Helper class to handle WebBrowserComponent callbacks.*?"
        r"};\n\n",
        "",
        text,
        flags=re.DOTALL,
    )

    if include not in text:
        text = text.replace(
            '#include "PluginProcessor.h"',
            f'#include "PluginProcessor.h"\n#include "{include}"',
        )

    text = re.sub(
        r"    std::unique_ptr<WebBrowserWithCallbacks> webView;\n"
        r"    std::unique_ptr<juce::Label> errorLabel;\n"
        r"    bool webViewInitialized = false;\n",
        "    OrbitalsWebViewHost webViewHost;\n",
        text,
    )

    text = re.sub(
        r"    void loadWebUI\(\);\n"
        r"    void loadAuthScreen\(\);\n"
        r"    void loadHTMLFile \(const juce::File& htmlFile\);\n",
        "",
        text,
    )

    path.write_text(text, encoding="utf-8")


def webview_ctor_block(cls: str, name: str, auth_timer_ms: int) -> str:
    timer_authorized = auth_timer_ms if auth_timer_ms >= 1000 else 50
    timer_unauthorized = 5000
    return f"""
    OrbitalsWebViewHostConfig webConfig;
    webConfig.pluginDisplayName = "{name}";
    webConfig.devPluginDirectory = juce::File (__FILE__).getParentDirectory().getParentDirectory();

    webViewHost.initialize (*this, webConfig, [this](const juce::var& msg)
    {{
        handleJavaScriptMessage (msg);
    }});

    isAuthorized = checkAuthorization();

    if (isAuthorized)
        startTimer ({timer_authorized});
    else
        startTimer ({timer_unauthorized});

    juce::Timer::callAfterDelay (300, [this]()
    {{
        webViewHost.dispatchInitialNavigation (isAuthorized);
    }});
"""


def patch_cpp(path: Path, cls: str, name: str, auth_timer_ms: int) -> None:
    text = path.read_text(encoding="utf-8")

    # Remove old webview constructor block through auth load
    text = re.sub(
        r"\n    auto safeEditor = juce::Component::SafePointer.*?// Check authorization first.*?"
        r"(        startTimer\(5000\).*?\n    \}\n)",
        webview_ctor_block(cls, name, auth_timer_ms),
        text,
        flags=re.DOTALL,
        count=1,
    )

    # Alternative pattern when comment differs
    if "webViewHost.initialize" not in text:
        text = re.sub(
            r"\n    auto safeEditor = juce::Component::SafePointer.*?"
            r"if \(! webViewInitialized\)\s*\{.*?\}\n\n\n",
            webview_ctor_block(cls, name, auth_timer_ms) + "\n",
            text,
            flags=re.DOTALL,
            count=1,
        )

    text = re.sub(
        rf"({cls}::~{cls} \(\)\s*(?:override\s*)?\{{\s*)"
        r"if \(webView != nullptr && webViewInitialized\)\s*"
        r"webView->onPageFinishedLoading = nullptr;\s*",
        r"\1webViewHost.shutdown ();\n    ",
        text,
    )

    text = text.replace(
        """    if (! webViewInitialized || webView == nullptr)
        return;
    webView->setBounds (getLocalBounds());""",
        "    webViewHost.resized (*this);",
    )

    # Remove loadWebUI, loadHTMLFile, loadAuthScreen functions entirely
    text = re.sub(
        r"void \w+AudioProcessorEditor::loadWebUI\(\)\s*\{.*?\n\}\n",
        "",
        text,
        flags=re.DOTALL,
    )
    text = re.sub(
        r"void \w+AudioProcessorEditor::loadHTMLFile \(const juce::File& htmlFile\)\s*\{.*?\n\}\n",
        "",
        text,
        flags=re.DOTALL,
    )
    text = re.sub(
        r"void \w+AudioProcessorEditor::loadAuthScreen\(\)\s*\{.*?\n\}\n",
        "",
        text,
        flags=re.DOTALL,
    )

    # checkAuthorization reload
    text = text.replace("loadWebUI();", "webViewHost.reloadSurface (true);")
    text = text.replace("loadAuthScreen();", "webViewHost.reloadSurface (false);")

    # sendMeteringData / evaluateJavascript
    text = re.sub(
        r"if \(webView != nullptr && webViewInitialized && webView->isVisible\(\)\)\s*\{\s*"
        r"webView->evaluateJavascript \(script\);",
        "webViewHost.evaluateJavascript (script);",
        text,
    )
    text = re.sub(
        r"if \(webView != nullptr && webViewInitialized\)\s*\{\s*"
        r"webView->evaluateJavascript \(script\);",
        "webViewHost.evaluateJavascript (script);",
        text,
    )
    text = re.sub(
        r"if \(webView != nullptr && webViewInitialized && isAuthorized\)",
        "if (webViewHost.isInitialized() && isAuthorized)",
        text,
    )
    text = re.sub(
        r"webView->emitEventIfBrowserIsVisible\(\"eval\", script\);",
        "webViewHost.evaluateJavascript (script);",
        text,
    )

    # timerCallback metering visibility checks
    text = text.replace(
        "isAuthorized && webView != nullptr && webViewInitialized && webView->isVisible()",
        "isAuthorized && webViewHost.isVisible()",
    )

    # keyPressed visibility
    text = text.replace(
        "webView != nullptr && webViewInitialized && webView->isVisible()",
        "webViewHost.isVisible()",
    )

    path.write_text(text, encoding="utf-8")


def migrate_all() -> None:
    for name, plugin_dir, include_key, timer_ms in PLUGINS:
        header = plugin_dir / "Source" / "PluginEditor.h"
        cpp = plugin_dir / "Source" / "PluginEditor.cpp"
        if not header.exists() or not cpp.exists():
            print(f"skip missing {name}")
            continue
        cls = f"{name}AudioProcessorEditor"
        patch_header(header, cls, include_key)
        patch_cpp(cpp, cls, name, timer_ms)
        print(f"migrated {name}")


if __name__ == "__main__":
    migrate_all()
