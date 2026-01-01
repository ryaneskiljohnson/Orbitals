/**
 * @fileoverview Perihelion - Velocity Sculptor Application Logic
 * @module Perihelion App
 * 
 * Handles UI interactions and solar particle visualization
 */

/* ===================================================================
   INITIALIZATION
   =================================================================== */

document.addEventListener('DOMContentLoaded', () => {
  initializePerihelion();
});

function initializePerihelion() {
  console.log('Perihelion initializing...');
  
  // Setup canvas
  const canvas = document.getElementById('solarCanvas');
  if (canvas) {
    setupSolarVisualization(canvas);
  }
  
  // Setup controls
  setupGravityKnob();
  setupSolarPointSlider();
  setupOrbitArc();
  setupBiasSlider();
  setupBypassToggle();
  
  console.log('Perihelion initialized!');
}

/* ===================================================================
   SOLAR VISUALIZATION
   =================================================================== */

function setupSolarVisualization(canvas) {
  const ctx = CanvasUtils.setupHiDPI(canvas);
  const rect = canvas.getBoundingClientRect();
  
  // Create gravitational particle system
  const particleSystem = new GravitationalParticleSystem(canvas, {
    maxParticles: 400,
    emissionRate: 0.4
  });
  
  // Add central sun attractor
  const centerX = rect.width / 2;
  const centerY = rect.height / 2;
  particleSystem.addAttractor(centerX, centerY, 0.06);
  
  // Particle emission settings
  let emissionIntensity = 0.6;
  let time = 0;
  
  // Animation loop
  animationManager.add('perihelion-solar', (deltaTime) => {
    time += deltaTime * 0.001;
    
    // Clear canvas with trail effect
    ctx.fillStyle = 'rgba(10, 10, 15, 0.08)';
    ctx.fillRect(0, 0, rect.width, rect.height);
    
    // Draw orbital rings (multiple layers)
    drawOrbitalRings(ctx, centerX, centerY, time);
    
    // Draw sun corona (outer glow)
    const coronaSize = 80 + Math.sin(time * 2) * 10;
    const coronaGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, coronaSize);
    coronaGradient.addColorStop(0, 'rgba(255, 255, 200, 0.15)');
    coronaGradient.addColorStop(0.4, 'rgba(255, 165, 0, 0.1)');
    coronaGradient.addColorStop(1, 'rgba(255, 106, 0, 0)');
    ctx.fillStyle = coronaGradient;
    ctx.beginPath();
    ctx.arc(centerX, centerY, coronaSize, 0, Math.PI * 2);
    ctx.fill();
    
    // Draw central sun with pulsing effect
    const sunRadius = 45 + Math.sin(time * 3) * 3;
    const sunGradient = ctx.createRadialGradient(
      centerX - 10, centerY - 10, 0,
      centerX, centerY, sunRadius
    );
    sunGradient.addColorStop(0, '#ffffff');
    sunGradient.addColorStop(0.2, '#ffff88');
    sunGradient.addColorStop(0.5, '#ffcc00');
    sunGradient.addColorStop(0.8, '#ffaa00');
    sunGradient.addColorStop(1, 'rgba(255, 106, 0, 0)');
    
    ctx.save();
    ctx.shadowColor = '#ffa500';
    ctx.shadowBlur = 50;
    ctx.fillStyle = sunGradient;
    ctx.beginPath();
    ctx.arc(centerX, centerY, sunRadius, 0, Math.PI * 2);
    ctx.fill();
    ctx.restore();
    
    // Draw solar flares (random bursts)
    if (Math.random() < 0.05) {
      drawSolarFlare(ctx, centerX, centerY, sunRadius);
    }
    
    // Emit new particles from edges with variety
    if (Math.random() < emissionIntensity) {
      const angle = Math.random() * Math.PI * 2;
      const distance = Math.max(rect.width, rect.height) * 0.55;
      const px = centerX + Math.cos(angle) * distance;
      const py = centerY + Math.sin(angle) * distance;
      
      // Vary particle properties for more interest
      const hue = 20 + Math.random() * 50; // Orange to yellow range
      const size = 1.5 + Math.random() * 3;
      
      particleSystem.emit(1, {
        size: size,
        color: ColorUtils.hslToRgb(hue, 100, 55 + Math.random() * 25),
        life: 3 + Math.random() * 3,
        decay: 0.006 + Math.random() * 0.004,
        glow: true,
        trailLength: Math.floor(6 + Math.random() * 6),
        speed: 0
      });
      particleSystem.options.emitterX = px;
      particleSystem.options.emitterY = py;
    }
    
    // Update and draw particles
    particleSystem.update(deltaTime);
    particleSystem.draw();
  });
}

/**
 * @brief Draw solar flare effect
 */
function drawSolarFlare(ctx, cx, cy, radius) {
  const angle = Math.random() * Math.PI * 2;
  const length = radius + Math.random() * 40;
  const x = cx + Math.cos(angle) * radius;
  const y = cy + Math.sin(angle) * radius;
  const endX = cx + Math.cos(angle) * length;
  const endY = cy + Math.sin(angle) * length;
  
  const gradient = ctx.createLinearGradient(x, y, endX, endY);
  gradient.addColorStop(0, 'rgba(255, 255, 100, 0.8)');
  gradient.addColorStop(0.5, 'rgba(255, 165, 0, 0.4)');
  gradient.addColorStop(1, 'transparent');
  
  ctx.save();
  ctx.strokeStyle = gradient;
  ctx.lineWidth = 2 + Math.random() * 3;
  ctx.lineCap = 'round';
  ctx.shadowColor = '#ffa500';
  ctx.shadowBlur = 20;
  ctx.beginPath();
  ctx.moveTo(x, y);
  ctx.lineTo(endX, endY);
  ctx.stroke();
  ctx.restore();
}

/**
 * @brief Draw decorative orbital rings around sun
 */
function drawOrbitalRings(ctx, centerX, centerY, time) {
  const ringCount = 5;
  const baseRadius = 65;
  
  for (let i = 0; i < ringCount; i++) {
    const radius = baseRadius + i * 35;
    const pulsePhase = time + i * 0.5;
    const alpha = 0.08 + Math.sin(pulsePhase) * 0.04;
    const lineWidth = 1 + Math.sin(pulsePhase * 2) * 0.5;
    
    // Create dashed ring effect
    ctx.save();
    ctx.strokeStyle = `rgba(255, 165, 0, ${alpha})`;
    ctx.lineWidth = lineWidth;
    ctx.setLineDash([5, 10]);
    ctx.lineDashOffset = -time * 20 * (i % 2 === 0 ? 1 : -1);
    ctx.shadowColor = 'rgba(255, 165, 0, 0.5)';
    ctx.shadowBlur = 10;
    ctx.beginPath();
    ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
    ctx.stroke();
    ctx.restore();
  }
}

/* ===================================================================
   CONTROL SETUP
   =================================================================== */

/**
 * @brief Setup gravity knob control
 */
function setupGravityKnob() {
  const knob = document.getElementById('gravityKnob');
  const indicator = knob.querySelector('.knob-indicator');
  const valueDisplay = document.getElementById('gravityValue');
  
  const gravityKnob = new OrbitalsKnob(knob, {
    min: 0,
    max: 100,
    value: 50,
    onChange: (value) => {
      valueDisplay.textContent = Math.round(value) + '%';
      
      // Update indicator rotation
      const rotation = -135 + (value / 100) * 270; // -135° to 135°
      indicator.style.transform = `translateX(-50%) rotate(${rotation}deg)`;
      
      // Send to JUCE
      sendParameterToJUCE('gravity', value);
    }
  });
}

/**
 * @brief Setup solar point vertical slider
 */
function setupSolarPointSlider() {
  const slider = document.getElementById('solarPointSlider');
  const valueDisplay = document.getElementById('solarPointValue');
  
  const solarSlider = new OrbitalsSlider(slider, {
    min: 1,
    max: 127,
    value: 64,
    orientation: 'vertical',
    onChange: (value) => {
      valueDisplay.textContent = Math.round(value);
      sendParameterToJUCE('solarPoint', value);
    }
  });
}

/**
 * @brief Setup orbit arc control
 */
function setupOrbitArc() {
  const arc = document.getElementById('orbitControl');
  const valueDisplay = document.getElementById('orbitValue');
  
  let isDragging = false;
  let currentValue = 50;
  
  const updateArc = (event) => {
    const rect = arc.getBoundingClientRect();
    const centerX = rect.left + rect.width / 2;
    const centerY = rect.top + rect.height;
    
    const dx = event.clientX - centerX;
    const dy = centerY - event.clientY;
    const angle = Math.atan2(dy, dx);
    
    // Map angle (-PI to 0) to value (0 to 100)
    const normalizedAngle = (angle + Math.PI) / Math.PI;
    currentValue = Math.max(0, Math.min(100, normalizedAngle * 100));
    
    valueDisplay.textContent = Math.round(currentValue) + '%';
    
    // Update visual (clip-path for the arc fill)
    const clipPercent = currentValue;
    arc.style.setProperty('--arc-fill', clipPercent + '%');
    
    sendParameterToJUCE('orbit', currentValue);
  };
  
  arc.addEventListener('mousedown', (e) => {
    isDragging = true;
    updateArc(e);
  });
  
  document.addEventListener('mousemove', (e) => {
    if (isDragging) updateArc(e);
  });
  
  document.addEventListener('mouseup', () => {
    isDragging = false;
  });
}

/**
 * @brief Setup inner/outer bias dual slider
 */
function setupBiasSlider() {
  const slider = document.getElementById('biasSlider');
  const handle = slider.querySelector('.bias-handle');
  const valueDisplay = document.getElementById('biasValue');
  
  let isDragging = false;
  let currentValue = 0;
  
  const updateBias = (event) => {
    const rect = slider.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const percent = Math.max(0, Math.min(1, x / rect.width));
    
    // Map 0-1 to -100 to +100
    currentValue = (percent - 0.5) * 200;
    
    // Update handle position
    handle.style.left = (percent * 100) + '%';
    valueDisplay.textContent = Math.round(currentValue);
    
    sendParameterToJUCE('bias', currentValue);
  };
  
  slider.addEventListener('mousedown', (e) => {
    isDragging = true;
    updateBias(e);
  });
  
  document.addEventListener('mousemove', (e) => {
    if (isDragging) updateBias(e);
  });
  
  document.addEventListener('mouseup', () => {
    isDragging = false;
  });
}

/**
 * @brief Setup bypass toggle
 */
function setupBypassToggle() {
  const toggle = document.getElementById('bypassToggle');
  let bypassed = false;
  
  toggle.addEventListener('click', () => {
    bypassed = !bypassed;
    toggle.classList.toggle('active', bypassed);
    sendParameterToJUCE('bypass', bypassed ? 1 : 0);
  });
}

/* ===================================================================
   JUCE COMMUNICATION
   =================================================================== */

/**
 * @brief Send parameter change to JUCE
 * @param {string} param - Parameter name
 * @param {number} value - Parameter value
 */
function sendParameterToJUCE(param, value) {
  // Check if JUCE bridge exists (multiple methods for compatibility)
  const message = {
    type: 'parameterChange',
    parameter: param,
    value: value
  };
  
  // Try Chrome WebView (Windows)
  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.postMessage(message);
    return;
  }
  
  // Try WebKit message handler (macOS)
  if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.message) {
    window.webkit.messageHandlers.message.postMessage(message);
    return;
  }
  
  // Fallback: console log for debugging
  console.log(`Parameter ${param} = ${value}`);
}

/**
 * @brief Receive messages from JUCE
 * @param {Object} message - Message from JUCE
 */
window.receiveMessageFromJUCE = function(message) {
  console.log('Received from JUCE:', message);
  
  // Handle incoming MIDI or parameter updates from JUCE
  if (message.type === 'midiNote') {
    // Visualize MIDI note
    visualizeMIDINote(message.velocity);
  }
};

/**
 * @brief Visualize incoming MIDI note
 * @param {number} velocity - MIDI velocity (0-127)
 */
function visualizeMIDINote(velocity) {
  // Update input meter
  const inputMeter = document.getElementById('inputMeter');
  if (inputMeter) {
    const percent = (velocity / 127) * 100;
    inputMeter.style.setProperty('--meter-height', percent + '%');
  }
  
  // Create particle burst
  const noteCount = document.getElementById('noteCount');
  if (noteCount) {
    const current = parseInt(noteCount.textContent.split(': ')[1] || 0);
    noteCount.textContent = `Notes: ${current + 1}`;
  }
}
