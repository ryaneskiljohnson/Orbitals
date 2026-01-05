# Synth - JUCE Project with CSS UI

A JUCE-based audio synthesizer application with CSS-styled web frontend support.

## Features

- **CSS-based UI**: Uses JUCE 8's WebView component to create beautiful, modern UIs with HTML/CSS/JavaScript
- **Native JUCE UI**: Falls back to native JUCE components if WebView is not available
- **Modern Styling**: Example gradient UI with glassmorphism effects

## Prerequisites

- CMake 3.22 or higher
- C++17 compatible compiler
- JUCE framework (version 8.0+ recommended for WebView support)

## Setup

### Option 1: Using JUCE as a Git Submodule (Recommended)

1. Add JUCE as a submodule:
```bash
git submodule add https://github.com/juce-framework/JUCE.git JUCE
git submodule update --init --recursive
```

### Option 2: Download JUCE Manually

1. Download JUCE from [juce.com](https://juce.com) or clone from GitHub
2. Place the JUCE folder in the project root directory

## Building

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Configure with CMake:
```bash
cmake ..
```

3. Build:
```bash
cmake --build .
```

Or use your IDE's build system after opening the CMake project.

## Using CSS in JUCE

This project demonstrates three ways to use CSS with JUCE:

### 1. JUCE 8 WebView (Implemented)

The project uses `juce::WebBrowserComponent` to embed HTML/CSS/JavaScript directly in your JUCE app. The UI files are in the `UI/` directory:

- `UI/ui.html` - Main HTML structure
- `UI/styles.css` - CSS styling
- `UI/app.js` - JavaScript logic

The `MainComponent` automatically loads the web UI if JUCE 8+ is available.

### 2. Alternative: JIVE

JIVE is a JUCE extension that provides CSS-like styling:
- GitHub: https://github.com/ImJimmi/JIVE
- Uses `juce::ValueTree` for declarative UI
- CSS-like styling with `juce::var`/`juce::DynamicObject`

### 3. Alternative: foleys_gui_magic

A GUI builder with cascading stylesheet system:
- Website: https://foleysfinest.com/PluginGuiMagic/
- Drag-and-drop editor
- DOM model similar to HTML

## Project Structure

```
Synth/
├── CMakeLists.txt          # Main CMake configuration
├── Synth.jucer             # Projucer project file
├── Source/
│   ├── Main.cpp            # Application entry point
│   ├── MainComponent.h    # Main UI component header
│   └── MainComponent.cpp  # Main UI component implementation
├── UI/                     # Web UI files (CSS/HTML/JS)
│   ├── ui.html
│   ├── styles.css
│   └── app.js
├── JUCE/                   # JUCE framework (submodule or manual)
└── README.md               # This file
```

## Communication Between Web UI and JUCE

To send messages from JavaScript to JUCE:

```javascript
// In your JavaScript (app.js)
window.chrome.webview.postMessage({
    type: 'volume',
    value: 75
});
```

To receive messages in JUCE C++:

```cpp
// In MainComponent
webView->bind("onMessage", [this](const juce::var& message) {
    // Handle message from JavaScript
    auto type = message["type"].toString();
    auto value = message["value"];
    // Update your audio engine
});
```

## Development

- **Web UI**: Edit files in `UI/` directory to customize the CSS styling
- **Audio Engine**: Add your synthesizer logic in `Source/MainComponent.cpp`
- **Native UI**: If you prefer native JUCE components, set `useWebView = false` in `MainComponent.cpp`

## License

Add your license information here.
