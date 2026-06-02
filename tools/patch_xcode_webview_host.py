#!/usr/bin/env python3
"""
@fileoverview Patch generated Xcode projects to compile shared OrbitalsWebViewHost sources.
@module Orbitals/tools
"""

from __future__ import annotations

import re
import uuid
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

ORBITALS = [
    "Apogee", "Eclipse", "Ion", "Kepler", "Lagrange", "Perihelion", "Retrograde", "Tidal", "Zenith"
]
MANDELBROT = [
    "Entanglement", "Fabric", "Hadron", "Mandelbrot", "Observer", "Planck", "Quarks", "Rabbithole", "Singularity"
]
EIGHT_BIT = ["ByteBeat", "ChipTune", "PixelPulse"]


def plugin_dir(name: str) -> Path:
    """
    @brief Resolve plugin root directory.
    @param name Plugin display name.
    @returns Absolute plugin folder path.
    """
    if name in ORBITALS:
        return ROOT / name
    if name in MANDELBROT:
        return ROOT / "MandelbrotSet" / name
    return ROOT / "8-Bit Kit" / name


def webview_rel_prefix(name: str) -> str:
    """
    @brief Relative path from Builds/MacOSX to shared WebView sources.
    @param name Plugin display name.
    @returns Relative path prefix for pbxproj file entries.
    """
    if name in ORBITALS:
        return "../../../_Shared/Source/WebView"
    return "../../../../_Shared/Source/WebView"


def juce_id() -> str:
    """
    @brief Generate a Projucer-style 24-character hex identifier.
    @returns Uppercase hex string.
    """
    return uuid.uuid4().hex[:24].upper()


def fix_webview_paths(text: str, name: str) -> tuple[str, bool]:
    """
    @brief Correct OrbitalsWebViewHost file paths when already present in pbxproj.
    @param text Raw project.pbxproj contents.
    @param name Plugin display name.
    @returns Tuple of updated text and whether any path was changed.
    """
    prefix = webview_rel_prefix(name)
    expected_cpp = f"{prefix}/OrbitalsWebViewHost.cpp"
    expected_h = f"{prefix}/OrbitalsWebViewHost.h"
    wrong_prefixes = [
        "../../../_Shared/Source/WebView",
        "../../../../_Shared/Source/WebView",
        "../../../../../_Shared/Source/WebView",
    ]
    changed = False
    for wrong in wrong_prefixes:
        if wrong == prefix:
            continue
        for suffix in ("OrbitalsWebViewHost.cpp", "OrbitalsWebViewHost.h"):
            old = f"path = {wrong}/{suffix};"
            new = f"path = {prefix}/{suffix};"
            if old in text:
                text = text.replace(old, new)
                changed = True
    if "OrbitalsWebViewHost.cpp" in text:
        if expected_cpp not in text or expected_h not in text:
            raise RuntimeError(f"OrbitalsWebViewHost paths remain incorrect for {name}")
    return text, changed


def patch_pbxproj(pbxproj: Path, name: str) -> bool:
    """
    @brief Insert or fix OrbitalsWebViewHost.cpp/.h in an existing Xcode project.
    @param pbxproj Path to project.pbxproj.
    @param name Plugin display name.
    @returns True if the project was modified.
    """
    text = pbxproj.read_text(encoding="utf-8")
    if "OrbitalsWebViewHost.cpp" in text:
        text, changed = fix_webview_paths(text, name)
        if changed:
            pbxproj.write_text(text, encoding="utf-8")
        return changed

    prefix = webview_rel_prefix(name)
    cpp_path = f"{prefix}/OrbitalsWebViewHost.cpp"
    h_path = f"{prefix}/OrbitalsWebViewHost.h"

    hub_auth_file = re.search(
        r"(\w+) /\* HubAuthComponent\.cpp \*/ /\* HubAuthComponent\.cpp \*/ = "
        r"\{isa = PBXFileReference;",
        text,
    )
    if not hub_auth_file:
        raise RuntimeError(f"Could not locate HubAuthComponent.cpp file reference in {pbxproj}")

    hub_auth_file_id = hub_auth_file.group(1)
    hub_auth_build = re.search(
        rf"(\w+) /\* HubAuthComponent\.cpp \*/ = "
        rf"\{{isa = PBXBuildFile; fileRef = {hub_auth_file_id}; \}};",
        text,
    )
    if not hub_auth_build:
        raise RuntimeError(f"Could not locate HubAuthComponent.cpp build file in {pbxproj}")

    editor_build = re.search(
        r"(\w+) /\* PluginEditor\.cpp \*/ = \{isa = PBXBuildFile; fileRef = \w+; \};",
        text,
    )
    if not editor_build:
        raise RuntimeError(f"Could not locate PluginEditor.cpp build file in {pbxproj}")

    build_cpp_id = juce_id()
    build_h_id = juce_id()
    file_cpp_id = juce_id()
    file_h_id = juce_id()

    build_cpp = (
        f"\t\t{build_cpp_id} /* OrbitalsWebViewHost.cpp */ = "
        f"{{isa = PBXBuildFile; fileRef = {file_cpp_id}; }};\n"
    )
    build_h = (
        f"\t\t{build_h_id} /* OrbitalsWebViewHost.h */ = "
        f"{{isa = PBXBuildFile; fileRef = {file_h_id}; }};\n"
    )

    file_cpp = (
        f"\t\t{file_cpp_id} /* OrbitalsWebViewHost.cpp */ /* OrbitalsWebViewHost.cpp */ = "
        f"{{isa = PBXFileReference; lastKnownFileType = sourcecode.cpp.cpp; "
        f'name = OrbitalsWebViewHost.cpp; path = {cpp_path}; sourceTree = SOURCE_ROOT; }};\n'
    )
    file_h = (
        f"\t\t{file_h_id} /* OrbitalsWebViewHost.h */ /* OrbitalsWebViewHost.h */ = "
        f"{{isa = PBXFileReference; lastKnownFileType = sourcecode.c.h; "
        f'name = OrbitalsWebViewHost.h; path = {h_path}; sourceTree = SOURCE_ROOT; }};\n'
    )

    text = text.replace("/* End PBXBuildFile section */", build_cpp + build_h + "/* End PBXBuildFile section */", 1)
    text = text.replace("/* End PBXFileReference section */", file_cpp + file_h + "/* End PBXFileReference section */", 1)

    source_group = re.search(
        rf"(/\* Source \*/ = \{{\s*isa = PBXGroup;\s*children = \([\s\S]*?{hub_auth_file_id},\s*\n)(\s*\);)",
        text,
    )
    if not source_group:
        raise RuntimeError(f"Could not locate Source group in {pbxproj}")

    group_insert = (
        f"{source_group.group(1)}"
        f"\t\t\t\t{file_h_id},\n"
        f"\t\t\t\t{file_cpp_id},\n"
        f"{source_group.group(2)}"
    )
    text = text[: source_group.start()] + group_insert + text[source_group.end() :]

    editor_build_id = editor_build.group(1)
    hub_auth_build_id = hub_auth_build.group(1)
    sources_phase = re.search(
        rf"(isa = PBXSourcesBuildPhase;\s*buildActionMask = 2147483647;\s*files = \([\s\S]*?"
        rf"{editor_build_id},\s*\n"
        rf"{hub_auth_build_id},\s*\n)",
        text,
    )
    if not sources_phase:
        sources_phase = re.search(
            rf"(isa = PBXSourcesBuildPhase;\s*buildActionMask = 2147483647;\s*files = \([\s\S]*?"
            rf"{hub_auth_build_id},\s*\n)",
            text,
        )
    if not sources_phase:
        raise RuntimeError(f"Could not locate Shared Code sources phase in {pbxproj}")

    phase_insert = f"{sources_phase.group(1)}\t\t\t\t{build_cpp_id},\n"
    text = text[: sources_phase.start()] + phase_insert + text[sources_phase.end() :]

    pbxproj.write_text(text, encoding="utf-8")
    return True


def patch_all() -> None:
    """@brief Patch every plugin Xcode project under Builds/MacOSX."""
    all_plugins = ORBITALS + MANDELBROT + EIGHT_BIT
    for name in all_plugins:
        pbxproj = plugin_dir(name) / "Builds" / "MacOSX" / f"{name}.xcodeproj" / "project.pbxproj"
        if not pbxproj.exists():
            print(f"skip (no xcodeproj): {name}")
            continue
        if patch_pbxproj(pbxproj, name):
            print(f"patched {name}")
        else:
            print(f"ok {name}")


if __name__ == "__main__":
    patch_all()
