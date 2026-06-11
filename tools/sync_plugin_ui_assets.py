#!/usr/bin/env python3
"""
@fileoverview Sync shared UI assets into each plugin UI folder and update index.html / styles.css paths.
@module Orbitals/tools
"""

from __future__ import annotations

import re
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

ORBITALS = [
    "Apogee", "Eclipse", "Ion", "Kepler", "Lagrange", "Perihelion", "Retrograde", "Tidal", "Zenith"
]
MANDELBROT = [
    "Entanglement", "Fabric", "Hadron", "Mandelbrot", "Observer", "Planck", "Quarks", "Rabbithole", "Singularity"
]
EIGHT_BIT = ["ByteBeat", "ChipTune", "PixelPulse"]

ORBITALS_SHARED_UI = ROOT / "_Shared" / "UI"
ORBITALS_SHARED_ASSETS = ROOT / "_Shared" / "Assets"
MANDELBROT_SHARED = ROOT / "MandelbrotSet" / "_Shared"
EIGHT_BIT_SHARED = ROOT / "8-Bit Kit" / "_Shared"

ORBITALS_UI_FILES = [
    "orbitals-design-system.css",
    "orbitals-animations.js",
    "orbitals-particles.js",
    "orbitals-components.js",
]

MANDELBROT_UI_FILES = [
    "mandelbrot-design-system.css",
    "mandelbrot-animations.js",
    "mandelbrot-particles.js",
    "mandelbrot-components.js",
]

BACKGROUND_MAP: dict[str, tuple[Path, str]] = {}


def register_orbital(name: str) -> None:
    key = name.lower()
    BACKGROUND_MAP[name] = (
        ORBITALS_SHARED_ASSETS / "backgrounds" / f"{key}-background.png",
        f"{key}-background.png",
    )


def register_mandelbrot(name: str) -> None:
    key = name.lower()
    BACKGROUND_MAP[name] = (
        MANDELBROT_SHARED / "Assets" / "backgrounds" / f"{key}.png",
        f"{key}.png",
    )


def register_eight_bit(name: str) -> None:
    key = name.lower()
    BACKGROUND_MAP[name] = (
        EIGHT_BIT_SHARED / "Assets" / "backgrounds" / f"{key}.png",
        f"{key}.png",
    )


for p in ORBITALS:
    register_orbital(p)
for p in MANDELBROT:
    register_mandelbrot(p)
for p in EIGHT_BIT:
    register_eight_bit(p)


def plugin_dir(name: str) -> Path:
    if name in ORBITALS:
        return ROOT / name
    if name in MANDELBROT:
        return ROOT / "MandelbrotSet" / name
    return ROOT / "8-Bit Kit" / name


def shared_ui_dir(name: str) -> Path:
    if name in ORBITALS:
        return ORBITALS_SHARED_UI
    if name in MANDELBROT:
        return MANDELBROT_SHARED / "UI"
    return EIGHT_BIT_SHARED / "UI"


def ui_file_list(name: str) -> list[str]:
    if name in ORBITALS:
        return ORBITALS_UI_FILES
    return MANDELBROT_UI_FILES


def copy_assets(name: str) -> None:
    ui_dir = plugin_dir(name) / "UI"
    ui_dir.mkdir(parents=True, exist_ok=True)
    shared = shared_ui_dir(name)

    for fname in ui_file_list(name):
        src = shared / fname
        if src.exists():
            shutil.copy2(src, ui_dir / fname)

    logo_src = ORBITALS_SHARED_ASSETS / "logos" / "nnaudio-logo.png"
    if logo_src.exists():
        shutil.copy2(logo_src, ui_dir / "nnaudio-logo.png")

    bridge_src = ORBITALS_SHARED_UI / "orbitals-juce-bridge.js"
    if bridge_src.exists():
        shutil.copy2(bridge_src, ui_dir / "orbitals-juce-bridge.js")

    bg_src, _ = BACKGROUND_MAP[name]
    if bg_src.exists():
        shutil.copy2(bg_src, ui_dir / "plugin-background.png")


def patch_index_html(name: str) -> None:
    path = plugin_dir(name) / "UI" / "index.html"
    if not path.exists():
        return
    text = path.read_text(encoding="utf-8")

    for fname in ui_file_list(name):
        text = re.sub(
            rf'href="\.\./\.\./_Shared/UI/{re.escape(fname)}"',
            f'href="{fname}"',
            text,
        )
        text = re.sub(
            rf'src="\.\./\.\./_Shared/UI/{re.escape(fname)}"',
            f'src="{fname}"',
            text,
        )

    bridge_js = ROOT / "_Shared" / "UI" / "orbitals-juce-bridge.js"
    if bridge_js.exists() and "orbitals-juce-bridge.js" not in text:
        text = text.replace(
            "</body>",
            '    <script src="orbitals-juce-bridge.js"></script>\n</body>',
        )

    path.write_text(text, encoding="utf-8")


def patch_styles_css(name: str) -> None:
    path = plugin_dir(name) / "UI" / "styles.css"
    if not path.exists():
        return
    text = path.read_text(encoding="utf-8")
    text = re.sub(
        r"url\('\.\./\.\./_Shared/Assets/backgrounds/[^']+'\)",
        "url('plugin-background.png')",
        text,
    )
    text = re.sub(
        r'url\("\.\./\.\./_Shared/Assets/backgrounds/[^"]+"\)',
        "url('plugin-background.png')",
        text,
    )
    text = re.sub(
        r"url\('\.\./\.\./_Shared/Assets/logos/nnaudio-logo\.png'\)",
        "url('nnaudio-logo.png')",
        text,
    )
    path.write_text(text, encoding="utf-8")


def webview_rel_prefix(name: str) -> str:
    """@brief Relative path from plugin folder to shared WebView sources."""
    if name in ORBITALS:
        return "../_Shared/Source/WebView"
    return "../../../_Shared/Source/WebView"


def patch_jucer(name: str) -> None:
    """
    @brief Register local UI assets and shared OrbitalsWebViewHost sources in the .jucer project.
    @param name Plugin folder name.
    """
    jucer_path = plugin_dir(name) / f"{name}.jucer"
    if not jucer_path.exists():
        return
    text = jucer_path.read_text(encoding="utf-8")
    ui_dir = plugin_dir(name) / "UI"

    entries = [
        ("index_html", "index.html"),
        ("app_js", "app.js"),
        ("styles_css", "styles.css"),
        ("bridge_js", "orbitals-juce-bridge.js"),
    ]
    for fname in ui_file_list(name):
        fid = re.sub(r"[^a-z0-9]", "_", fname.lower())
        entries.append((fid, fname))
    entries.extend([
        ("nnaudio_logo", "nnaudio-logo.png"),
        ("plugin_background", "plugin-background.png"),
    ])

    ui_lines = ['    <GROUP id="{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}" name="UI">']
    for fid, fname in entries:
        fpath = ui_dir / fname
        if fpath.exists():
            ui_lines.append(
                f'      <FILE id="{fid}" name="{fname}" compile="0" resource="1" file="UI/{fname}" />'
            )
    ui_lines.append("    </GROUP>")
    ui_block = "\n".join(ui_lines)

    ui_match = re.search(
        r'<GROUP id="\{A1B2C3D4-E5F6-7890-ABCD-EF1234567890\}" name="UI">.*?</GROUP>',
        text,
        re.DOTALL,
    )
    if ui_match:
        text = text[: ui_match.start()] + ui_block + text[ui_match.end() :]

    if "OrbitalsWebViewHost.cpp" not in text:
        rel_prefix = webview_rel_prefix(name)
        h_rel = f"{rel_prefix}/OrbitalsWebViewHost.h"
        cpp_rel = f"{rel_prefix}/OrbitalsWebViewHost.cpp"
        insert = (
            f'      <FILE id="OrbitalsWebViewHost_h" name="OrbitalsWebViewHost.h" '
            f'compile="0" resource="0" file="{h_rel}" />\n'
            f'      <FILE id="OrbitalsWebViewHost_cpp" name="OrbitalsWebViewHost.cpp" '
            f'compile="1" resource="0" file="{cpp_rel}" />\n'
        )
        source_match = re.search(
            r'(<GROUP id="\{D7F8A864-8ECA-4FE0-8FEC-20B87407D899\}" name="Source">.*?)(</GROUP>)',
            text,
            re.DOTALL,
        )
        if source_match:
            text = (
                text[: source_match.start(2)]
                + insert
                + text[source_match.start(2) :]
            )

    if "JUCE_USE_WIN_WEBVIEW2" not in text:
        text = text.replace(
            "<JUCEOPTIONS ",
            '<JUCEOPTIONS JUCE_USE_WIN_WEBVIEW2="1" ',
            1,
        )

    jucer_path.write_text(text, encoding="utf-8")


def patch_design_system_css(name: str) -> None:
    """@brief Fix bundled asset paths inside copied design-system stylesheets."""
    for fname in ui_file_list(name):
        if not fname.endswith("-design-system.css"):
            continue
        path = plugin_dir(name) / "UI" / fname
        if not path.exists():
            continue
        text = path.read_text(encoding="utf-8")
        text = re.sub(
            r"url\('\.\./\.\./_Shared/Assets/logos/nnaudio-logo\.png'\)",
            "url('nnaudio-logo.png')",
            text,
        )
        text = re.sub(
            r'url\("\.\./\.\./_Shared/Assets/logos/nnaudio-logo\.png"\)',
            "url('nnaudio-logo.png')",
            text,
        )
        path.write_text(text, encoding="utf-8")


def sync_all() -> None:
    all_plugins = ORBITALS + MANDELBROT + EIGHT_BIT
    for name in all_plugins:
        copy_assets(name)
        patch_index_html(name)
        patch_styles_css(name)
        patch_design_system_css(name)
        patch_jucer(name)
        print(f"synced {name}")


if __name__ == "__main__":
    sync_all()
