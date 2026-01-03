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
  setupControls();
  
  // Setup canvas
  const canvas = document.getElementById('solarCanvas');
  if (canvas) {
    setupSolarVisualization(canvas);
  }
});

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
   * Visual representation: Particles are brighter at perihelion (closest to sun)
   * This represents higher velocity when closer to the solar point (target velocity)
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
    
    // Brightness based on distance from center (perihelion = closest = brightest)
    // This represents velocity: closer to solar point = higher velocity = brighter
    // Matches MIDI processing where velocities are pulled toward solar point
    const distanceRatio = this.perihelion / Math.max(this.distance, this.perihelion);
    const brightnessMultiplier = 0.5 + distanceRatio * 0.5; // Brighter when closer
    
    // Use particle's base color but adjust brightness based on position
    const particleColor = this.color;
    
    if (this.glow) {
      ctx.shadowColor = particleColor;
      ctx.shadowBlur = this.size * 3 * brightnessMultiplier; // Stronger glow when closer
    }
    
    ctx.fillStyle = particleColor;
    ctx.beginPath();
    ctx.arc(this.x, this.y, this.size * brightnessMultiplier, 0, Math.PI * 2);
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
    const getCanvasSize = () => {
      const rect = canvas.getBoundingClientRect();
      return {
        width: rect.width,
        height: rect.height
      };
    };
    
    // Get the center position - use gravity knob center
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
      // Map gravity 0-100 to orbital multiplier 3.0-0.8 (adjusted so particles stay visible)
      // Minimum multiplier 0.8 ensures particles don't hide behind the knob (knob is ~85px radius)
      const gravityMultiplier = 3.0 - (gravityValue / 100) * 2.2;
      const semiMajorAxis = Math.max(baseSize * gravityMultiplier, 100); // Min 100px radius
      
      // Eccentricity: 0 (circle) to 0.8 (highly elliptical) based on orbit control
      const eccentricity = (orbitValue / 100) * 0.8;
      
      // Orbital plane rotation based on bias
      const orbitalAngle = (biasValue / 100) * Math.PI;
      
      // Random starting position on orbit
      const trueAnomaly = Math.random() * Math.PI * 2;
      
      // Orbital speed varies with gravity (tighter orbits = faster)
      const orbitalSpeed = 0.01 + (gravityValue / 100) * 0.03;
      
      // Particle color based on solar point (higher = brighter/hotter)
      const solarPointPercent = (solarPointValue - 1) / 126;
      const hue = 20 + solarPointPercent * 50;
      const brightness = 50 + solarPointPercent * 30;
      
      // Create particle
      const particle = new OrbitalParticle(centerX, centerY, {
        centerX: centerX,
        centerY: centerY,
        semiMajorAxis: semiMajorAxis,
        eccentricity: eccentricity,
        orbitalAngle: orbitalAngle,
        trueAnomaly: trueAnomaly,
        orbitalSpeed: orbitalSpeed,
        size: 3 + Math.random() * 2.5,
        color: ColorUtils.hslToRgb(hue, 100, brightness),
        trailLength: 60 + Math.floor(orbitValue / 100 * 40), // Longer trails with higher orbit
        showTrail: true,
        glow: true
      });
      
      return particle;
    }
    
    /**
     * @brief Update gravity value - affects pull strength
     * MIDI: pull = gravity * (1 - eccentricity)
     * Visual: Higher gravity = tighter orbits, faster speed, more particles
     */
    window.updateGravityValue = function(value) {
      gravityValue = Math.max(0, Math.min(100, value));
      
      // Update particle count based on gravity
      targetParticleCount = Math.floor(5 + (gravityValue / 100) * 8);
      
      // Update existing particles
      orbitalParticles.forEach(particle => {
        const baseSize = (solarPointValue / 127) * 250 + 50;
        const gravityMultiplier = 3.0 - (gravityValue / 100) * 2.2;
        particle.semiMajorAxis = Math.max(baseSize * gravityMultiplier, 100); // Min 100px
        particle.perihelion = particle.semiMajorAxis * (1 - particle.eccentricity);
        particle.aphelion = particle.semiMajorAxis * (1 + particle.eccentricity);
        particle.orbitalSpeed = 0.01 + (gravityValue / 100) * 0.03;
      });
      
      // Add particles if needed
      while (orbitalParticles.length < targetParticleCount) {
        orbitalParticles.push(createOrbitalParticle());
      }
    };
    
    /**
     * @brief Update orbit value - affects eccentricity
     * MIDI: eccentricity = orbit, also affects pull: pull = gravity * (1 - eccentricity)
     * Visual: Higher orbit = more elliptical orbits, longer trails
     */
    window.updateOrbitValue = function(value) {
      orbitValue = Math.max(0, Math.min(100, value));
      // Update eccentricity and trails
      orbitalParticles.forEach(particle => {
        particle.eccentricity = (orbitValue / 100) * 0.8;
        particle.perihelion = particle.semiMajorAxis * (1 - particle.eccentricity);
        particle.aphelion = particle.semiMajorAxis * (1 + particle.eccentricity);
        // Longer trails for more elliptical orbits
        particle.trailLength = 60 + Math.floor(orbitValue / 100 * 40);
      });
    };
    
    /**
     * @brief Update solar point value - affects target velocity
     * MIDI: distance = (velocity - solarPoint) / 127.0f
     * Visual: Higher solar point = larger base orbits, brighter colors
     */
    window.updateSolarPointValue = function(value) {
      solarPointValue = Math.max(1, Math.min(127, value));
      // Update orbital size and particle colors
      orbitalParticles.forEach(particle => {
        const baseSize = (solarPointValue / 127) * 250 + 50;
        const gravityMultiplier = 3.0 - (gravityValue / 100) * 2.2;
        particle.semiMajorAxis = Math.max(baseSize * gravityMultiplier, 100); // Min 100px
        particle.perihelion = particle.semiMajorAxis * (1 - particle.eccentricity);
        particle.aphelion = particle.semiMajorAxis * (1 + particle.eccentricity);
        
        // Update color based on solar point (higher = brighter/hotter)
        const solarPointPercent = (solarPointValue - 1) / 126;
        const hue = 20 + solarPointPercent * 50;
        const brightness = 50 + solarPointPercent * 30;
        particle.color = ColorUtils.hslToRgb(hue, 100, brightness);
      });
    };
    
    /**
     * @brief Update bias value - affects inner/outer orbit preference
     * MIDI: biasedDistance = distance + (bias * 0.3f)
     * Visual: Rotates orbital plane
     */
    window.updateBiasValue = function(value) {
      biasValue = Math.max(-100, Math.min(100, value));
      // Update orbital plane rotation (matches MIDI bias effect)
      orbitalParticles.forEach(particle => {
        particle.orbitalAngle = (biasValue / 100) * Math.PI;
      });
    };
    
    // Initialize with orbital particles - count varies with gravity
    let targetParticleCount = Math.floor(5 + (gravityValue / 100) * 8); // 5-13 particles
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
      
      // Clear canvas - keep fully transparent background
      ctx.clearRect(0, 0, canvasSize.width, canvasSize.height);
      
      // Draw velocity zones (concentric rings showing velocity ranges)
      const solarPointPercent = (solarPointValue - 1) / 126;
      const zoneCount = 3;
      for (let i = 0; i < zoneCount; i++) {
        const zoneRadius = 120 + i * 70;
        const zoneAlpha = 0.04 * (1 - i / zoneCount) * (0.5 + solarPointPercent * 0.5);
        const zoneHue = 30 + solarPointPercent * 30;
        
        ctx.save();
        ctx.strokeStyle = `hsla(${zoneHue}, 100%, 50%, ${zoneAlpha})`;
        ctx.lineWidth = 1.5;
        ctx.setLineDash([6, 10]);
        ctx.lineDashOffset = time * 8 * (i % 2 === 0 ? 1 : -1);
        ctx.beginPath();
        ctx.arc(centerX, centerY, zoneRadius, 0, Math.PI * 2);
        ctx.stroke();
        ctx.restore();
      }
      
      // Draw gravitational field lines
      const fieldLineCount = Math.floor(6 + (gravityValue / 100) * 10);
      const fieldAlpha = 0.04 + (gravityValue / 100) * 0.06;
      for (let i = 0; i < fieldLineCount; i++) {
        const angle = (Math.PI * 2 * i) / fieldLineCount + time * 0.1;
        const length = 120 + (gravityValue / 100) * 180;
        
        const gradient = ctx.createLinearGradient(
          centerX, centerY,
          centerX + Math.cos(angle) * length, centerY + Math.sin(angle) * length
        );
        gradient.addColorStop(0, `rgba(255, 200, 100, ${fieldAlpha})`);
        gradient.addColorStop(0.7, `rgba(255, 165, 0, ${fieldAlpha * 0.3})`);
        gradient.addColorStop(1, 'rgba(255, 165, 0, 0)');
        
        ctx.save();
        ctx.strokeStyle = gradient;
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(centerX, centerY);
        ctx.lineTo(centerX + Math.cos(angle) * length, centerY + Math.sin(angle) * length);
        ctx.stroke();
        ctx.restore();
      }
      
      // Draw orbital reference rings
      drawOrbitalRings(ctx, centerX, centerY, time, gravityValue, orbitValue);
      
      // Draw bias axis indicator
      if (Math.abs(biasValue) > 5) {
        const biasAngle = (biasValue / 100) * Math.PI;
        const biasLength = 250;
        const biasAlpha = Math.min(Math.abs(biasValue) / 100 * 0.4, 0.4);
        
        ctx.save();
        ctx.translate(centerX, centerY);
        ctx.rotate(biasAngle);
        
        const biasGradient = ctx.createLinearGradient(-biasLength, 0, biasLength, 0);
        biasGradient.addColorStop(0, `rgba(100, 150, 255, ${biasAlpha})`);
        biasGradient.addColorStop(0.5, 'rgba(255, 255, 255, 0)');
        biasGradient.addColorStop(1, `rgba(255, 100, 50, ${biasAlpha})`);
        
        ctx.strokeStyle = biasGradient;
        ctx.lineWidth = 2;
        ctx.setLineDash([8, 8]);
        ctx.lineDashOffset = time * 5;
        ctx.beginPath();
        ctx.moveTo(-biasLength, 0);
        ctx.lineTo(biasLength, 0);
        ctx.stroke();
        
        ctx.fillStyle = `rgba(100, 150, 255, ${biasAlpha * 1.5})`;
        ctx.beginPath();
        ctx.arc(-biasLength, 0, 4, 0, Math.PI * 2);
        ctx.fill();
        
        ctx.fillStyle = `rgba(255, 100, 50, ${biasAlpha * 1.5})`;
        ctx.beginPath();
        ctx.arc(biasLength, 0, 4, 0, Math.PI * 2);
        ctx.fill();
        
        ctx.restore();
      }
      
      // Draw sun corona (outer glow)
      const gravityEffect = 0.7 + (gravityValue / 100) * 0.6;
      const coronaSize = (80 + Math.sin(time * 2) * 10) * gravityEffect;
      const coronaIntensity = 0.12 + (gravityValue / 100) * 0.15;
      const coronaGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, coronaSize);
      coronaGradient.addColorStop(0, `rgba(255, 255, 200, ${coronaIntensity})`);
      coronaGradient.addColorStop(0.3, `rgba(255, 220, 100, ${coronaIntensity * 0.7})`);
      coronaGradient.addColorStop(0.6, `rgba(255, 165, 0, ${coronaIntensity * 0.4})`);
      coronaGradient.addColorStop(1, 'rgba(255, 106, 0, 0)');
      ctx.fillStyle = coronaGradient;
      ctx.beginPath();
      ctx.arc(centerX, centerY, coronaSize, 0, Math.PI * 2);
      ctx.fill();
      
      // Draw central sun
      const solarPointEffect = 0.85 + ((solarPointValue - 1) / 126) * 0.4;
      const sunRadius = (45 + Math.sin(time * 3) * 3) * solarPointEffect;
      const sunBrightness = 0.75 + (gravityValue / 100) * 0.25;
      
      const sunGradient = ctx.createRadialGradient(
        centerX, centerY, 0,
        centerX, centerY, sunRadius
      );
      sunGradient.addColorStop(0, `rgba(255, 255, 255, ${sunBrightness})`);
      sunGradient.addColorStop(0.2, `rgba(255, 255, 136, ${sunBrightness * 0.95})`);
      sunGradient.addColorStop(0.4, `rgba(255, 220, 0, ${sunBrightness * 0.85})`);
      sunGradient.addColorStop(0.7, `rgba(255, 170, 0, ${sunBrightness * 0.7})`);
      sunGradient.addColorStop(1, 'rgba(255, 106, 0, 0)');
      
      ctx.save();
      ctx.shadowColor = '#ffa500';
      ctx.shadowBlur = 40 + gravityValue / 100 * 30;
      ctx.fillStyle = sunGradient;
      ctx.beginPath();
      ctx.arc(centerX, centerY, sunRadius, 0, Math.PI * 2);
      ctx.fill();
      
      // White hot core
      const coreSize = sunRadius * 0.2 * (0.8 + (gravityValue / 100) * 0.4);
      ctx.shadowBlur = 20;
      ctx.fillStyle = `rgba(255, 255, 255, ${0.9 + Math.sin(time * 4) * 0.1})`;
      ctx.beginPath();
      ctx.arc(centerX, centerY, coreSize, 0, Math.PI * 2);
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
 * @brief Draw decorative orbital reference rings
 * @param {CanvasRenderingContext2D} ctx - Canvas context
 * @param {number} centerX - Center X
 * @param {number} centerY - Center Y
 * @param {number} time - Time value
 * @param {number} gravityValue - Gravity value (0-100) for ring spacing
 */
/**
 * @brief Draw orbital reference rings with elliptical distortion
 * @param {CanvasRenderingContext2D} ctx - Canvas context
 * @param {number} centerX - Center X
 * @param {number} centerY - Center Y
 * @param {number} time - Time value
 * @param {number} gravityValue - Gravity value (0-100)
 * @param {number} orbitValue - Orbit/eccentricity value (0-100)
 * Visual: More rings with higher gravity, elliptical shape with higher orbit
 */
function drawOrbitalRings(ctx, centerX, centerY, time, gravityValue = 50, orbitValue = 50) {
  // More rings with higher gravity (stronger gravitational field)
  const tightness = 1 - (gravityValue / 100) * 0.7;
  const ringCount = Math.floor(4 + (gravityValue / 100) * 6); // 4-10 rings
  const baseRadius = 60;
  
  // Elliptical distortion based on orbit value
  const ellipseRatio = 1 - (orbitValue / 100) * 0.35; // More elliptical with higher orbit
  
  for (let i = 0; i < ringCount; i++) {
    const radius = baseRadius + i * 35 * tightness;
    const pulsePhase = time + i * 0.3;
    const pulseAmount = 0.06 + Math.sin(pulsePhase) * 0.03;
    const alpha = pulseAmount * (0.8 + gravityValue / 100 * 0.4); // More visible with gravity
    
    ctx.save();
    ctx.strokeStyle = `rgba(255, 165, 0, ${alpha})`;
    ctx.lineWidth = 0.5 + (gravityValue / 100) * 0.8;
    ctx.setLineDash([4, 10]);
    ctx.lineDashOffset = -time * 12 * (i % 2 === 0 ? 1 : -1); // Counter-rotating
    ctx.beginPath();
    
    // Draw ellipse if orbit value is significant, otherwise circle
    if (orbitValue > 15) {
      ctx.ellipse(centerX, centerY, radius, radius * ellipseRatio, 0, 0, Math.PI * 2);
    } else {
      ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
    }
    ctx.stroke();
    ctx.restore();
  }
}

/* ===================================================================
   CONTROL SETUP
   =================================================================== */

function setupControls() {
  // Orbit Slider (Left)
  const orbitSlider = document.getElementById('orbitSlider');
  if (orbitSlider) {
    new OrbitalsSlider(orbitSlider, {
      min: 0, max: 100, value: 50,
      orientation: 'vertical',
      onChange: (v) => {
        document.getElementById('orbitValue').textContent = Math.round(v) + '%';
        const percent = (v - 0) / (100 - 0);
        orbitSlider.style.setProperty('--slider-value', percent);
        if (window.updateOrbitValue) window.updateOrbitValue(v);
        sendParameterToJUCE('orbit', v);
      }
    });
    orbitSlider.style.setProperty('--slider-value', 0.5);
  }
  
  // Gravity Knob (Center)
  const gravityKnob = document.getElementById('gravityKnob');
  if (gravityKnob) {
    new OrbitalsKnob(gravityKnob, {
      min: 0, max: 100, value: 50,
      onChange: (v) => {
        document.getElementById('gravityValue').textContent = Math.round(v) + '%';
        if (window.updateGravityValue) window.updateGravityValue(v);
        sendParameterToJUCE('gravity', v);
      }
    });
  }
  
  // Solar Point Slider (Right)
  const solarPointSlider = document.getElementById('solarPointSlider');
  if (solarPointSlider) {
    new OrbitalsSlider(solarPointSlider, {
      min: 1, max: 127, value: 64,
      orientation: 'vertical',
      onChange: (v) => {
        document.getElementById('solarPointValue').textContent = Math.round(v);
        const percent = (v - 1) / (127 - 1);
        solarPointSlider.style.setProperty('--slider-value', percent);
        if (window.updateSolarPointValue) window.updateSolarPointValue(v);
        sendParameterToJUCE('solarPoint', v);
      }
    });
    const initialPercent = (64 - 1) / (127 - 1);
    solarPointSlider.style.setProperty('--slider-value', initialPercent);
  }
  
  // Bias Slider (Bottom)
  const biasSlider = document.getElementById('biasSlider');
  if (biasSlider) {
    new OrbitalsSlider(biasSlider, {
      min: -100, max: 100, value: 0,
      orientation: 'horizontal',
      onChange: (v) => {
        document.getElementById('biasValue').textContent = Math.round(v);
        const percent = (v - (-100)) / (100 - (-100));
        biasSlider.style.setProperty('--slider-value', percent);
        if (window.updateBiasValue) window.updateBiasValue(v);
        sendParameterToJUCE('bias', v);
      }
    });
    biasSlider.style.setProperty('--slider-value', 0.5);
  }
  
  // Setup bypass toggle
  if (window.setupBypassToggle) {
    window.setupBypassToggle(sendParameterToJUCE);
  }
}

function sendParameterToJUCE(param, value) {
  if (window.chrome?.webview) {
    window.chrome.webview.postMessage({ type: 'parameterChange', parameter: param, value });
  }
}

// Receive messages from JUCE
window.receiveMessageFromJUCE = function(message) {
  if (!message || typeof message !== 'object') return;
  
  if (message.type === 'midiNote') {
    const noteCountEl = document.getElementById('noteCount');
    if (noteCountEl) {
      const currentCount = parseInt(noteCountEl.textContent.match(/\d+/)?.[0] || '0');
      noteCountEl.textContent = `Notes: ${currentCount + 1}`;
      clearTimeout(window.noteCountResetTimeout);
      window.noteCountResetTimeout = setTimeout(() => {
        noteCountEl.textContent = 'Notes: 0';
      }, 2000);
    }
  } else if (message.type === 'parameterUpdate') {
    const param = message.parameter;
    const value = message.value;
    
    if (param === 'orbit') {
      const slider = document.getElementById('orbitSlider');
      if (slider && slider._orbitalsSlider) {
        slider._orbitalsSlider.setValue(value * 100);
      }
    } else if (param === 'gravity') {
      const knob = document.getElementById('gravityKnob');
      if (knob && knob._orbitalsKnob) {
        knob._orbitalsKnob.setValue(value * 100);
      }
    } else if (param === 'solarPoint') {
      const slider = document.getElementById('solarPointSlider');
      if (slider && slider._orbitalsSlider) {
        slider._orbitalsSlider.setValue(Math.round(value * 126) + 1);
      }
    } else if (param === 'bias') {
      const slider = document.getElementById('biasSlider');
      if (slider && slider._orbitalsSlider) {
        slider._orbitalsSlider.setValue((value * 200) - 100);
      }
    }
  }
};
