#!/usr/bin/env python3
"""
@fileoverview Fix JS bridge guards and standardize sendToJUCE across all plugin app.js files.
@module Orbitals/tools
"""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

SEND_TO_JUCE_HELPER = """
/**
 * @brief Sends parameter changes to C++ via JUCE native bridge (macOS + Windows).
 * @param {string} param Parameter ID.
 * @param {*} value Raw parameter value.
 */
function sendToJUCE(param, value) {
    if (typeof window.postMessageToJUCE !== 'function') return;
    window.postMessageToJUCE({ type: 'parameterChange', parameter: param, value: value });
}
"""

SEND_TO_PLUGIN_IMPL = """
function sendToPlugin(parameter, value) {
    if (typeof window.postMessageToJUCE !== 'function') return;
    window.postMessageToJUCE({
        type: parameter === 'openSettings' ? 'openSettings' : 'parameterChange',
        parameter: parameter,
        value: value
    });
}
"""


def find_app_js() -> list[Path]:
    out: list[Path] = []
    for p in ROOT.rglob("UI/app.js"):
        if "Builds" in p.parts:
            continue
        out.append(p)
    return sorted(out)


def patch_app_js(path: Path) -> None:
    text = path.read_text(encoding="utf-8")
    original = text

    # Remove broken platform guards
    text = re.sub(
        r"function sendToJUCE\(param, value\) \{\s*if \(window\.chrome\?\.webview\) \{\s*"
        r"window\.postMessageToJUCE\(\{ type: 'parameterChange', parameter: param, value \}\);\s*\}\s*\}",
        SEND_TO_JUCE_HELPER.strip(),
        text,
    )
    text = re.sub(
        r"function sendParameterToJUCE\(param, value\) \{\s*if \(window\.chrome\?\.webview\) \{\s*"
        r"window\.postMessageToJUCE\(\{ type: 'parameterChange', parameter: param, value \}\);\s*\}\s*\}",
        SEND_TO_JUCE_HELPER.strip(),
        text,
    )

    # Replace window.juce guards in sendToPlugin
    text = re.sub(
        r"function sendToPlugin\(parameter, value\) \{\s*if \(window\.juce\) \{\s*"
        r"window\.postMessageToJUCE\(\{[\s\S]*?\}\);\s*\}\s*\}",
        SEND_TO_PLUGIN_IMPL.strip(),
        text,
    )

    # playNote/stopNote in ChipTune
    text = text.replace("if (window.juce) {", "if (typeof window.postMessageToJUCE === 'function') {")

    # Add sendToJUCE if missing and setupBypassToggle references it
    if "function sendToJUCE" not in text and "setupBypassToggle(sendToJUCE)" in text:
        text = SEND_TO_JUCE_HELPER + "\n" + text

    # Remove Fabric debug alerts
    text = re.sub(r"\s*if \(parameter === 'bypass'\) \{\s*alert\([^)]+\);\s*\}", "", text)

    if text != original:
        path.write_text(text, encoding="utf-8")
        print(f"patched {path.relative_to(ROOT)}")


def patch_auth_html(path: Path) -> None:
    if not path.exists():
        return
    text = path.read_text(encoding="utf-8")
    if "postMessageToJUCE" in text and "window.chrome.webview" in text:
        return
    bridge = (ROOT / "_Shared" / "UI" / "juce-native-bridge-snippet.html").read_text(encoding="utf-8")
    if "postMessageToJUCE" in text:
        return
    text = text.replace("<body>", "<body>\n" + bridge.strip())
    path.write_text(text, encoding="utf-8")
    print(f"patched auth {path.relative_to(ROOT)}")


def main() -> None:
    for app in find_app_js():
        patch_app_js(app)
    for auth in ROOT.rglob("UI/auth.html"):
        if "Builds" not in auth.parts:
            patch_auth_html(auth)


if __name__ == "__main__":
    main()
