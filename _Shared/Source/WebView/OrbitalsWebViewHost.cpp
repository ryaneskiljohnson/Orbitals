/*
  @fileoverview Shared CymaSynth-style WebView host implementation.
  @module Orbitals/_Shared/Source/WebView
  @note Platform parity with CymaSynth PluginEditor.cpp:
        Mac — withResourceProvider + injected index.html (cache-bust query on juce.backend URL).
        Windows — WebView2 temp mirror (OrbitalsWebUI_*) for main UI; OrbitalsAuth_* temp auth.html
        (no query on file://); per-instance Temp/OrbitalsWebView/<uuid> user-data folder.
*/

#include "OrbitalsWebViewHost.h"
#include "OrbitalsWebViewHtmlInject.h"

#include <cstring>

namespace
{
static const char* kPaintHealthCheckScript = R"(
(function() {
    if (!document.body) return false;
    var style = window.getComputedStyle(document.body);
    var bg = style.backgroundColor;
    return bg === 'rgb(0, 0, 0)' || bg === 'black' || bg.indexOf('0, 0, 0') >= 0;
})();
)";

juce::String getMimeTypeForPath(const juce::String& path)
{
    if (path.endsWithIgnoreCase(".html") || path.endsWithIgnoreCase(".htm"))
        return "text/html; charset=utf-8";
    if (path.endsWithIgnoreCase(".js"))
        return "application/javascript; charset=utf-8";
    if (path.endsWithIgnoreCase(".css"))
        return "text/css; charset=utf-8";
    if (path.endsWithIgnoreCase(".png"))
        return "image/png";
    if (path.endsWithIgnoreCase(".jpg") || path.endsWithIgnoreCase(".jpeg"))
        return "image/jpeg";
    if (path.endsWithIgnoreCase(".svg"))
        return "image/svg+xml";
    if (path.endsWithIgnoreCase(".woff2"))
        return "font/woff2";
    if (path.endsWithIgnoreCase(".woff"))
        return "font/woff";
    if (path.endsWithIgnoreCase(".ttf"))
        return "font/ttf";
    return "application/octet-stream";
}

bool filePathIsUnderWebRoot(const juce::File& candidate, const juce::File& root)
{
    juce::String rootPath = root.getFullPathName();
    const auto sep = juce::File::getSeparatorChar();
    if (!rootPath.endsWithChar(sep))
        rootPath += juce::String::charToString(sep);
    return candidate.getFullPathName().startsWithIgnoreCase(rootPath);
}

juce::String normalizedProviderPath(const juce::String& url)
{
    juce::String path = url;
    if (path.startsWithIgnoreCase("http://") || path.startsWithIgnoreCase("https://")
        || path.startsWithIgnoreCase("file://"))
    {
        path = path.fromFirstOccurrenceOf("://", false, false);
        const int slash = path.indexOfChar('/');
        if (slash >= 0)
            path = path.substring(slash);
    }
    path = path.upToFirstOccurrenceOf("?", false, false);
    return path.trimCharactersAtStart("/");
}

juce::File tryFindDevIndexHtml(const juce::File& devPluginDirectory)
{
    return devPluginDirectory.getChildFile("UI").getChildFile("index.html");
}

juce::File tryFindBundledUiDirectory()
{
    const auto executableFile = OrbitalsPluginPaths::getModuleBinaryFileForResourceDiscovery();

#if JUCE_MAC
    juce::File resourcesDir;
    if (OrbitalsPluginPaths::tryResolveMacBundleResourcesDirFromExecutable(executableFile, resourcesDir))
    {
        const auto webDir = resourcesDir.getChildFile("web");
        if (webDir.isDirectory())
            return webDir;

        if (resourcesDir.getChildFile("index.html").existsAsFile())
            return resourcesDir;
    }
#endif

    juce::File currentDir = executableFile.getParentDirectory();
    for (int depth = 0; depth < 50 && currentDir.exists(); ++depth)
    {
        if (currentDir.getFileName() == "Contents")
        {
            const auto resourcesDir = currentDir.getChildFile("Resources");
            const auto webDir = resourcesDir.getChildFile("web");
            if (webDir.isDirectory())
                return webDir;

            if (resourcesDir.getChildFile("index.html").existsAsFile())
                return resourcesDir;
        }

#if JUCE_WINDOWS
        if (executableFile.getFileExtension() == ".exe"
            && currentDir.getChildFile("index.html").existsAsFile())
        {
            return currentDir;
        }
#endif

        auto parent = currentDir.getParentDirectory();
        if (parent.getFullPathName() == currentDir.getFullPathName())
            break;
        currentDir = parent;
    }

    return {};
}

juce::File tryFindBundledIndexHtml()
{
    const auto uiDir = tryFindBundledUiDirectory();
    if (uiDir.isDirectory())
    {
        const auto htmlFile = uiDir.getChildFile("index.html");
        if (htmlFile.existsAsFile())
            return htmlFile;
    }

    return {};
}
} // namespace

#if JUCE_WINDOWS
juce::File OrbitalsWebViewHost::createUniqueWebView2UserDataFolder()
{
    return juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("OrbitalsWebView")
        .getChildFile(juce::Uuid().toString());
}

bool OrbitalsWebViewHost::copyWebFolderToTempBundle(const juce::File& sourceWebDir,
                                                    const juce::File& destBundleDir)
{
    if (!sourceWebDir.isDirectory())
        return false;

    if (destBundleDir.exists())
        (void) destBundleDir.deleteRecursively();

    return sourceWebDir.copyDirectoryTo(destBundleDir) && destBundleDir.isDirectory();
}
#endif

std::optional<juce::WebBrowserComponent::Resource> OrbitalsWebViewHost::getResourceForUrl(
    const juce::String& url,
    OrbitalsWebViewState& webState)
{
    if (!webState.webUiResourceDirectory.isDirectory())
        return std::nullopt;

    juce::String path = normalizedProviderPath(url);
    if (path.containsIgnoreCase(".."))
        return std::nullopt;

    if (path.isEmpty() || path.equalsIgnoreCase("index.html"))
    {
        if (webState.webUiInjectedIndexHtml.isEmpty())
            return std::nullopt;

        juce::WebBrowserComponent::Resource resource;
        resource.mimeType = "text/html; charset=utf-8";
        juce::CharPointer_UTF8 utf8(webState.webUiInjectedIndexHtml.toRawUTF8());
        const std::string text(utf8.getAddress());
        resource.data.resize(text.size());
        std::memcpy(resource.data.data(), text.data(), text.size());
        return resource;
    }

    juce::File file = webState.webUiResourceDirectory.getChildFile(path);
    if (!filePathIsUnderWebRoot(file, webState.webUiResourceDirectory))
        return std::nullopt;
    if (!file.existsAsFile())
        return std::nullopt;

    juce::MemoryBlock block;
    if (!file.loadFileAsData(block))
        return std::nullopt;

    juce::WebBrowserComponent::Resource resource;
    resource.mimeType = getMimeTypeForPath(path);
    resource.data.resize(block.getSize());
    std::memcpy(resource.data.data(), block.getData(), block.getSize());
    return resource;
}

juce::File OrbitalsWebViewHost::resolveUiDirectory(const OrbitalsWebViewHostConfig& hostConfig)
{
#if JUCE_DEBUG
    const auto devUiDir = hostConfig.devPluginDirectory.getChildFile("UI");
    if (devUiDir.isDirectory())
        return devUiDir;
#endif

    const auto bundledUiDir = tryFindBundledUiDirectory();
    if (bundledUiDir.isDirectory())
        return bundledUiDir;

    return hostConfig.devPluginDirectory.getChildFile("UI");
}

juce::File OrbitalsWebViewHost::resolveIndexHtmlFile(const OrbitalsWebViewHostConfig& hostConfig)
{
#if JUCE_DEBUG
    const auto devHtml = tryFindDevIndexHtml(hostConfig.devPluginDirectory);
    if (devHtml.existsAsFile())
        return devHtml;
#endif

    const auto bundled = tryFindBundledIndexHtml();
    if (bundled.existsAsFile())
        return bundled;

    return tryFindDevIndexHtml(hostConfig.devPluginDirectory);
}

void OrbitalsWebViewHost::dispatchJavaScriptMessage(
    const juce::var& message,
    const std::function<void(const juce::var&)>& handler)
{
    if (!handler)
        return;

    if (!juce::MessageManager::existsAndIsCurrentThread())
    {
        juce::MessageManager::callAsync([message, handler]() { dispatchJavaScriptMessage(message, handler); });
        return;
    }

    juce::var msg = message;
    if (msg.isString())
    {
        const juce::var parsed = juce::JSON::parse(msg.toString());
        if (parsed.isObject())
            msg = parsed;
    }

    if (!msg.isObject())
        return;

    auto* obj = msg.getDynamicObject();
    if (obj == nullptr)
        return;

    if (obj->hasProperty("payload") && obj->getProperty("payload").isObject())
    {
        msg = obj->getProperty("payload");
        obj = msg.getDynamicObject();
        if (obj == nullptr)
            return;
    }

    handler(msg);
}

bool OrbitalsWebViewHost::initialize(juce::AudioProcessorEditor& editor,
                                     const OrbitalsWebViewHostConfig& hostConfig,
                                     std::function<void(const juce::var&)> onJavaScriptMessage)
{
    config = hostConfig;
    jsMessageHandler = std::move(onJavaScriptMessage);
    editorComponent = &editor;

    juce::WebBrowserComponent::Options options;

#if JUCE_WINDOWS
    state.webView2UserDataFolder = createUniqueWebView2UserDataFolder();
    options = options.withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
                     .withWinWebView2Options(
                         juce::WebBrowserComponent::Options::WinWebView2{}.withUserDataFolder(
                             state.webView2UserDataFolder));
#else
    options = options.withResourceProvider(
        [this](const juce::String& resourceUrl) -> std::optional<juce::WebBrowserComponent::Resource>
        {
            if (editorComponent.getComponent() == nullptr)
                return std::nullopt;
            return getResourceForUrl(resourceUrl, state);
        });
#endif

    options = options.withNativeIntegrationEnabled(true)
                     .withKeepPageLoadedWhenBrowserIsHidden()
                     .withEventListener("message",
                                        [this](const juce::var& message)
                                        {
                                            if (editorComponent.getComponent() == nullptr)
                                                return;
                                            dispatchJavaScriptMessage(message, jsMessageHandler);
                                        });

    try
    {
        webView = std::make_unique<OrbitalsWebBrowserCallbacks>(options);
        if (webView == nullptr)
        {
            webViewInitialized = false;
            return false;
        }

        webViewInitialized = true;
        editor.addChildComponent(webView.get());
        webView->setBounds(editor.getLocalBounds());
        webView->setVisible(false);

        webView->onPageFinishedLoading = [this](const juce::String& url)
        {
            juce::MessageManager::callAsync([this, url]()
            {
                if (editorComponent.getComponent() == nullptr)
                    return;
                if (auto* editorPtr = editorComponent.getComponent())
                    handlePageFinished(url, *editorPtr);
            });
        };

        webView->onPageLoadHadNetworkError = [](const juce::String& info)
        {
            DBG("OrbitalsWebViewHost network error: " + info);
        };

        navigateToDarkPlaceholder();
        return true;
    }
    catch (...)
    {
        webViewInitialized = false;
        webView.reset();
    }

#if JUCE_WINDOWS
    if (!webViewInitialized && state.webView2UserDataFolder.isDirectory()
        && state.webView2UserDataFolder.getParentDirectory().getFileName() == "OrbitalsWebView")
    {
        state.webView2UserDataFolder.deleteRecursively();
    }
#endif

    if (!webViewInitialized)
    {
        errorLabel = std::make_unique<juce::Label>(
            "error",
            "WebView initialization failed.\nThe plugin UI cannot be displayed.\n\nOn Windows, WebView2 must be available.");
        errorLabel->setJustificationType(juce::Justification::centred);
        errorLabel->setColour(juce::Label::textColourId, juce::Colours::white);
        errorLabel->setBounds(editor.getLocalBounds());
        editor.addAndMakeVisible(errorLabel.get());
    }
    return webViewInitialized;
}

void OrbitalsWebViewHost::cleanupTransientWebState()
{
    state.webUiInjectedIndexHtml.clear();
    state.webUiResourceDirectory = juce::File();

#if JUCE_WINDOWS
    if (state.webUiWindowsTempBundle.isDirectory())
        state.webUiWindowsTempBundle.deleteRecursively();
    state.webUiWindowsTempBundle = juce::File();
#endif
}

void OrbitalsWebViewHost::shutdown()
{
    if (webView != nullptr)
    {
        webView->onPageFinishedLoading = nullptr;
        webView->onPageLoadHadNetworkError = nullptr;
    }

    cleanupTransientWebState();

#if JUCE_WINDOWS
    if (state.webView2UserDataFolder.isDirectory()
        && state.webView2UserDataFolder.getParentDirectory().getFileName() == "OrbitalsWebView")
    {
        state.webView2UserDataFolder.deleteRecursively();
    }
    state.webView2UserDataFolder = juce::File();
#endif
}

void OrbitalsWebViewHost::resized(juce::Component& editor)
{
    if (!webViewInitialized || webView == nullptr)
        return;
    webView->setBounds(editor.getLocalBounds());
}

void OrbitalsWebViewHost::navigateToDarkPlaceholder()
{
    if (!webViewInitialized || webView == nullptr)
        return;

    const juce::String placeholderHtml(
        "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
        "<style>html,body{margin:0;padding:0;width:100%;height:100%;background:#0a0a0f!important;overflow:hidden;}</style>"
        "</head><body></body></html>");
    const juce::String escaped = juce::URL::addEscapeChars(placeholderHtml, true);
    webView->goToURL("data:text/html;charset=utf-8," + escaped);
}

void OrbitalsWebViewHost::dispatchInitialNavigation(bool authorized)
{
    if (state.initialNavigationDispatched.exchange(true))
        return;

    juce::MessageManager::callAsync([this, authorized]() { reloadSurface(authorized); });
}

void OrbitalsWebViewHost::reloadSurface(bool authorized)
{
    if (authorized)
        loadWebUI();
    else
        loadAuthScreen();
}

void OrbitalsWebViewHost::loadWebUI()
{
    lastSurface = Surface::webUi;
    const auto htmlFile = resolveIndexHtmlFile(config);
    if (!htmlFile.existsAsFile())
    {
        DBG("OrbitalsWebViewHost: index.html not found for " + config.pluginDisplayName);
        return;
    }

    loadHtmlContent(htmlFile, Surface::webUi);
}

void OrbitalsWebViewHost::loadAuthScreen()
{
    if (!webViewInitialized || webView == nullptr)
        return;

    lastSurface = Surface::auth;

    juce::MessageManager::callAsync([this]()
    {
        if (!webViewInitialized || webView == nullptr || editorComponent.getComponent() == nullptr)
            return;

        const auto uiDir = resolveUiDirectory(config);
        const juce::String authHtml = OrbitalsWebViewHtmlInject::buildAuthHtml(
            config.pluginDisplayName,
            OrbitalsWebViewHtmlInject::fileToDataUrl(uiDir.getChildFile("plugin-background.png")),
            OrbitalsWebViewHtmlInject::fileToDataUrl(uiDir.getChildFile("nnaudio-logo.png")));

        beginNewNavigation();

#if JUCE_WINDOWS
        const auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                 .getChildFile("OrbitalsAuth_" + juce::String(juce::Time::currentTimeMillis()));
        tempDir.createDirectory();
        const auto tempFile = tempDir.getChildFile("auth.html");
        if (!tempFile.replaceWithText(authHtml))
            return;

        const juce::String filePath = tempFile.getFullPathName().replace(" ", "%20");
        webView->goToURL("file://" + filePath);
#else
        juce::File providerRoot = uiDir;
        if (!providerRoot.isDirectory())
        {
            providerRoot = juce::File::getSpecialLocation(juce::File::tempDirectory)
                               .getChildFile("OrbitalsAuthRp_" + juce::Uuid().toString());
            (void) providerRoot.createDirectory();
        }

        state.webUiResourceDirectory = providerRoot;
        state.webUiInjectedIndexHtml = authHtml;

        juce::String root = juce::WebBrowserComponent::getResourceProviderRoot();
        if (!root.endsWithChar('/'))
            root += "/";

        const auto timestamp = juce::Time::currentTimeMillis();
        const auto random = juce::Random::getSystemRandom().nextInt(1000000);
        webView->goToURL(root + "index.html?t=" + juce::String(timestamp) + "&r=" + juce::String(random));
#endif

        if (auto* editorPtr = editorComponent.getComponent())
            scheduleLoadFallback(state.navigationGeneration.load(), *editorPtr);
    });
}

juce::String OrbitalsWebViewHost::prepareHtmlFromFile(const juce::File& htmlFile) const
{
    juce::String htmlContent = htmlFile.loadFileAsString();
    OrbitalsWebViewHtmlInject::injectBlackFlashStyles(htmlContent);
    OrbitalsWebViewHtmlInject::injectBodyScripts(htmlContent);
    return htmlContent;
}

bool OrbitalsWebViewHost::loadHtmlContent(const juce::File& htmlFile, Surface surface)
{
    if (!webViewInitialized || webView == nullptr)
        return false;

    const juce::String htmlContent = prepareHtmlFromFile(htmlFile);
    const juce::File webDir = htmlFile.getParentDirectory();
    navigateToInjectedHtml(htmlContent, surface, webDir);
    return true;
}

void OrbitalsWebViewHost::navigateToInjectedHtml(const juce::String& htmlContent,
                                                 Surface surface,
                                                 const juce::File& webDir)
{
    if (!webViewInitialized || webView == nullptr || !webDir.isDirectory())
        return;

    beginNewNavigation();
    juce::ignoreUnused(surface);

#if JUCE_WINDOWS
    state.webUiResourceDirectory = juce::File();
    state.webUiInjectedIndexHtml.clear();

    if (state.webUiWindowsTempBundle.isDirectory())
        state.webUiWindowsTempBundle.deleteRecursively();

    state.webUiWindowsTempBundle = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                       .getChildFile("OrbitalsWebUI_" + juce::String(juce::Time::currentTimeMillis()));

    if (!copyWebFolderToTempBundle(webDir, state.webUiWindowsTempBundle))
    {
        DBG("OrbitalsWebViewHost: failed to mirror UI folder on Windows");
        return;
    }

    const juce::File tempIndex(state.webUiWindowsTempBundle.getChildFile("index.html"));
    if (!tempIndex.replaceWithText(htmlContent))
        return;

    const juce::String filePath = tempIndex.getFullPathName().replace("\\", "/").replace(" ", "%20");
    webView->goToURL("file:///" + filePath);
#else
    state.webUiResourceDirectory = webDir;
    state.webUiInjectedIndexHtml = htmlContent;

    juce::String root = juce::WebBrowserComponent::getResourceProviderRoot();
    if (!root.endsWithChar('/'))
        root += "/";

    const auto timestamp = juce::Time::currentTimeMillis();
    const auto random = juce::Random::getSystemRandom().nextInt(1000000);
    webView->goToURL(root + "index.html?t=" + juce::String(timestamp) + "&r=" + juce::String(random));
#endif

    if (auto* editorPtr = editorComponent.getComponent())
        scheduleLoadFallback(state.navigationGeneration.load(), *editorPtr);
}

void OrbitalsWebViewHost::beginNewNavigation()
{
    state.loadFailureUiShown.store(false);
    state.navigationGeneration.fetch_add(1u);
    state.fallbackPending = false;
}

void OrbitalsWebViewHost::scheduleLoadFallback(uint32_t generation, juce::Component& editor)
{
    if (state.fallbackPending && state.fallbackPendingGeneration == generation)
        return;

    state.fallbackPending = true;
    state.fallbackPendingGeneration = generation;

    juce::Component::SafePointer<juce::Component> safeEditor(&editor);

    juce::Timer::callAfterDelay(5000, [this, safeEditor, generation]()
    {
        state.fallbackPending = false;
        if (state.navigationGeneration.load() != generation)
            return;
        if (!webViewInitialized || webView == nullptr || webView->isVisible())
            return;

        webView->evaluateJavascript(
            kPaintHealthCheckScript,
            [this, safeEditor, generation](const juce::WebBrowserComponent::EvaluationResult& result)
            {
                juce::ignoreUnused(result);
                if (state.navigationGeneration.load() != generation)
                    return;
                if (!webViewInitialized || webView == nullptr)
                    return;
                if (auto* editorPtr = safeEditor.getComponent())
                    showWebViewAfterLoad(generation, *editorPtr);
            });
    });

    juce::Timer::callAfterDelay(3000, [this, safeEditor, generation]()
    {
        if (state.navigationGeneration.load() != generation)
            return;
        if (!webViewInitialized || webView == nullptr || webView->isVisible())
            return;
        if (auto* editorPtr = safeEditor.getComponent())
            showWebViewAfterLoad(generation, *editorPtr);
    });
}

void OrbitalsWebViewHost::showWebViewAfterLoad(uint32_t generation, juce::Component& editor)
{
    if (state.navigationGeneration.load() != generation)
        return;
    if (!webViewInitialized || webView == nullptr)
        return;

    if (!webView->isVisible())
    {
        webView->setBounds(editor.getLocalBounds());
        webView->setVisible(true);
#if JUCE_WINDOWS && !JucePlugin_Build_Standalone
        webView->repaint();
#endif
        editor.repaint();
    }
}

void OrbitalsWebViewHost::handlePageFinished(const juce::String& url, juce::Component& editor)
{
    if (!webViewInitialized || webView == nullptr)
        return;

    if (url.startsWithIgnoreCase("data:") || url.startsWithIgnoreCase("about:"))
        return;

    const uint32_t generation = state.navigationGeneration.load();
    juce::Component::SafePointer<juce::Component> safeEditor(&editor);

    juce::Timer::callAfterDelay(400, [this, safeEditor, generation]()
    {
        if (state.navigationGeneration.load() != generation)
            return;
        if (auto* editorPtr = safeEditor.getComponent())
            showWebViewAfterLoad(generation, *editorPtr);
    });
}

void OrbitalsWebViewHost::evaluateJavascript(const juce::String& script) const
{
    if (webViewInitialized && webView != nullptr && webView->isVisible())
        webView->evaluateJavascript(script);
}

bool OrbitalsWebViewHost::isVisible() const
{
    return webViewInitialized && webView != nullptr && webView->isVisible();
}
