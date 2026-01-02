# ORBITALS DESIGN SYSTEM GUIDE

**NNAudio - Orbitals MIDI FX Plugin Line**

---

## Overview

This design guide documents the visual design system, component library, and implementation patterns for the Orbitals MIDI FX plugin line. Use this guide to maintain consistency across all 8 plugins and any future additions to the line.

---

## Design Philosophy

### Core Principles

1. **Unified Yet Unique**: Each plugin has a distinct visual identity while maintaining clear family resemblance
2. **Space Theme**: All plugins use celestial/astronomical concepts and aesthetics
3. **Functional Beauty**: Visual elements should reflect the plugin's function
4. **Performance-First**: 60fps animations on all target platforms
5. **Accessibility**: Clear labels, readable fonts, intuitive interactions

### Brand Identity

- **Line Name**: ORBITALS
- **Tagline**: Space-themed MIDI transformation
- **Company**: NNAudio
- **Target Audience**: Professional producers, composers, sound designers
- **Price Point**: Premium ($29-39 individual, $149-199 bundle)

---

## Color System

### Base Palette (Shared across all plugins)

```css
--space-black: #0a0a0f;       /* Deep space background */
--nebula-purple: #6b2fb5;      /* Accent color */
--stellar-blue: #0094ff;       /* Primary blue */
--orbit-cyan: #00d4ff;         /* Bright cyan */
--solar-gold: #ffa500;         /* Warm gold */
--comet-white: #e8e8f0;        /* Text white */
--deep-space: #12121a;         /* UI background */
```

### Plugin-Specific Primary Colors

Each plugin has its own primary color scheme that overrides the base:

1. **Perihelion**: Solar gold (#FFA500), Deep orange (#FF6B00), Bright yellow (#FFCC00)
2. **Lagrange**: Stellar blue (#0094FF), Orbit cyan (#00D4FF), Nebula purple (#6B2FB5)
3. **Apogee**: Orbit cyan (#00D4FF), Stellar blue (#0094FF), Peak white (#FFFFFF)
4. **Retrograde**: Nebula purple (#6B2FB5), Bright magenta (#A855F7), Pink accent (#EC4899)
5. **Tidal**: Deep ocean blue (#1E40AF), Wave blue (#3B82F6), Lunar silver (#CBD5E1)
6. **Ion**: Electric purple (#8B5CF6), Plasma cyan (#06B6D4), Ion pink (#F0ABFC)
7. **Eclipse**: Eclipse black (#000000), Corona gold (#FBBF24), Shadow gray (#78716C)
8. **Kepler**: Celestial blue (#3B82F6), Orbit light blue (#60A5FA), Planet gold (#FBBF24)

### Usage Guidelines

- **Primary**: Main UI elements, active states, glows
- **Secondary**: Supporting elements, gradients
- **Accent**: Highlights, special indicators
- **Glow**: Always use as rgba with 0.5-0.7 alpha for shadow effects

---

## Typography

### Font Stack

```css
--font-header: 'Orbitron', 'Space Mono', monospace;
--font-body: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
--font-mono: 'JetBrains Mono', 'Courier New', monospace;
```

### Font Sizes

```css
--font-size-xs: 10px;    /* Labels, small text */
--font-size-sm: 12px;    /* Value displays */
--font-size-base: 14px;  /* Body text */
--font-size-md: 16px;    /* Subheadings */
--font-size-lg: 20px;    /* Large values */
--font-size-xl: 24px;    /* Plugin names */
--font-size-2xl: 32px;   /* Titles */
--font-size-3xl: 48px;   /* Hero text */
```

### Typography Rules

1. Plugin names: `Orbitron`, 24px, 700 weight, uppercase, 2px letter-spacing
2. Control labels: `Inter`, 10px, 500 weight, uppercase, 1.5px letter-spacing, 60% opacity
3. Value displays: `JetBrains Mono`, 12-14px, regular weight
4. Body text: `Inter`, 14px, 400-500 weight

---

## Spacing System

```css
--spacing-xs: 4px;
--spacing-sm: 8px;
--spacing-md: 16px;
--spacing-lg: 24px;
--spacing-xl: 32px;
--spacing-2xl: 48px;
--spacing-3xl: 64px;
```

### Usage

- **xs**: Internal padding for small elements
- **sm**: Gap between related elements
- **md**: Standard component padding
- **lg**: Gap between component groups
- **xl**: Section padding
- **2xl**: Large section spacing
- **3xl**: Page-level spacing

---

## Component Library

### 1. Knobs (Rotary Controls)

**Variants**:
- `knob-medium`: 80×80px
- `knob-large`: 120×120px
- `large-knob`: Same as knob-large (alias)

**Structure**:
```html
<div class="knob-large" id="myKnob" data-param="paramName">
  <div class="knob-inner">
    <div class="knob-indicator"></div>
    <div class="knob-rings"></div> <!-- Optional: animated rings -->
  </div>
</div>
```

**Styling**:
- Background: Radial gradient or solid with plugin-primary color
- Border: 2-4px solid, plugin-primary or glass-border
- Indicator: White line, 3-4px wide, rotates -135° to +135°
- Hover: Scale 1.05, increased glow
- Shadow: Always has glow effect matching plugin-primary

**Interaction**:
- Mouse down + drag vertically to adjust
- Mouse wheel for fine control
- Shift + drag for fine adjustment (optional)

---

### 2. Sliders

**Variants**:
- `slider-vertical`: 40×200px
- `slider-vertical-tall`: 40×300px
- `slider-horizontal`: 200×40px

**Structure**:
```html
<div class="slider-vertical" id="mySlider" data-param="paramName">
  <div class="slider-track"></div>
  <div class="slider-handle"></div>
</div>
```

**Styling**:
- Track: 4px width (vertical) or height (horizontal), glass background
- Handle: 20-24px circle or rectangle, plugin-primary color
- Fill: Optional gradient showing filled portion
- Cursor: `ns-resize` (vertical) or `ew-resize` (horizontal)

---

### 3. XY Pads

**Variants**:
- `xy-pad`: 150×150px standard
- `xy-pad-orbit`: 180×180px larger version

**Structure**:
```html
<div class="xy-pad" id="myPad" data-param="paramName">
  <!-- Crosshairs generated with ::before and ::after -->
  <div class="xy-handle"></div> <!-- Created by JavaScript -->
</div>
```

**Styling**:
- Background: Glass with subtle grid
- Crosshairs: Semi-transparent lines (::before and ::after)
- Handle: 16px circle, plugin-primary, white border, glowing shadow
- Border: 2px, transitions to plugin-primary on hover

---

### 4. Range Sliders (Dual Handle)

**Variants**:
- `range-slider`: Standard dual-handle slider

**Structure**:
```html
<div class="range-slider" id="myRangeSlider" data-param="paramName">
  <div class="range-slider-track"></div>
  <div class="range-slider-range"></div> <!-- Highlighted range -->
  <div class="range-slider-handle range-handle-min"></div>
  <div class="range-slider-handle range-handle-max"></div>
</div>
```

**Styling**:
- Track: Full-width background
- Range: Highlighted section between handles
- Handles: Draggable, plugin-primary color

---

### 5. Toggle Switches / Mode Selectors

**Variants**:
- `mode-selector`: 3-way segmented control
- `tempo-selector`: Tempo-synced value buttons
- `curve-selector`: Waveform/curve type selector

**Structure**:
```html
<div class="mode-selector">
  <button data-mode="velocity" class="active">VELOCITY</button>
  <button data-mode="timing">TIMING</button>
  <button data-mode="phrase">PHRASE</button>
</div>
```

**Styling**:
- Container: Glass background, grouped buttons
- Buttons: Transparent by default, plugin-primary when active
- Active state: Background = plugin-primary, text = space-black, glow effect
- Transition: 150ms for smooth state changes

---

### 6. Value Displays

**Structure**:
```html
<div class="value-display" id="myValue">50%</div>
<div class="value-display large" id="myLargeValue">64</div>
```

**Styling**:
- Font: JetBrains Mono (monospace)
- Background: Dark glass (--glass-bg-dark)
- Border: 1px plugin-primary
- Text: plugin-primary color with subtle glow
- Padding: xs-sm for standard, sm-lg for large
- Min-width: 60px standard, 80px large

---

## Layout Patterns

### Header (Consistent across all plugins)

```html
<header class="orbitals-header">
  <div class="orbitals-logo"></div>
  <h1>PLUGIN NAME</h1>
  <div class="preset-browser"><span>Preset Name</span></div>
</header>
```

**Styling**:
- Height: Auto, padding: md lg
- Background: Glass with blur
- Border-bottom: 1px glass-border
- Logo: 32px circle, orbit-cyan background
- Plugin name: Plugin-primary color, glowing text-shadow

---

### Main Content Area

```html
<main class="plugin-main">
  <canvas id="pluginCanvas"></canvas>
  <div class="controls-[layout]">
    <!-- Controls here -->
  </div>
</main>
```

**Layout Types**:
- `controls-area`: Flexible positioning (Perihelion)
- `controls-grid`: Grid layout (Lagrange)
- `controls-layout`: Custom flex layout (Apogee)

**Canvas**:
- Always positioned absolute, full width/height
- Z-index: 0 (background)
- Cleared each frame with semi-transparent black for trail effect

---

### Footer (Consistent across all plugins)

```html
<footer class="orbitals-footer">
  <button class="bypass-toggle">BYPASS</button>
  <div class="[plugin-specific-meter]"></div>
</footer>
```

**Styling**:
- Height: Auto, padding: md lg
- Background: Glass with blur
- Border-top: 1px glass-border
- Bypass button: Toggleable, glows when active

---

## Animation Guidelines

### Performance Targets

- **60fps**: All animations must run at 60fps
- **Hardware acceleration**: Use CSS transforms and opacity when possible
- **Canvas optimization**: Clear strategy for particle systems (semi-transparent fill vs full clear)

### Animation Types

1. **Particle Systems**
   - Use `GravitationalParticleSystem` for attraction effects (Perihelion)
   - Use `ParticleSystem` for standard emission
   - Limit particles: 300-500 max per plugin
   - Decay particles off-screen immediately

2. **Orbital Motion**
   - Use `OrbitalMotion.circular()` for perfect circles
   - Use `OrbitalMotion.elliptical()` for ellipses
   - Apply Kepler's laws for realistic speed variation

3. **UI Transitions**
   - Use CSS transitions for hover states (150-300ms)
   - Use `tween()` function for animated value changes
   - Apply easing functions for natural motion

### Easing Functions

```javascript
// Available in Easing object
Easing.easeOutCubic    // Most common for UI
Easing.easeInOutSine   // Smooth, wave-like
Easing.easeOutBack     // Elastic feel
Easing.easeOutElastic  // Bounce effect
```

---

## Visual Effects

### Glassmorphism

```css
background: var(--glass-bg);
backdrop-filter: var(--glass-blur);
border: 1px solid var(--glass-border);
```

**Usage**: All panels, control backgrounds, overlays

---

### Glow Effects

```css
box-shadow: 0 0 20px var(--plugin-glow);
text-shadow: 0 0 10px var(--plugin-primary);
```

**Usage**: Active controls, plugin names, important indicators

---

### Particle Trails

- Length: 5-10 points
- Opacity: Fade from 100% to 0% along trail
- Width: Taper from particle size to 0
- Use for velocity visualization, motion indication

---

## Canvas Visualization Patterns

### 1. Background Stars (All Plugins)

```javascript
// In plugin-container::before pseudo-element
background-image: radial-gradient(1px 1px at 20% 30%, white, transparent), ...
opacity: 0.3;
```

---

### 2. Central Attraction Point

**Used in**: Perihelion (sun), Lagrange (center), Ion (key center)

```javascript
const gradient = ctx.createRadialGradient(cx, cy, 0, cx, cy, radius);
gradient.addColorStop(0, '#ffffff');
gradient.addColorStop(0.5, pluginPrimary);
gradient.addColorStop(1, 'transparent');
ctx.fillStyle = gradient;
ctx.arc(cx, cy, radius, 0, Math.PI * 2);
ctx.fill();
```

---

### 3. Orbital Paths

**Used in**: All plugins with orbital themes

```javascript
ctx.strokeStyle = 'rgba(plugin-primary-rgb, 0.3)';
ctx.lineWidth = 2;
ctx.arc(cx, cy, orbitRadius, 0, Math.PI * 2);
ctx.stroke();
```

---

### 4. Trajectory Arcs

**Used in**: Apogee

```javascript
for (let t = 0; t <= 1; t += 0.01) {
  const x = startX + (endX - startX) * t;
  const y = startY + trajectory(t) * height;
  // Draw curve
}
```

---

## JavaScript Integration

### Component Initialization

```javascript
document.addEventListener('DOMContentLoaded', () => {
  // Setup canvas visualization
  const canvas = document.getElementById('pluginCanvas');
  if (canvas) setupVisualization(canvas);
  
  // Initialize controls
  setupControls();
});
```

### JUCE Communication

```javascript
// Send to JUCE
function sendToJUCE(param, value) {
  if (window.chrome?.webview) {
    window.chrome.webview.postMessage({
      type: 'parameterChange',
      parameter: param,
      value: value
    });
  }
}

// Receive from JUCE
window.receiveMessageFromJUCE = function(message) {
  // Handle MIDI notes, parameter updates, etc.
};
```

---

## File Structure

Each plugin follows this structure:

```
PluginName/
├── Source/
│   ├── PluginProcessor.h
│   ├── PluginProcessor.cpp
│   ├── PluginEditor.h
│   └── PluginEditor.cpp
├── UI/
│   ├── index.html      # Main interface
│   ├── styles.css      # Plugin-specific styles
│   └── app.js          # Control logic + visualization
├── CMakeLists.txt
├── PluginName.jucer
└── README.md
```

---

## Responsive Considerations

### Window Sizes

- **Minimum**: 800×600px
- **Default**: 1000×800px
- **Maximum**: Scalable to 4K

### Scaling Strategy

```css
@media (max-width: 768px) {
  :root {
    --font-size-base: 12px;
    --spacing-lg: 16px;
  }
}
```

---

## Accessibility

### Keyboard Navigation

- Tab through controls in logical order
- Enter/Space to activate buttons
- Arrow keys for sliders/knobs (when focused)
- Esc to close modals/menus

### Screen Readers

- Use semantic HTML
- Include aria-labels on custom controls
- Provide text alternatives for visual feedback

### Contrast

- Minimum 4.5:1 for normal text
- Minimum 3:1 for large text and UI components
- All plugins pass WCAG AA standards

---

## Future Expansion

### Adding New Plugins to the Line

1. Choose celestial theme and name
2. Select primary color (unique, fits space palette)
3. Design control layout matching function
4. Create canvas visualization reflecting concept
5. Use shared components from `_Shared/UI/`
6. Follow this design guide for consistency

### Potential Future Plugins

- **ESCAPE VELOCITY**: Note spreading/dispersion
- **SINGULARITY**: Gravity well compression
- **RADIANT**: Energy/brightness modulation
- **AXIS**: Rotational transformations

---

## Credits

**Design System**: NNAudio Design Team
**JUCE Framework**: JUCE 8+
**WebView UI**: HTML5, CSS3, Canvas API
**Libraries**: Orbitals shared component library

---

**© 2026 NNAudio - Premium Music Production Tools**
