#!/usr/bin/env python3
"""
@fileoverview Batch-update Orbitals + Mandelbrot plugins to match CymaSynth WebView setup.
@module Orbitals/tools
"""
from __future__ import annotations

import re
import sys
from pathlib import Path
from string import Template

ROOT = Path(__file__).resolve().parents[1]

BRIDGE_MARK = "postMessageToJUCE"

BRIDGE_INLINE = r"""    <!-- JUCE native bridge: WKWebView + WebView2 (matches CymaSynth) -->
    <script>
    (function() {
        function sendToHost(msg) {
            try {
                var envelope = (msg && msg.eventId !== undefined) ? msg : { eventId: 'message', payload: msg };
                var payload = (typeof envelope === 'string') ? envelope : JSON.stringify(envelope);
                if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.__JUCE__) {
                    window.webkit.messageHandlers.__JUCE__.postMessage(payload);
                    return true;
                }
                if (window.chrome && window.chrome.webview && typeof window.chrome.webview.postMessage === 'function') {
                    window.chrome.webview.postMessage(payload);
                    return true;
                }
                return false;
            } catch (e) { return false; }
        }
        window.postMessageToJUCE = function(data) { sendToHost(data); };
        window.__JUCE__ = window.__JUCE__ || {
            postMessage: sendToHost,
            backend: {
                emitEvent: function(name, data) {
                    if (name === 'message') return sendToHost(data);
                    return sendToHost({ event: name, data: data });
                }
            }
        };
    })();
    </script>
"""

HANDLE_JS_PREAMBLE = """    juce::var msg = message;
    if (msg.isString())
    {
        juce::var parsed = juce::JSON::parse (msg.toString());
        if (parsed.isObject())
            msg = parsed;
    }
    if (! msg.isObject())
        return;

    auto obj = msg.getDynamicObject();
    if (obj == nullptr)
        return;

    if (obj->hasProperty (\"payload\") && obj->getProperty (\"payload\").isObject())
    {
        msg = obj->getProperty (\"payload\");
        obj = msg.getDynamicObject();
        if (obj == nullptr)
            return;
    }

"""

# C++ source must contain \\n escapes, not literal newlines inside the string literal.
_WEBVIEW_FAIL_LABEL = r'        errorLabel = std::make_unique<juce::Label> ("error", "WebView initialization failed.\nThe plugin UI cannot be displayed.\n\nOn Windows, WebView2 must be available.");'

CTOR_TEMPLATE = Template("""    auto safeEditor = juce::Component::SafePointer<$cls> (this);
    auto options = juce::WebBrowserComponent::Options{}
#if JUCE_WINDOWS
                       .withBackend (juce::WebBrowserComponent::Options::Backend::webview2)
#endif
                       .withNativeIntegrationEnabled (true)
                       .withKeepPageLoadedWhenBrowserIsHidden()
                       .withEventListener ("message", [safeEditor](const juce::var& message) {
                           if (safeEditor != nullptr)
                               safeEditor->handleJavaScriptMessage (message);
                       });

    try
    {
        webView = std::make_unique<WebBrowserWithCallbacks> (options);
        if (webView != nullptr)
        {
            webViewInitialized = true;
            addChildComponent (webView.get());
            webView->setBounds (getLocalBounds());
            webView->setVisible (false);

            auto safeThis = juce::Component::SafePointer<$cls> (this);
            webView->onPageFinishedLoading = [safeThis](const juce::String&)
            {
                if (safeThis == nullptr) return;
                juce::MessageManager::callAsync ([safeThis]()
                {
                    if (safeThis == nullptr) return;
                    juce::Timer::callAfterDelay (2000, [safeThis]()
                    {
                        if (safeThis == nullptr || ! safeThis->webViewInitialized || safeThis->webView == nullptr || safeThis->webView->isVisible()) return;

                        juce::String checkScript = R"(
                    (function() {
                        if (!document.body) return false;
                        var style = window.getComputedStyle(document.body);
                        var bg = style.backgroundColor;
                        return bg === 'rgb(0, 0, 0)' || bg === 'black' || bg.indexOf('0, 0, 0') >= 0;
                    })();
                )";

                        if (safeThis->webView != nullptr && safeThis->webViewInitialized)
                        {
                            safeThis->webView->evaluateJavascript (checkScript, [safeThis](const juce::WebBrowserComponent::EvaluationResult& result)
                            {
                                if (safeThis == nullptr || ! safeThis->webViewInitialized || safeThis->webView == nullptr || safeThis->webView->isVisible()) return;

                                bool isBlack = false;
                                if (auto* value = result.getResult())
                                {
                                    if (value->isBool())
                                        isBlack = static_cast<bool> (*value);
                                }

                                juce::Timer::callAfterDelay (isBlack ? 0 : 500, [safeThis]()
                                {
                                    if (safeThis == nullptr || ! safeThis->webViewInitialized || safeThis->webView == nullptr || safeThis->webView->isVisible()) return;
                                    safeThis->webView->setVisible (true);
                                    safeThis->repaint();
                                });
                            });
                        }
                    });
                });
            };

            auto safeThisFallback = juce::Component::SafePointer<$cls> (this);
            juce::Timer::callAfterDelay (3000, [safeThisFallback]()
            {
                if (safeThisFallback == nullptr || ! safeThisFallback->webViewInitialized || safeThisFallback->webView == nullptr || safeThisFallback->webView->isVisible()) return;
                safeThisFallback->webView->setVisible (true);
                safeThisFallback->repaint();
            });
        }
        else
        {
            webViewInitialized = false;
        }
    }
    catch (const std::exception& e)
    {
        DBG (juce::String ("Failed to initialize WebView: ") + e.what());
        webViewInitialized = false;
        webView.reset();
    }
    catch (...)
    {
        webViewInitialized = false;
        webView.reset();
    }

    if (! webViewInitialized)
    {
        $fail_label
        errorLabel->setJustificationType (juce::Justification::centred);
        errorLabel->setColour (juce::Label::textColourId, juce::Colours::white);
        errorLabel->setBounds (getLocalBounds());
        addAndMakeVisible (errorLabel.get());
    }

""")


def ctor_substitute(cls: str) -> str:
    return CTOR_TEMPLATE.substitute(cls=cls, fail_label=_WEBVIEW_FAIL_LABEL)


def discover_editor_cpps() -> list[Path]:
    out: list[Path] = []
    for p in ROOT.rglob("Source/PluginEditor.cpp"):
        t = p.read_text(encoding="utf-8", errors="replace")
        if "WebBrowserWithCallbacks" in t and "AudioProcessorEditor" in t:
            out.append(p)
    return sorted(out)


def editor_class_name(text: str) -> str | None:
    m = re.search(r"(\w+AudioProcessorEditor)::\1\s*\(", text)
    return m.group(1) if m else None


def replace_webview_ctor(text: str, cls: str) -> str:
    pattern = re.compile(
        r"\n    // Create WebView with native integration enabled.*?(?=\n    // Check authorization)",
        re.DOTALL,
    )
    new_block = "\n" + ctor_substitute(cls)
    if not pattern.search(text):
        raise RuntimeError("ctor pattern not found")
    return pattern.sub(new_block, text, count=1)


def patch_destructor(text: str, cls: str) -> str:
    if "onPageFinishedLoading = nullptr" in text:
        return text
    depat = re.compile(
        rf"({re.escape(cls)}::~{re.escape(cls)}\s*\(\)\s*(?:override\s*)?\{{\s*)\}}",
        re.DOTALL,
    )

    def repl(m: re.Match) -> str:
        body = m.group(1)
        return (
            body
            + "\n    if (webView != nullptr && webViewInitialized)\n"
            + "        webView->onPageFinishedLoading = nullptr;\n"
            + "}"
        )

    new_t, n = depat.subn(repl, text, count=1)
    if n != 1:
        raise RuntimeError(f"destructor patch failed for {cls}")
    return new_t


def patch_resized(text: str) -> str:
    old = """    if (webView != nullptr)
        webView->setBounds (getLocalBounds());"""
    new = """    if (! webViewInitialized || webView == nullptr)
        return;
    webView->setBounds (getLocalBounds());"""
    if old not in text:
        raise RuntimeError("resized webView block not found")
    return text.replace(old, new, 1)


def patch_handle_javascript_simple(text: str) -> str:
    """Replace opening of handleJavaScriptMessage for simple handlers."""
    pat = re.compile(
        r"(void \w+AudioProcessorEditor::handleJavaScriptMessage \(const juce::var& message\)\s*\{\s*)"
        r"if \(!message\.isObject\(\)\)\s*return;\s*"
        r"auto obj = message\.getDynamicObject\(\);\s*"
        r"if \(obj == nullptr\)\s*return;\s*",
        re.DOTALL,
    )
    m = pat.search(text)
    if not m:
        return text
    if "obj->hasProperty (\"payload\")" in text[m.start() : m.start() + 400]:
        return text
    return text[: m.start()] + m.group(1) + HANDLE_JS_PREAMBLE + text[m.end() :]


def patch_handle_chip_tune(text: str) -> str | None:
    old = """void ChipTuneAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
{
    std::cout << "=== RECEIVED MESSAGE FROM JAVASCRIPT ===" << std::endl;
    DBG("=== RECEIVED MESSAGE FROM JAVASCRIPT ===");
    
    if (!message.isObject())
    {
        std::cout << "Message is not an object" << std::endl;
        DBG("Message is not an object");
        return;
    }

    auto obj = message.getDynamicObject();
    if (obj == nullptr)
    {
        std::cout << "Message dynamic object is nullptr" << std::endl;
        DBG("Message dynamic object is nullptr");
        return;
    }
"""
    if old not in text:
        return None
    new = """void ChipTuneAudioProcessorEditor::handleJavaScriptMessage (const juce::var& message)
{
    std::cout << "=== RECEIVED MESSAGE FROM JAVASCRIPT ===" << std::endl;
    DBG("=== RECEIVED MESSAGE FROM JAVASCRIPT ===");

    juce::var msg = message;
    if (msg.isString())
    {
        juce::var parsed = juce::JSON::parse (msg.toString());
        if (parsed.isObject())
            msg = parsed;
    }
    if (! msg.isObject())
    {
        std::cout << "Message is not an object" << std::endl;
        DBG("Message is not an object");
        return;
    }

    auto obj = msg.getDynamicObject();
    if (obj == nullptr)
    {
        std::cout << "Message dynamic object is nullptr" << std::endl;
        DBG("Message dynamic object is nullptr");
        return;
    }

    if (obj->hasProperty ("payload") && obj->getProperty ("payload").isObject())
    {
        msg = obj->getProperty ("payload");
        obj = msg.getDynamicObject();
        if (obj == nullptr)
            return;
    }
"""
    return text.replace(old, new, 1)


def patch_handle_singularity(text: str) -> str | None:
    old = """    if (!message.isObject())
    {
        std::cout << "Message is not an object" << std::endl;
        DBG("Message is not an object");
        return;
    }

    auto obj = message.getDynamicObject();
    if (obj == nullptr)
    {
        std::cout << "Message dynamic object is nullptr" << std::endl;
        DBG("Message dynamic object is nullptr");
        return;
    }
"""
    if "SingularityAudioProcessorEditor::handleJavaScriptMessage" not in text or old not in text:
        return None
    new = """    juce::var msg = message;
    if (msg.isString())
    {
        juce::var parsed = juce::JSON::parse (msg.toString());
        if (parsed.isObject())
            msg = parsed;
    }
    if (! msg.isObject())
    {
        std::cout << "Message is not an object" << std::endl;
        DBG("Message is not an object");
        return;
    }

    auto obj = msg.getDynamicObject();
    if (obj == nullptr)
    {
        std::cout << "Message dynamic object is nullptr" << std::endl;
        DBG("Message dynamic object is nullptr");
        return;
    }

    if (obj->hasProperty ("payload") && obj->getProperty ("payload").isObject())
    {
        msg = obj->getProperty ("payload");
        obj = msg.getDynamicObject();
        if (obj == nullptr)
            return;
    }
"""
    return text.replace(old, new, 1)


def patch_handle_fabric(text: str) -> str | None:
    old = """    // JUCE sends the message as an already-parsed object from JSON::fromString()
    // So it should already be an object, not a string
    if (!message.isObject())
    {
        std::cout << "Message is not an object, returning" << std::endl;
        DBG("Message is not an object, returning");
        return;
    }

    auto obj = message.getDynamicObject();
    if (obj == nullptr)
    {
        DBG("Could not get dynamic object");
        return;
    }
"""
    if old not in text:
        return None
    new = """    juce::var msg = message;
    if (msg.isString())
    {
        juce::var parsed = juce::JSON::parse (msg.toString());
        if (parsed.isObject())
            msg = parsed;
    }
    if (! msg.isObject())
    {
        std::cout << "Message is not an object, returning" << std::endl;
        DBG("Message is not an object, returning");
        return;
    }

    auto obj = msg.getDynamicObject();
    if (obj == nullptr)
    {
        DBG("Could not get dynamic object");
        return;
    }

    if (obj->hasProperty ("payload") && obj->getProperty ("payload").isObject())
    {
        msg = obj->getProperty ("payload");
        obj = msg.getDynamicObject();
        if (obj == nullptr)
            return;
    }
"""
    return text.replace(old, new, 1)


def patch_plugin_editor_h(path: Path) -> None:
    t = path.read_text(encoding="utf-8", errors="replace")
    if "webViewInitialized" in t:
        return
    needle = "    std::unique_ptr<WebBrowserWithCallbacks> webView;"
    if needle not in t:
        raise RuntimeError(f"webView member not found in {path}")
    repl = needle + "\n    std::unique_ptr<juce::Label> errorLabel;\n    bool webViewInitialized = false;"
    t = t.replace(needle, repl, 1)
    path.write_text(t, encoding="utf-8")


def patch_load_html_guard(text: str) -> str:
    """Insert guard immediately after void X::loadHTMLFile opening brace."""
    pat = re.compile(
        r"(void \w+AudioProcessorEditor::loadHTMLFile \(const juce::File& htmlFile\)\s*\n\{\s*\n)",
    )
    m = pat.search(text)
    if not m:
        return text
    insert = m.group(1) + """    if (! webViewInitialized || webView == nullptr)
        return;

"""
    if "loadHTMLFile" in text[m.start() : m.start() + 300] and "if (! webViewInitialized || webView == nullptr)" in text[
        m.start() : m.start() + 300
    ]:
        return text
    return text[: m.start()] + insert + text[m.end() :]


def inject_index_bridge(path: Path) -> None:
    t = path.read_text(encoding="utf-8", errors="replace")
    if BRIDGE_MARK in t:
        return
    m = re.search(r"<body[^>]*>", t, re.IGNORECASE)
    if not m:
        print(f"skip index (no body): {path}", file=sys.stderr)
        return
    t = t[: m.end()] + "\n" + BRIDGE_INLINE + t[m.end() :]
    path.write_text(t, encoding="utf-8")


def patch_jucer(path: Path) -> None:
    t = path.read_text(encoding="utf-8", errors="replace")
    if "JUCE_USE_WIN_WEBVIEW2" in t:
        return

    def inject_juce_options(match: re.Match) -> str:
        tag = match.group(0)
        if "JUCE_USE_WIN_WEBVIEW2" in tag:
            return tag
        return tag[:-2].rstrip() + ' JUCE_USE_WIN_WEBVIEW2="1" />'

    t = re.sub(r"<JUCEOPTIONS[^>]*/>", inject_juce_options, t)

    def add_def(m: re.Match) -> str:
        full = m.group(0)
        if "JUCE_USE_WIN_WEBVIEW2" in full:
            return full
        return re.sub(
            r'(preprocessorDefs="[^"]*)(")',
            r"\1 JUCE_USE_WIN_WEBVIEW2=1\2",
            full,
            count=1,
        )

    t = re.sub(r"<CONFIGURATION[^>]+preprocessorDefs=\"[^\"]+\"[^>]*>", add_def, t)
    path.write_text(t, encoding="utf-8")


def process_fabric_ctor_manually() -> None:
    path = ROOT / "MandelbrotSet/Fabric/Source/PluginEditor.cpp"
    t = path.read_text(encoding="utf-8", errors="replace")
    cls = "FabricAudioProcessorEditor"
    start = t.find('    std::cout << "Creating WebView..."')
    if start < 0:
        start = t.find("    // Create WebView with native integration")
    end = t.find("    // Check authorization first, then load appropriate UI", start)
    if start < 0 or end < 0:
        raise RuntimeError("Fabric ctor bounds not found")
    new_t = t[:start] + ctor_substitute(cls) + t[end:]
    path.write_text(new_t, encoding="utf-8")


def main() -> None:
    for ui in ROOT.rglob("UI/index.html"):
        if "_Shared" in str(ui):
            continue
        inject_index_bridge(ui)

    for jucer in ROOT.rglob("*.jucer"):
        if "_Shared" in str(jucer):
            continue
        tj = jucer.read_text(encoding="utf-8", errors="replace")
        if "audioplug" not in tj and "audioplugin" not in tj.lower():
            continue
        if "PluginEditor.cpp" not in tj:
            continue
        patch_jucer(jucer)

    for cpp in discover_editor_cpps():
        cls = editor_class_name(cpp.read_text(encoding="utf-8", errors="replace"))
        if not cls:
            print(f"skip (no class): {cpp}", file=sys.stderr)
            continue
        patch_plugin_editor_h(cpp.parent / "PluginEditor.h")

        if "Fabric" in str(cpp):
            process_fabric_ctor_manually()
            t = cpp.read_text(encoding="utf-8", errors="replace")
        else:
            t = cpp.read_text(encoding="utf-8", errors="replace")
            t = replace_webview_ctor(t, cls)

        t = patch_destructor(t, cls)
        try:
            t = patch_resized(t)
        except RuntimeError as e:
            print(f"{cpp}: {e}", file=sys.stderr)

        t = patch_load_html_guard(t)

        ct = patch_handle_chip_tune(t)
        if ct is not None:
            t = ct
        st = patch_handle_singularity(t)
        if st is not None:
            t = st
        ft = patch_handle_fabric(t)
        if ft is not None:
            t = ft
        t = patch_handle_javascript_simple(t)

        t = re.sub(
            r"^    webView->goToURL\(fileURL\);$",
            "    if (webView != nullptr && webViewInitialized)\n"
            "        webView->goToURL(fileURL);",
            t,
            flags=re.MULTILINE,
        )

        cpp.write_text(t, encoding="utf-8")

    print("Done.")


def verify_webview_plugins() -> int:
    """
    @brief Confirms every Orbitals WebView plugin uses OrbitalsWebViewHost and local UI assets.
    @returns 0 if all pass, 1 if any check fails.
    """
    failures: list[tuple[str, list[str]]] = []
    plugin_count = 0

    for ju in sorted(ROOT.rglob("*.jucer"), key=lambda p: str(p)):
        if "_Shared" in str(ju):
            continue
        jt = ju.read_text(encoding="utf-8", errors="replace")
        if "audioplug" not in jt and "audioplugin" not in jt.lower():
            continue
        if "PluginEditor.cpp" not in jt:
            continue
        base = ju.parent
        ed = base / "Source" / "PluginEditor.cpp"
        eh = base / "Source" / "PluginEditor.h"
        ix = base / "UI" / "index.html"
        app_js = base / "UI" / "app.js"
        if not ed.exists():
            continue

        et = ed.read_text(encoding="utf-8", errors="replace")
        if "OrbitalsWebViewHost" not in et and "WebBrowserWithCallbacks" not in et:
            continue

        plugin_count += 1
        reasons: list[str] = []

        if "OrbitalsWebViewHost" not in et:
            reasons.append("PluginEditor.cpp must use OrbitalsWebViewHost")
        if "loadHTMLFile" in et:
            reasons.append("remove legacy loadHTMLFile inlining")
        if "webViewInitialized" in et or "webView->" in et or "std::unique_ptr<juce::WebBrowserComponent>" in et:
            reasons.append("remove legacy webView / webViewInitialized members")
        if "webViewHost.initialize" not in et:
            reasons.append("missing webViewHost.initialize()")
        if "webViewHost.dispatchInitialNavigation" not in et:
            reasons.append("missing webViewHost.dispatchInitialNavigation()")
        if "webViewHost.shutdown()" not in et:
            reasons.append("missing webViewHost.shutdown() in destructor")
        if 'hasProperty ("payload")' not in et and "hasProperty (\"payload\")" not in et:
            reasons.append("handleJavaScriptMessage should unwrap payload envelope")

        if eh.exists():
            ht = eh.read_text(encoding="utf-8", errors="replace")
            if "OrbitalsWebViewHost" not in ht:
                reasons.append("PluginEditor.h missing OrbitalsWebViewHost member")

        if "JUCE_USE_WIN_WEBVIEW2" not in jt:
            reasons.append("jucer missing JUCE_USE_WIN_WEBVIEW2")
        if "OrbitalsWebViewHost.cpp" not in jt:
            reasons.append("jucer missing OrbitalsWebViewHost.cpp")
        if 'file="UI/orbitals-juce-bridge.js"' not in jt and "orbitals-juce-bridge.js" not in jt:
            reasons.append("jucer missing orbitals-juce-bridge.js resource")
        if "../../_Shared/UI/" in jt:
            reasons.append("jucer still references ../../_Shared/UI/ paths")

        if ix.exists():
            it = ix.read_text(encoding="utf-8", errors="replace")
            if "../../_Shared/UI/" in it or "../../_Shared/Assets/" in it:
                reasons.append("index.html still uses _Shared relative paths")
            if "orbitals-juce-bridge.js" not in it and "postMessageToJUCE" not in it:
                reasons.append("index.html missing bridge (orbitals-juce-bridge.js or inline snippet)")
        else:
            reasons.append("missing UI/index.html")

        if app_js.exists():
            aj = app_js.read_text(encoding="utf-8", errors="replace")
            if "window.chrome?.webview" in aj or "if (window.chrome?.webview)" in aj:
                reasons.append("app.js uses broken chrome.webview guard")
            if re.search(r"if\s*\(\s*window\.juce\s*\)", aj):
                reasons.append("app.js uses broken window.juce guard")
            if "alert(" in aj and "Sent to C++" in aj:
                reasons.append("app.js contains debug alert() for bridge")
        else:
            reasons.append("missing UI/app.js")

        if reasons:
            failures.append((str(base.relative_to(ROOT)), reasons))

    if failures:
        for path, rs in failures:
            print(f"FAIL {path}:", file=sys.stderr)
            for r in rs:
                print(f"  - {r}", file=sys.stderr)
        return 1

    print(f"OK: {plugin_count} WebView plugins pass OrbitalsWebViewHost migration checks.")
    return 0


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "verify":
        raise SystemExit(verify_webview_plugins())
    main()
