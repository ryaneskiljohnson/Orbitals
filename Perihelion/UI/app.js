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
  setupOrbitSlider();
  setupBiasSlider();
  setupBypassToggle();
  
  console.log('Perihelion initialized!');
}

/* ===================================================================
   SOLAR VISUALIZATION
   =================================================================== */

/**
 * @brief Orbital particle for Perihelion - follows elliptical orbits
 */
class OrbitalParticle {
  constructor(x, y, options = {}) {
    this.centerX = options.centerX || x;
    this.centerY = options.centerY || y;
    
    // Orbital parameters
    this.semiMajorAxis = options.semiMajorAxis || 100;  // a
    this.eccentricity = options.eccentricity || 0.6;    // e (0 = circle, 1 = line)
    this.orbitalAngle = options.orbitalAngle || 0;      // Rotation of orbit plane
    this.trueAnomaly = options.trueAnomaly || 0;        // Position on orbit (0-2π)
    this.orbitalSpeed = options.orbitalSpeed || 0.02;   // Angular velocity
    
    // Calculate initial position
    this.updatePosition();
    
    // Visual properties
    this.size = options.size || 2;
    this.color = options.color || '#ffa500';
    this.alpha = options.alpha || 1;
    this.life = options.life || Infinity;
    this.maxLife = options.maxLife || Infinity;
    this.decay = options.decay || 0;
    this.glow = options.glow !== false;
    
    // Trail for orbital path visualization
    this.trail = [];
    this.trailLength = options.trailLength || 60;
    this.showTrail = options.showTrail !== false;
    
    // Initialize trail with current position
    if (this.showTrail) {
      for (let i = 0; i < this.trailLength; i++) {
        this.trail.push({ x: this.x, y: this.y, distance: this.distance });
      }
    }
  }
  
  /**
   * @brief Calculate position on elliptical orbit using Kepler's laws
   */
  updatePosition() {
    // Elliptical orbit calculation
    const a = this.semiMajorAxis;
    const e = this.eccentricity;
    const b = a * Math.sqrt(1 - e * e); // Semi-minor axis
    
    // Position on ellipse (before rotation)
    const xEllipse = a * Math.cos(this.trueAnomaly);
    const yEllipse = b * Math.sin(this.trueAnomaly);
    
    // Rotate orbit plane
    const cosAngle = Math.cos(this.orbitalAngle);
    const sinAngle = Math.sin(this.orbitalAngle);
    const xRotated = xEllipse * cosAngle - yEllipse * sinAngle;
    const yRotated = xEllipse * sinAngle + yEllipse * cosAngle;
    
    // Position relative to center
    this.x = this.centerX + xRotated;
    this.y = this.centerY + yRotated;
    
    // Distance from center (for perihelion/aphelion)
    this.distance = Math.sqrt(xRotated * xRotated + yRotated * yRotated);
    this.perihelion = a * (1 - e); // Closest distance
    this.aphelion = a * (1 + e);   // Farthest distance
  }
  
  /**
   * @brief Update orbital motion with Kepler's laws
   * @param {number} deltaTime - Time since last update (ms)
   */
  update(deltaTime) {
    const dt = deltaTime / 16.67; // Normalize to 60fps
    
    // Kepler's second law: speed varies with distance
    // Particles move faster at perihelion (closest to sun)
    const distanceRatio = this.perihelion / this.distance;
    const speedMultiplier = distanceRatio * distanceRatio; // Inverse square law
    
    // Update true anomaly (position on orbit)
    this.trueAnomaly += this.orbitalSpeed * speedMultiplier * dt;
    if (this.trueAnomaly >= Math.PI * 2) {
      this.trueAnomaly -= Math.PI * 2;
    }
    
    // Update position
    this.updatePosition();
    
    // Update trail
    if (this.showTrail && this.trailLength > 0) {
      this.trail.push({ x: this.x, y: this.y, distance: this.distance });
      if (this.trail.length > this.trailLength) {
        this.trail.shift();
      }
    }
    
    // Life decay
    if (this.decay > 0) {
      this.life -= this.decay * dt;
      this.alpha = Math.max(0, this.life / this.maxLife);
    }
    
    return this.life > 0;
  }
  
  /**
   * @brief Draw particle and orbital trail
   * @param {CanvasRenderingContext2D} ctx - Canvas context
   */
  draw(ctx) {
    // Draw trail (orbital path)
    if (this.showTrail && this.trail.length > 1) {
      ctx.save();
      ctx.strokeStyle = `rgba(255, 165, 0, ${this.alpha * 0.2})`;
      ctx.lineWidth = 1;
      ctx.beginPath();
      
      // Draw trail with fading opacity
      this.trail.forEach((point, i) => {
        const trailAlpha = (i / this.trail.length) * this.alpha * 0.3;
        ctx.strokeStyle = `rgba(255, 165, 0, ${trailAlpha})`;
        if (i === 0) {
          ctx.moveTo(point.x, point.y);
        } else {
          ctx.lineTo(point.x, point.y);
        }
      });
      ctx.stroke();
      ctx.restore();
    }
    
    // Draw particle
    ctx.save();
    ctx.globalAlpha = this.alpha;
    
    // Brightness based on distance (brighter at perihelion)
    const brightness = 0.5 + (this.perihelion / this.distance) * 0.5;
    const particleColor = this.color;
    
    if (this.glow) {
      ctx.shadowColor = particleColor;
      ctx.shadowBlur = this.size * 3;
    }
    
    ctx.fillStyle = particleColor;
    ctx.beginPath();
    ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
    ctx.fill();
    
    ctx.restore();
  }
}

function setupSolarVisualization(canvas) {
  // Wait for layout to be ready
  const initCanvas = () => {
    let ctx = CanvasUtils.setupHiDPI(canvas);
    
    // Get the parent container (plugin-main)
    const container = canvas.parentElement;
    
    // Get canvas CSS dimensions (for drawing coordinates)
    // After setupHiDPI, ctx.scale(DPR, DPR) is applied, so we draw in CSS pixels
    const getCanvasSize = () => {
      // Get the actual canvas bounding rect - this gives us the exact drawing area
      const rect = canvas.getBoundingClientRect();
      return {
        width: rect.width,
        height: rect.height
      };
    };
    
    // Get the center position
    // Use mathematical center with a small offset to account for UI asymmetry
    const getCenter = () => {
      const size = getCanvasSize();
      const canvasRect = canvas.getBoundingClientRect();
      
      // Find the actual Gravity knob position
      const gravityKnob = document.getElementById('gravityKnob');
      if (gravityKnob) {
        const knobRect = gravityKnob.getBoundingClientRect();
        // Calculate knob center relative to canvas coordinate system
        const knobCenterX = knobRect.left + knobRect.width / 2 - canvasRect.left;
        const knobCenterY = knobRect.top + knobRect.height / 2 - canvasRect.top;
        
        return {
          x: knobCenterX,
          y: knobCenterY
        };
      }
      
      // Fallback to mathematical center
      return {
        x: size.width / 2,
        y: size.height / 2
      };
    };
    
    let canvasSize = getCanvasSize();
    let center = getCenter();
    let centerX = center.x;
    let centerY = center.y;
    
    // Orbital particles array
    const orbitalParticles = [];
    let time = 0;
    
    // Control values (will be updated by UI controls)
    let gravityValue = 50;      // 0-100: affects orbital tightness (semi-major axis)
    let orbitValue = 50;        // 0-100: affects eccentricity and number of orbits
    let solarPointValue = 64;   // 1-127: affects base orbital size
    let biasValue = 0;          // -100 to +100: affects orbit plane rotation
    
    // Handle canvas and container resize
    const resizeObserver = new ResizeObserver(() => {
      ctx = CanvasUtils.setupHiDPI(canvas);
      // Wait a frame for layout to settle
      requestAnimationFrame(() => {
        canvasSize = getCanvasSize();
        center = getCenter();
        centerX = center.x;
        centerY = center.y;
        
        // Update all particle centers
        orbitalParticles.forEach(particle => {
          particle.centerX = centerX;
          particle.centerY = centerY;
          particle.updatePosition(); // Recalculate position immediately
        });
      });
    });
    resizeObserver.observe(canvas);
    if (container) {
      resizeObserver.observe(container);
    }
    
    /**
     * @brief Create a new orbital particle
     */
    function createOrbitalParticle() {
      // Calculate orbital parameters based on controls
      // Solar point affects base size: 50-300px
      const baseSize = (solarPointValue / 127) * 250 + 50;
      
      // Gravity affects orbital distance: 0% = very far out, 100% = tight orbits
      // Map gravity 0-100 to orbital multiplier 3.0-0.3 (10x range)
      // At 0% gravity: 3.0x = very far out
      // At 100% gravity: 0.3x = tight orbits
      const gravityMultiplier = 3.0 - (gravityValue / 100) * 2.7;
      const semiMajorAxis = baseSize * gravityMultiplier;
      
      // Eccentricity: 0 (circle) to 0.8 (highly elliptical) based on orbit control
      const eccentricity = (orbitValue / 100) * 0.8;
      
      // Orbital plane rotation based on bias
      const orbitalAngle = (biasValue / 100) * Math.PI;
      
      // Random starting position on orbit
      const trueAnomaly = Math.random() * Math.PI * 2;
      
      // Orbital speed varies with gravity (tighter orbits = faster)
      const orbitalSpeed = 0.01 + (gravityValue / 100) * 0.03;
      
      // Create particle
      const particle = new OrbitalParticle(centerX, centerY, {
        centerX: centerX,
        centerY: centerY,
        semiMajorAxis: semiMajorAxis,
        eccentricity: eccentricity,
        orbitalAngle: orbitalAngle,
        trueAnomaly: trueAnomaly,
        orbitalSpeed: orbitalSpeed,
        size: 2 + Math.random() * 2,
        color: ColorUtils.hslToRgb(20 + Math.random() * 50, 100, 50 + Math.random() * 30),
        trailLength: 80,
        showTrail: true,
        glow: true
      });
      
      return particle;
    }
    
    // Expose control value setters
    window.updateGravityValue = function(value) {
      gravityValue = Math.max(0, Math.min(100, value));
      // Update existing particles' orbital parameters
      orbitalParticles.forEach(particle => {
        const baseSize = (solarPointValue / 127) * 250 + 50;
        // Map gravity 0-100 to orbital multiplier 3.0-0.3
        const gravityMultiplier = 3.0 - (gravityValue / 100) * 2.7;
        particle.semiMajorAxis = baseSize * gravityMultiplier;
        particle.perihelion = particle.semiMajorAxis * (1 - particle.eccentricity);
        particle.aphelion = particle.semiMajorAxis * (1 + particle.eccentricity);
        // Orbital speed: faster with more gravity (tighter orbits)
        particle.orbitalSpeed = 0.01 + (gravityValue / 100) * 0.03;
      });
    };
    
    window.updateOrbitValue = function(value) {
      orbitValue = Math.max(0, Math.min(100, value));
      // Update eccentricity
      orbitalParticles.forEach(particle => {
        particle.eccentricity = (orbitValue / 100) * 0.8;
        particle.perihelion = particle.semiMajorAxis * (1 - particle.eccentricity);
        particle.aphelion = particle.semiMajorAxis * (1 + particle.eccentricity);
      });
    };
    
    window.updateSolarPointValue = function(value) {
      solarPointValue = Math.max(1, Math.min(127, value));
      // Update orbital size
      orbitalParticles.forEach(particle => {
        const baseSize = (solarPointValue / 127) * 250 + 50;
        const gravityMultiplier = 3.0 - (gravityValue / 100) * 2.7;
        particle.semiMajorAxis = baseSize * gravityMultiplier;
        particle.perihelion = particle.semiMajorAxis * (1 - particle.eccentricity);
        particle.aphelion = particle.semiMajorAxis * (1 + particle.eccentricity);
      });
    };
    
    window.updateBiasValue = function(value) {
      biasValue = Math.max(-100, Math.min(100, value));
      // Update orbital plane rotation
      orbitalParticles.forEach(particle => {
        particle.orbitalAngle = (biasValue / 100) * Math.PI;
      });
    };
    
    // Initialize with some orbital particles
    const targetParticleCount = 8; // Number of simultaneous orbits
    for (let i = 0; i < targetParticleCount; i++) {
      orbitalParticles.push(createOrbitalParticle());
    }
    
    // Animation loop
    animationManager.add('perihelion-solar', (deltaTime) => {
      time += deltaTime * 0.001;
      
      // Update canvas size and center every frame to ensure perfect centering
      canvasSize = getCanvasSize();
      center = getCenter();
      centerX = center.x;
      centerY = center.y;
      
      // Update all particle centers to match
      orbitalParticles.forEach(particle => {
        particle.centerX = centerX;
        particle.centerY = centerY;
      });
      
      // Clear canvas with trail effect
      // Use transparent clear instead of dark fill to let background show through
      ctx.clearRect(0, 0, canvasSize.width, canvasSize.height);
      
      // Draw orbital reference rings (subtle guides)
      drawOrbitalRings(ctx, centerX, centerY, time, gravityValue);
      
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
        centerX, centerY, 0,
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
      
      // Update and draw orbital particles
      orbitalParticles.forEach((particle, index) => {
        particle.update(deltaTime);
        particle.draw(ctx);
      });
      
      // Maintain particle count - replace if any die
      while (orbitalParticles.length < targetParticleCount) {
        orbitalParticles.push(createOrbitalParticle());
      }
      
      // Remove particles that are too far out or dead
      for (let i = orbitalParticles.length - 1; i >= 0; i--) {
        const particle = orbitalParticles[i];
        const maxDistance = Math.max(canvasSize.width, canvasSize.height) * 0.6;
        if (particle.distance > maxDistance || particle.alpha <= 0) {
          orbitalParticles.splice(i, 1);
          orbitalParticles.push(createOrbitalParticle());
        }
      }
    });
  };
  
  // Initialize immediately if DOM is ready, otherwise wait
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initCanvas);
  } else {
    // Use requestAnimationFrame to ensure layout is complete
    requestAnimationFrame(() => {
      setTimeout(initCanvas, 0);
    });
  }
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
 * @brief Draw decorative orbital reference rings
 * @param {CanvasRenderingContext2D} ctx - Canvas context
 * @param {number} centerX - Center X
 * @param {number} centerY - Center Y
 * @param {number} time - Time value
 * @param {number} gravityValue - Gravity value (0-100) for ring spacing
 */
function drawOrbitalRings(ctx, centerX, centerY, time, gravityValue = 50) {
  // Draw subtle reference rings based on gravity (tighter orbits = more rings)
  const tightness = 1 - (gravityValue / 100) * 0.7;
  const ringCount = Math.floor(3 + (gravityValue / 100) * 4); // 3-7 rings
  const baseRadius = 60;
  
  for (let i = 0; i < ringCount; i++) {
    const radius = baseRadius + i * 40 * tightness;
    const pulsePhase = time + i * 0.3;
    const alpha = 0.05 + Math.sin(pulsePhase) * 0.02;
    
    // Subtle dashed ring
    ctx.save();
    ctx.strokeStyle = `rgba(255, 165, 0, ${alpha})`;
    ctx.lineWidth = 0.5;
    ctx.setLineDash([3, 8]);
    ctx.lineDashOffset = -time * 15 * (i % 2 === 0 ? 1 : -1);
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
      
      // Update orbital system - gravity affects orbital tightness
      if (window.updateGravityValue) {
        window.updateGravityValue(value);
      }
      
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
  
  if (!slider) {
    console.error('Solar point slider not found');
    return;
  }
  
  const fill = slider.querySelector('.slider-fill');
  console.log('Solar slider found:', slider);
  console.log('Solar fill element:', fill);
  console.log('Solar slider computed style:', window.getComputedStyle(slider));
  if (fill) {
    console.log('Solar fill computed style:', window.getComputedStyle(fill));
    console.log('Solar fill height:', fill.offsetHeight);
  }
  
  const solarSlider = new OrbitalsSlider(slider, {
    min: 1,
    max: 127,
    value: 64,
    orientation: 'vertical',
    onChange: (value) => {
      valueDisplay.textContent = Math.round(value);
      
      // Ensure fill updates - force CSS variable update
      const percent = (value - 1) / (127 - 1);
      slider.style.setProperty('--slider-value', percent);
      
      console.log('Solar slider value changed:', value, 'percent:', percent);
      console.log('Solar slider --slider-value:', slider.style.getPropertyValue('--slider-value'));
      if (fill) {
        const fillHeight = window.getComputedStyle(fill).height;
        console.log('Solar fill height after update:', fillHeight);
      }
      
      // Update orbital system - solar point affects orbital size
      if (window.updateSolarPointValue) {
        window.updateSolarPointValue(value);
      }
      
      sendParameterToJUCE('solarPoint', value);
    }
  });
  
  // Force initial fill display
  const initialPercent = (64 - 1) / (127 - 1);
  slider.style.setProperty('--slider-value', initialPercent);
  console.log('Solar slider initial --slider-value set to:', slider.style.getPropertyValue('--slider-value'));
  if (fill) {
    setTimeout(() => {
      const fillHeight = window.getComputedStyle(fill).height;
      console.log('Solar fill initial height:', fillHeight);
      console.log('Solar fill display:', window.getComputedStyle(fill).display);
      console.log('Solar fill visibility:', window.getComputedStyle(fill).visibility);
      console.log('Solar fill opacity:', window.getComputedStyle(fill).opacity);
      console.log('Solar fill z-index:', window.getComputedStyle(fill).zIndex);
    }, 100);
  }
}

/**
 * @brief Setup orbit arc control - controls orbital eccentricity
 */
function setupOrbitSlider() {
  const slider = document.getElementById('orbitSlider');
  const valueDisplay = document.getElementById('orbitValue');
  
  if (!slider) {
    console.error('Orbit slider not found');
    return;
  }
  
  const fill = slider.querySelector('.slider-fill');
  console.log('Orbit slider found:', slider);
  console.log('Orbit fill element:', fill);
  console.log('Orbit slider computed style:', window.getComputedStyle(slider));
  if (fill) {
    console.log('Orbit fill computed style:', window.getComputedStyle(fill));
    console.log('Orbit fill height:', fill.offsetHeight);
  }
  
  // Use OrbitalsSlider component for consistent behavior
  const orbitSlider = new OrbitalsSlider(slider, {
    min: 0,
    max: 100,
    value: 50,
    step: 1,
    orientation: 'vertical',
    onChange: (value) => {
      valueDisplay.textContent = Math.round(value) + '%';
      
      // Ensure fill updates - force CSS variable update
      const percent = (value - 0) / (100 - 0);
      slider.style.setProperty('--slider-value', percent);
      
      console.log('Orbit slider value changed:', value, 'percent:', percent);
      console.log('Orbit slider --slider-value:', slider.style.getPropertyValue('--slider-value'));
      if (fill) {
        const fillHeight = window.getComputedStyle(fill).height;
        console.log('Orbit fill height after update:', fillHeight);
      }
      
      // Update orbital system - orbit affects eccentricity
      if (window.updateOrbitValue) {
        window.updateOrbitValue(value);
      }
      
      sendParameterToJUCE('orbit', value);
    }
  });
  
  // Force initial fill display
  slider.style.setProperty('--slider-value', 0.5);
  console.log('Orbit slider initial --slider-value set to:', slider.style.getPropertyValue('--slider-value'));
  if (fill) {
    setTimeout(() => {
      const fillHeight = window.getComputedStyle(fill).height;
      console.log('Orbit fill initial height:', fillHeight);
      console.log('Orbit fill display:', window.getComputedStyle(fill).display);
      console.log('Orbit fill visibility:', window.getComputedStyle(fill).visibility);
      console.log('Orbit fill opacity:', window.getComputedStyle(fill).opacity);
      console.log('Orbit fill z-index:', window.getComputedStyle(fill).zIndex);
    }, 100);
  }
}

/**
 * @brief Setup inner/outer bias dual slider - controls orbital plane rotation
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
    
    // Update orbital system - bias affects orbital plane rotation
    if (window.updateBiasValue) {
      window.updateBiasValue(currentValue);
    }
    
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
  // Use shared bypass toggle implementation
  if (window.setupBypassToggle) {
    window.setupBypassToggle(sendParameterToJUCE);
  }
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
