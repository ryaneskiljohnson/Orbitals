/**
 * @fileoverview JUCE native bridge helpers for Orbitals plugin WebViews.
 * @module Orbitals/_Shared/UI
 * @note Matches CymaSynth / JUCE 8 native integration (WKWebView + WebView2).
 */

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
        } catch (e) {
            return false;
        }
    }

    window.postMessageToJUCE = function(data) {
        sendToHost(data);
    };

    window.__JUCE__ = window.__JUCE__ || {
        postMessage: sendToHost,
        backend: {
            emitEvent: function(name, data) {
                if (name === 'message') return sendToHost(data);
                return sendToHost({ event: name, data: data });
            }
        }
    };

    /**
     * @brief Send APVTS parameter change to C++ (macOS and Windows).
     * @param {string} param Parameter ID.
     * @param {*} value Raw parameter value.
     */
    window.sendToJUCE = function(param, value) {
        if (typeof window.postMessageToJUCE !== 'function') return;
        window.postMessageToJUCE({ type: 'parameterChange', parameter: param, value: value });
    };

    /**
     * @brief Open standalone audio/MIDI settings from UI.
     */
    window.openPluginSettings = function() {
        if (typeof window.postMessageToJUCE !== 'function') return;
        window.postMessageToJUCE({ type: 'openSettings' });
    };
})();
