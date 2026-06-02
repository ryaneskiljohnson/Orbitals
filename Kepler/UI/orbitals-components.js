/**
 * @fileoverview Orbitals Components Library - Reusable UI Components
 * @module Orbitals Components
 * 
 * Provides interactive control components for the Orbitals plugin line:
 * - Knobs (circular rotary controls)
 * - Sliders (vertical, horizontal, range)
 * - XY Pads (2D touch controls)
 * - Toggles and buttons
 */

/* ===================================================================
   KNOB COMPONENT
   =================================================================== */

/**
 * @brief Interactive circular knob control
 * @param {HTMLElement} element - DOM element for the knob
 * @param {Object} options - Configuration options
 * @returns {Object} Knob instance with setValue and getValue methods
 */
class OrbitalsKnob {
  constructor(element, options = {}) {
    this.element = element;
    this.options = {
      min: options.min || 0,
      max: options.max || 100,
      value: options.value || 50,
      step: options.step || 1,
      angleStart: options.angleStart || -135,
      angleEnd: options.angleEnd || 135,
      onChange: options.onChange || null,
      parameter: element.dataset.param || ''
    };
    
    this.value = this.options.value;
    this.isDragging = false;
    this.startY = 0;
    this.startValue = 0;
    
    this.init();
  }
  
  init() {
    this.element.classList.add('no-select');
    this.element.addEventListener('mousedown', this.onMouseDown.bind(this));
    this.element.addEventListener('wheel', this.onWheel.bind(this));
    this.updateRotation();
  }
  
  onMouseDown(e) {
    e.preventDefault();
    this.isDragging = true;
    this.startY = e.clientY;
    this.startValue = this.value;
    
    // Disable transitions during drag for smooth updates
    const indicator = this.element.querySelector('.knob-indicator');
    if (indicator) {
      indicator.style.transition = 'none';
      indicator.style.willChange = 'transform';
    }
    
    document.addEventListener('mousemove', this.onMouseMove.bind(this));
    document.addEventListener('mouseup', this.onMouseUp.bind(this));
  }
  
  onMouseMove(e) {
    if (!this.isDragging) return;
    
    const deltaY = this.startY - e.clientY;
    const sensitivity = 0.5;
    const range = this.options.max - this.options.min;
    const change = (deltaY * sensitivity * range) / 100;
    
    this.setValue(this.startValue + change);
  }
  
  onMouseUp() {
    this.isDragging = false;
    
    // Re-enable transitions after drag
    const indicator = this.element.querySelector('.knob-indicator');
    if (indicator) {
      indicator.style.transition = '';
      indicator.style.willChange = '';
    }
    
    document.removeEventListener('mousemove', this.onMouseMove);
    document.removeEventListener('mouseup', this.onMouseUp);
  }
  
  onWheel(e) {
    e.preventDefault();
    const delta = -e.deltaY * 0.1;
    this.setValue(this.value + delta);
  }
  
  setValue(newValue) {
    this.value = Math.max(this.options.min, Math.min(this.options.max, newValue));
    
    // Snap to step
    this.value = Math.round(this.value / this.options.step) * this.options.step;
    
    this.updateRotation();
    
    if (this.options.onChange) {
      this.options.onChange(this.value, this.options.parameter);
    }
  }
  
  getValue() {
    return this.value;
  }
  
  updateRotation() {
    const percent = (this.value - this.options.min) / (this.options.max - this.options.min);
    const angleRange = this.options.angleEnd - this.options.angleStart;
    const angle = this.options.angleStart + (percent * angleRange);
    
    // Update rotation immediately (transitions are disabled during drag)
    const indicator = this.element.querySelector('.knob-indicator');
    if (indicator) {
      // Preserve translateX(-50%) for centering, add rotation
      indicator.style.transform = `translateX(-50%) rotate(${angle}deg)`;
    } else {
      this.element.style.setProperty('--knob-rotation', `${angle}deg`);
    }
  }
}

/* ===================================================================
   SLIDER COMPONENT
   =================================================================== */

/**
 * @brief Interactive slider control (vertical or horizontal)
 * @param {HTMLElement} element - DOM element for the slider
 * @param {Object} options - Configuration options
 * @returns {Object} Slider instance with setValue and getValue methods
 */
class OrbitalsSlider {
  constructor(element, options = {}) {
    this.element = element;
    this.options = {
      min: options.min || 0,
      max: options.max || 100,
      value: options.value || 50,
      step: options.step || 1,
      orientation: options.orientation || 'vertical', // 'vertical' or 'horizontal'
      onChange: options.onChange || null,
      parameter: element.dataset.param || ''
    };
    
    this.value = this.options.value;
    this.isDragging = false;
    
    this.init();
  }
  
  init() {
    this.element.classList.add('no-select');
    this.element.addEventListener('mousedown', this.onMouseDown.bind(this));
    this.element.addEventListener('wheel', this.onWheel.bind(this));
    
    // Create handle if it doesn't exist
    if (!this.element.querySelector('.slider-handle')) {
      const handle = document.createElement('div');
      handle.className = 'slider-handle';
      this.element.appendChild(handle);
    }
    
    this.handle = this.element.querySelector('.slider-handle');
    this.updatePosition();
  }
  
  onMouseDown(e) {
    e.preventDefault();
    this.isDragging = true;
    this.updateFromMouse(e);
    
    document.addEventListener('mousemove', this.onMouseMove.bind(this));
    document.addEventListener('mouseup', this.onMouseUp.bind(this));
  }
  
  onMouseMove(e) {
    if (!this.isDragging) return;
    this.updateFromMouse(e);
  }
  
  onMouseUp() {
    this.isDragging = false;
    document.removeEventListener('mousemove', this.onMouseMove);
    document.removeEventListener('mouseup', this.onMouseUp);
  }
  
  onWheel(e) {
    e.preventDefault();
    const delta = -e.deltaY * 0.1;
    this.setValue(this.value + delta);
  }
  
  updateFromMouse(e) {
    const rect = this.element.getBoundingClientRect();
    let percent;
    
    if (this.options.orientation === 'vertical') {
      // Account for 12px padding (24px total, which is 10.9% of 220px height)
      const padding = 12;
      const paddingPercent = padding / rect.height;
      const rawPercent = 1 - ((e.clientY - rect.top) / rect.height);
      // Map from full range to padded range
      percent = (rawPercent - paddingPercent) / (1 - 2 * paddingPercent);
    } else {
      percent = (e.clientX - rect.left) / rect.width;
    }
    
    percent = Math.max(0, Math.min(1, percent));
    const newValue = this.options.min + (percent * (this.options.max - this.options.min));
    this.setValue(newValue);
  }
  
  setValue(newValue) {
    this.value = Math.max(this.options.min, Math.min(this.options.max, newValue));
    
    // Snap to step
    this.value = Math.round(this.value / this.options.step) * this.options.step;
    
    this.updatePosition();
    
    if (this.options.onChange) {
      this.options.onChange(this.value, this.options.parameter);
    }
  }
  
  getValue() {
    return this.value;
  }
  
  updatePosition() {
    const percent = (this.value - this.options.min) / (this.options.max - this.options.min);
    
    // Set CSS variable for fill visualization - must be set first
    this.element.style.setProperty('--slider-value', percent);
    
    // Disable transitions for instant updates
    this.handle.style.transition = 'none';
    this.handle.style.position = 'absolute';
    
    // Update handle position synchronously
    if (this.options.orientation === 'vertical') {
      // Account for 12px padding at top and bottom
      // Map percent (0-1) to the padded range
      // Total height: 220px, padding: 12px top/bottom, usable: 196px
      // bottom = 12px + (percent * 196px) = 12px + (percent * (220px - 24px))
      // As percentage: (12 + percent * 196) / 220 * 100 = 5.45 + percent * 89.09
      const paddingPercent = 5.45; // 12/220 * 100
      const usablePercent = 89.09; // 196/220 * 100
      const bottomPercent = paddingPercent + (percent * usablePercent);
      this.handle.style.bottom = `${bottomPercent}%`;
      this.handle.style.left = '50%';
      this.handle.style.transform = 'translate(-50%, 50%)';
    } else {
      this.handle.style.left = `${percent * 100}%`;
      this.handle.style.top = '50%';
      this.handle.style.transform = 'translate(-50%, -50%)';
    }
  }
}

/* ===================================================================
   XY PAD COMPONENT
   =================================================================== */

/**
 * @brief 2D touch pad control for X/Y parameters
 * @param {HTMLElement} element - DOM element for the XY pad
 * @param {Object} options - Configuration options
 * @returns {Object} XYPad instance with setValue and getValue methods
 */
class OrbitalsXYPad {
  constructor(element, options = {}) {
    this.element = element;
    this.options = {
      minX: options.minX || 0,
      maxX: options.maxX || 100,
      minY: options.minY || 0,
      maxY: options.maxY || 100,
      valueX: options.valueX || 50,
      valueY: options.valueY || 50,
      onChange: options.onChange || null,
      parameter: element.dataset.param || ''
    };
    
    this.valueX = this.options.valueX;
    this.valueY = this.options.valueY;
    this.isDragging = false;
    
    this.init();
  }
  
  init() {
    this.element.classList.add('no-select');
    this.element.addEventListener('mousedown', this.onMouseDown.bind(this));
    
    // Create handle if it doesn't exist
    if (!this.element.querySelector('.xy-handle')) {
      const handle = document.createElement('div');
      handle.className = 'xy-handle';
      handle.style.cssText = `
        position: absolute;
        width: 16px;
        height: 16px;
        background: var(--plugin-primary, var(--orbit-cyan));
        border: 2px solid white;
        border-radius: 50%;
        box-shadow: 0 0 12px var(--plugin-primary, var(--orbit-cyan));
        pointer-events: none;
      `;
      this.element.appendChild(handle);
    }
    
    this.handle = this.element.querySelector('.xy-handle');
    this.updatePosition();
  }
  
  onMouseDown(e) {
    e.preventDefault();
    this.isDragging = true;
    this.updateFromMouse(e);
    
    document.addEventListener('mousemove', this.onMouseMove.bind(this));
    document.addEventListener('mouseup', this.onMouseUp.bind(this));
  }
  
  onMouseMove(e) {
    if (!this.isDragging) return;
    this.updateFromMouse(e);
  }
  
  onMouseUp() {
    this.isDragging = false;
    document.removeEventListener('mousemove', this.onMouseMove);
    document.removeEventListener('mouseup', this.onMouseUp);
  }
  
  updateFromMouse(e) {
    const rect = this.element.getBoundingClientRect();
    
    const percentX = Math.max(0, Math.min(1, (e.clientX - rect.left) / rect.width));
    const percentY = Math.max(0, Math.min(1, 1 - ((e.clientY - rect.top) / rect.height)));
    
    const newX = this.options.minX + (percentX * (this.options.maxX - this.options.minX));
    const newY = this.options.minY + (percentY * (this.options.maxY - this.options.minY));
    
    this.setValue(newX, newY);
  }
  
  setValue(x, y) {
    this.valueX = Math.max(this.options.minX, Math.min(this.options.maxX, x));
    this.valueY = Math.max(this.options.minY, Math.min(this.options.maxY, y));
    
    this.updatePosition();
    
    if (this.options.onChange) {
      this.options.onChange(this.valueX, this.valueY, this.options.parameter);
    }
  }
  
  getValue() {
    return { x: this.valueX, y: this.valueY };
  }
  
  updatePosition() {
    const percentX = (this.valueX - this.options.minX) / (this.options.maxX - this.options.minX);
    const percentY = (this.valueY - this.options.minY) / (this.options.maxY - this.options.minY);
    
    this.handle.style.transition = 'none';
    this.handle.style.left = `${percentX * 100}%`;
    this.handle.style.bottom = `${percentY * 100}%`;
    this.handle.style.transform = 'translate(-50%, 50%)';
  }
}

/* ===================================================================
   RANGE SLIDER (DUAL HANDLE)
   =================================================================== */

/**
 * @brief Dual-handle range slider for min/max values
 * @param {HTMLElement} element - DOM element for the range slider
 * @param {Object} options - Configuration options
 */
class OrbitalsRangeSlider {
  constructor(element, options = {}) {
    this.element = element;
    this.options = {
      min: options.min || 0,
      max: options.max || 100,
      valueMin: options.valueMin || 25,
      valueMax: options.valueMax || 75,
      onChange: options.onChange || null,
      parameter: element.dataset.param || ''
    };
    
    this.valueMin = this.options.valueMin;
    this.valueMax = this.options.valueMax;
    this.activeHandle = null;
    this.isDraggingRange = false;
    this.dragStartX = 0;
    this.dragStartMin = 0;
    this.dragStartMax = 0;
    
    this.init();
  }
  
  init() {
    // Create handles and range display
    this.element.innerHTML = `
      <div class="range-slider-track"></div>
      <div class="range-slider-range"></div>
      <div class="range-slider-handle range-handle-min"></div>
      <div class="range-slider-handle range-handle-max"></div>
    `;
    
    this.handleMin = this.element.querySelector('.range-handle-min');
    this.handleMax = this.element.querySelector('.range-handle-max');
    this.range = this.element.querySelector('.range-slider-range');
    
    // Handle proximity threshold (in pixels)
    this.handleProximityThreshold = 20;
    
    this.handleMin.addEventListener('mousedown', (e) => { e.stopPropagation(); this.onMouseDown(e, 'min'); });
    this.handleMax.addEventListener('mousedown', (e) => { e.stopPropagation(); this.onMouseDown(e, 'max'); });
    
    // Allow dragging the entire range, but check proximity to handles first
    this.range.addEventListener('mousedown', (e) => this.onRangeMouseDown(e));
    this.range.addEventListener('mousemove', (e) => this.onRangeHover(e)); // For cursor changes on hover
    this.range.addEventListener('mouseleave', () => {
      if (!this.isDraggingRange) {
        this.range.style.cursor = 'grab';
      }
    });
    this.element.addEventListener('mousedown', (e) => {
      // If clicking on the slider but not on a handle or range, check if it's in the range area
      if (e.target === this.element || e.target === this.element.querySelector('.range-slider-track')) {
        const rect = this.element.getBoundingClientRect();
        const percent = (e.clientX - rect.left) / rect.width;
        const percentMin = (this.valueMin - this.options.min) / (this.options.max - this.options.min);
        const percentMax = (this.valueMax - this.options.min) / (this.options.max - this.options.min);
        
        // If click is within the range, check proximity to handles
        if (percent >= percentMin && percent <= percentMax) {
          const handle = this.getHandleAtPosition(e.clientX);
          if (handle) {
            // Close to a handle - resize that end
            this.onMouseDown(e, handle);
          } else {
            // In the middle - move entire range
            this.onRangeMouseDown(e);
          }
        }
      }
    });
    
    this.updatePosition();
  }
  
  /**
   * @brief Check which handle (if any) is near the mouse position
   * @param {number} clientX - Mouse X position
   * @returns {string|null} 'min', 'max', or null
   */
  getHandleAtPosition(clientX) {
    const rect = this.element.getBoundingClientRect();
    const percentMin = (this.valueMin - this.options.min) / (this.options.max - this.options.min);
    const percentMax = (this.valueMax - this.options.min) / (this.options.max - this.options.min);
    
    const minHandleX = rect.left + (percentMin * rect.width);
    const maxHandleX = rect.left + (percentMax * rect.width);
    
    const distanceToMin = Math.abs(clientX - minHandleX);
    const distanceToMax = Math.abs(clientX - maxHandleX);
    
    // Check if within threshold of either handle
    if (distanceToMin <= this.handleProximityThreshold && distanceToMin < distanceToMax) {
      return 'min';
    } else if (distanceToMax <= this.handleProximityThreshold) {
      return 'max';
    }
    
    return null;
  }
  
  onRangeMouseDown(e) {
    // Check if we're actually close to a handle - if so, resize that handle instead
    const handle = this.getHandleAtPosition(e.clientX);
    if (handle) {
      // Close to a handle - resize that end instead of moving range
      this.onMouseDown(e, handle);
      return;
    }
    
    e.preventDefault();
    e.stopPropagation();
    this.isDraggingRange = true;
    this.dragStartX = e.clientX;
    this.dragStartMin = this.valueMin;
    this.dragStartMax = this.valueMax;
    
    document.addEventListener('mousemove', this.onRangeMouseMove.bind(this));
    document.addEventListener('mouseup', this.onRangeMouseUp.bind(this));
  }
  
  
  /**
   * @brief Update cursor on hover over range (not dragging)
   * @param {MouseEvent} e - Mouse event
   */
  onRangeHover(e) {
    if (this.isDraggingRange) return; // Don't update cursor while dragging
    
    const handle = this.getHandleAtPosition(e.clientX);
    if (handle) {
      // Close to a handle - show resize cursor
      this.range.style.cursor = 'ew-resize';
    } else {
      // In the middle - show grab cursor
      this.range.style.cursor = 'grab';
    }
  }
  
  /**
   * @brief Handle mouse move while dragging the range
   * @param {MouseEvent} e - Mouse event
   */
  onRangeMouseMove(e) {
    if (!this.isDraggingRange) return;
    
    // While dragging, show grabbing cursor
    this.range.style.cursor = 'grabbing';
    
    const rect = this.element.getBoundingClientRect();
    const deltaX = e.clientX - this.dragStartX;
    const deltaPercent = deltaX / rect.width;
    const deltaValue = deltaPercent * (this.options.max - this.options.min);
    
    const newMin = this.dragStartMin + deltaValue;
    const newMax = this.dragStartMax + deltaValue;
    
    // Clamp to bounds
    if (newMin < this.options.min) {
      const offset = this.options.min - newMin;
      this.valueMin = this.options.min;
      this.valueMax = newMax + offset;
    } else if (newMax > this.options.max) {
      const offset = newMax - this.options.max;
      this.valueMin = newMin - offset;
      this.valueMax = this.options.max;
    } else {
      this.valueMin = newMin;
      this.valueMax = newMax;
    }
    
    // Ensure min doesn't exceed max
    if (this.valueMin > this.valueMax) {
      const temp = this.valueMin;
      this.valueMin = this.valueMax;
      this.valueMax = temp;
    }
    
    this.updatePosition();
    
    if (this.options.onChange) {
      this.options.onChange(this.valueMin, this.valueMax, this.options.parameter);
    }
  }
  
  onRangeMouseUp() {
    this.isDraggingRange = false;
    document.removeEventListener('mousemove', this.onRangeMouseMove);
    document.removeEventListener('mouseup', this.onRangeMouseUp);
    // Reset cursor
    this.range.style.cursor = 'grab';
  }
  
  onMouseDown(e, handle) {
    e.preventDefault();
    this.activeHandle = handle;
    
    document.addEventListener('mousemove', this.onMouseMove.bind(this));
    document.addEventListener('mouseup', this.onMouseUp.bind(this));
  }
  
  onMouseMove(e) {
    if (!this.activeHandle || this.isDraggingRange) return;
    
    const rect = this.element.getBoundingClientRect();
    const percent = Math.max(0, Math.min(1, (e.clientX - rect.left) / rect.width));
    const value = this.options.min + (percent * (this.options.max - this.options.min));
    
    if (this.activeHandle === 'min') {
      this.valueMin = Math.min(value, this.valueMax);
    } else {
      this.valueMax = Math.max(value, this.valueMin);
    }
    
    this.updatePosition();
    
    if (this.options.onChange) {
      this.options.onChange(this.valueMin, this.valueMax, this.options.parameter);
    }
  }
  
  onMouseUp() {
    this.activeHandle = null;
    document.removeEventListener('mousemove', this.onMouseMove);
    document.removeEventListener('mouseup', this.onMouseUp);
  }
  
  updatePosition() {
    const percentMin = (this.valueMin - this.options.min) / (this.options.max - this.options.min);
    const percentMax = (this.valueMax - this.options.min) / (this.options.max - this.options.min);
    
    // Set CSS variables for styling
    this.element.style.setProperty('--range-min', `${percentMin * 100}%`);
    this.element.style.setProperty('--range-max', `${percentMax * 100}%`);
    
    // Also set directly for compatibility
    this.handleMin.style.left = `${percentMin * 100}%`;
    this.handleMax.style.left = `${percentMax * 100}%`;
    this.range.style.left = `${percentMin * 100}%`;
    this.range.style.width = `${(percentMax - percentMin) * 100}%`;
  }
}

/* ===================================================================
   CONTEXTUAL HELP TOOLTIP SYSTEM
   =================================================================== */

/**
 * @brief Tooltip manager for contextual help
 */
class OrbitalsTooltip {
  constructor() {
    this.tooltip = null;
    this.currentElement = null;
    this.hideTimeout = null;
    this.enabled = false; // Help is disabled by default
    this.init();
  }
  
  /**
   * @brief Initialize tooltip element
   */
  init() {
    // Create tooltip element
    this.tooltip = document.createElement('div');
    this.tooltip.className = 'orbitals-tooltip';
    this.tooltip.innerHTML = `
      <div class="orbitals-tooltip-title"></div>
      <div class="orbitals-tooltip-description"></div>
    `;
    document.body.appendChild(this.tooltip);
  }
  
  /**
   * @brief Enable or disable help tooltips
   * @param {boolean} enabled - Whether to enable tooltips
   */
  setEnabled(enabled) {
    this.enabled = enabled;
    if (!enabled) {
      this.hide();
    }
  }
  
  /**
   * @brief Toggle help on/off
   * @returns {boolean} New enabled state
   */
  toggle() {
    this.enabled = !this.enabled;
    if (!this.enabled) {
      this.hide();
    }
    return this.enabled;
  }
  
  /**
   * @brief Show tooltip for an element
   * @param {HTMLElement} element - Element to show tooltip for
   * @param {string} title - Tooltip title
   * @param {string} description - Tooltip description
   */
  show(element, title, description) {
    if (!this.tooltip || !this.enabled) return; // Only show if enabled
    
    clearTimeout(this.hideTimeout);
    this.currentElement = element;
    
    // Update content
    this.tooltip.querySelector('.orbitals-tooltip-title').textContent = title;
    this.tooltip.querySelector('.orbitals-tooltip-description').textContent = description;
    
    // Position tooltip
    const rect = element.getBoundingClientRect();
    const tooltipRect = this.tooltip.getBoundingClientRect();
    
    // Position below element by default
    let left = rect.left + rect.width / 2 - tooltipRect.width / 2;
    let top = rect.bottom + 12;
    
    // Keep tooltip within viewport
    const padding = 16;
    if (left < padding) left = padding;
    if (left + tooltipRect.width > window.innerWidth - padding) {
      left = window.innerWidth - tooltipRect.width - padding;
    }
    
    // If tooltip would go off bottom, position above
    if (top + tooltipRect.height > window.innerHeight - padding) {
      top = rect.top - tooltipRect.height - 12;
      // Flip arrow
      this.tooltip.classList.add('tooltip-above');
    } else {
      this.tooltip.classList.remove('tooltip-above');
    }
    
    this.tooltip.style.left = left + 'px';
    this.tooltip.style.top = top + 'px';
    
    // Show with animation
    requestAnimationFrame(() => {
      this.tooltip.classList.add('visible');
    });
  }
  
  /**
   * @brief Hide tooltip
   */
  hide() {
    if (!this.tooltip) return;
    
    this.hideTimeout = setTimeout(() => {
      this.tooltip.classList.remove('visible');
      this.currentElement = null;
    }, 100);
  }
  
  /**
   * @brief Register help text for an element
   * @param {HTMLElement|string} element - Element or selector
   * @param {string} title - Help title
   * @param {string} description - Help description
   */
  register(element, title, description) {
    const el = typeof element === 'string' ? document.querySelector(element) : element;
    if (!el) return;
    
    // Store help data
    el.dataset.helpTitle = title;
    el.dataset.helpDescription = description;
    
    // Add event listeners
    el.addEventListener('mouseenter', () => {
      this.show(el, title, description);
    });
    
    el.addEventListener('mouseleave', () => {
      this.hide();
    });
  }
  
  /**
   * @brief Auto-register all elements with data-help attributes
   */
  autoRegister() {
    document.querySelectorAll('[data-help-title]').forEach(element => {
      const title = element.dataset.helpTitle;
      const description = element.dataset.helpDescription || '';
      this.register(element, title, description);
    });
  }
}

// Create global tooltip instance
const orbitalsTooltip = new OrbitalsTooltip();

// Auto-register on DOM ready
if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', () => {
    setTimeout(() => {
      orbitalsTooltip.autoRegister();
      setupHelpToggle();
    }, 100);
  });
} else {
  setTimeout(() => {
    orbitalsTooltip.autoRegister();
    setupHelpToggle();
  }, 100);
}

/**
 * @brief Setup help toggle button
 */
function setupHelpToggle() {
  const helpButton = document.querySelector('.help-toggle');
  if (helpButton) {
    helpButton.addEventListener('click', () => {
      const isEnabled = orbitalsTooltip.toggle();
      helpButton.classList.toggle('active', isEnabled);
      
      // Visual feedback
      if (isEnabled) {
        console.log('Help enabled: Hover over controls for descriptions');
      } else {
        console.log('Help disabled');
      }
    });
  }
}

/**
 * @brief Setup bypass toggle button with ON/OFF state
 * @param {Function} sendToJUCE - Function to send parameter to JUCE
 */
function setupBypassToggle(sendToJUCE) {
  const toggle = document.getElementById('bypassToggle');
  if (!toggle) return;
  
  const textSpan = toggle.querySelector('.bypass-text');
  if (!textSpan) return;
  
  // Initialize state (not bypassed = ON by default)
  let bypassed = false;
  updateBypassState(bypassed);
  
  toggle.addEventListener('click', (e) => {
    e.preventDefault();
    e.stopPropagation();
    bypassed = !bypassed;
    updateBypassState(bypassed);
    
    // Send to JUCE
    if (sendToJUCE) {
      sendToJUCE('bypass', bypassed ? 1 : 0);
    }
  });
  
  function updateBypassState(isBypassed) {
    if (isBypassed) {
      toggle.classList.add('active');
      textSpan.textContent = 'OFF';
    } else {
      toggle.classList.remove('active');
      textSpan.textContent = 'ON';
    }
  }
  
  // Expose method to update from JUCE
  toggle.updateFromJUCE = function(value) {
    bypassed = value > 0.5;
    updateBypassState(bypassed);
  };
}

/* ===================================================================
   EXPORT FOR USE
   =================================================================== */

// Make available globally
if (typeof window !== 'undefined') {
  window.OrbitalsKnob = OrbitalsKnob;
  window.OrbitalsSlider = OrbitalsSlider;
  window.OrbitalsXYPad = OrbitalsXYPad;
  window.OrbitalsRangeSlider = OrbitalsRangeSlider;
  window.OrbitalsTooltip = OrbitalsTooltip;
  window.orbitalsTooltip = orbitalsTooltip;
  window.setupBypassToggle = setupBypassToggle;
}
