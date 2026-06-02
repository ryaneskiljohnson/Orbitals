#!/usr/bin/env python3
"""Clean up leftover webView references after OrbitalsWebViewHost migration."""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def cleanup(path: Path) -> None:
    text = path.read_text(encoding="utf-8")
    original = text

    text = re.sub(
        r"\s*if \(webView != nullptr && webViewInitialized\)\s*"
        r"webView->onPageFinishedLoading = nullptr;",
        "\n    webViewHost.shutdown();",
        text,
    )

    text = text.replace("webView->evaluateJavascript (script);", "webViewHost.evaluateJavascript (script);")
    text = text.replace("webView->evaluateJavascript(script);", "webViewHost.evaluateJavascript(script);")
    text = text.replace("webView->emitEventIfBrowserIsVisible(\"eval\", script);", "webViewHost.evaluateJavascript (script);")

    text = re.sub(
        r"if \(webView != nullptr && webViewInitialized && webView->isVisible\(\)\)",
        "if (webViewHost.isVisible())",
        text,
    )
    text = re.sub(
        r"if \(webView != nullptr && webView->isVisible\(\)\)",
        "if (webViewHost.isVisible())",
        text,
    )
    text = re.sub(
        r"webView != nullptr && webViewInitialized && webView->isVisible\(\)",
        "webViewHost.isVisible()",
        text,
    )
    text = re.sub(
        r"webView != nullptr && webView->isVisible\(\)",
        "webViewHost.isVisible()",
        text,
    )
    text = re.sub(
        r"\(webView != nullptr\)",
        "(webViewHost.isInitialized())",
        text,
    )
    text = re.sub(
        r"webView == nullptr",
        "! webViewHost.isInitialized()",
        text,
    )
    text = re.sub(
        r"! webViewInitialized \|\| webView == nullptr",
        "! webViewHost.isInitialized()",
        text,
    )
    text = re.sub(
        r"webViewInitialized && webView != nullptr",
        "webViewHost.isInitialized()",
        text,
    )

    # Remove Fabric debug test block
    text = re.sub(
        r"\s*// Immediately try to run JavaScript.*?Both JavaScript execution methods called.*?\}\);\s*\}\);",
        "",
        text,
        flags=re.DOTALL,
    )

    if text != original:
        path.write_text(text, encoding="utf-8")
        print(f"cleaned {path.relative_to(ROOT)}")


def main() -> None:
    for cpp in ROOT.rglob("Source/PluginEditor.cpp"):
        if "Builds" not in cpp.parts:
            cleanup(cpp)


if __name__ == "__main__":
    main()
