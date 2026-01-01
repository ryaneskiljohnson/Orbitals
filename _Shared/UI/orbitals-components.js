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
    
    // Rotate the indicator line
    if (this.element.querySelector('.knob-indicator')) {
      this.element.querySelector('.knob-indicator').style.transform = `rotate(${angle}deg)`;
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
      percent = 1 - ((e.clientY - rect.top) / rect.height);
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
    
    if (this.options.orientation === 'vertical') {
      this.handle.style.bottom = `${percent * 100}%`;
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
    
    this.handleMin.addEventListener('mousedown', (e) => this.onMouseDown(e, 'min'));
    this.handleMax.addEventListener('mousedown', (e) => this.onMouseDown(e, 'max'));
    
    this.updatePosition();
  }
  
  onMouseDown(e, handle) {
    e.preventDefault();
    this.activeHandle = handle;
    
    document.addEventListener('mousemove', this.onMouseMove.bind(this));
    document.addEventListener('mouseup', this.onMouseUp.bind(this));
  }
  
  onMouseMove(e) {
    if (!this.activeHandle) return;
    
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
    
    this.handleMin.style.left = `${percentMin * 100}%`;
    this.handleMax.style.left = `${percentMax * 100}%`;
    this.range.style.left = `${percentMin * 100}%`;
    this.range.style.width = `${(percentMax - percentMin) * 100}%`;
  }
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
}
