/**
 * @fileoverview Orbitals Particle System Engine
 * @module Orbitals Particles
 * 
 * Provides particle system for visual effects across all Orbitals plugins
 */

/* ===================================================================
   PARTICLE CLASS
   =================================================================== */

/**
 * @brief Single particle with physics properties
 */
class Particle {
  constructor(x, y, options = {}) {
    this.x = x;
    this.y = y;
    this.vx = options.vx || 0;
    this.vy = options.vy || 0;
    this.ax = options.ax || 0;
    this.ay = options.ay || 0;
    this.size = options.size || 2;
    this.color = options.color || '#00d4ff';
    this.alpha = options.alpha || 1;
    this.life = options.life || 1;
    this.maxLife = options.maxLife || 1;
    this.decay = options.decay || 0.01;
    this.glow = options.glow || true;
    this.trail = options.trail || [];
    this.trailLength = options.trailLength || 0;
  }
  
  /**
   * @brief Update particle physics
   * @param {number} deltaTime - Time since last update (ms)
   */
  update(deltaTime) {
    const dt = deltaTime / 16.67; // Normalize to 60fps
    
    // Apply acceleration
    this.vx += this.ax * dt;
    this.vy += this.ay * dt;
    
    // Update position
    this.x += this.vx * dt;
    this.y += this.vy * dt;
    
    // Trail management
    if (this.trailLength > 0) {
      this.trail.push({ x: this.x, y: this.y, alpha: this.alpha });
      if (this.trail.length > this.trailLength) {
        this.trail.shift();
      }
    }
    
    // Life decay
    this.life -= this.decay * dt;
    this.alpha = Math.max(0, this.life / this.maxLife);
    
    return this.life > 0;
  }
  
  /**
   * @brief Draw particle to canvas
   * @param {CanvasRenderingContext2D} ctx - Canvas context
   */
  draw(ctx) {
    if (this.alpha <= 0) return;
    
    ctx.save();
    
    // Draw trail
    if (this.trail.length > 0) {
      for (let i = 0; i < this.trail.length - 1; i++) {
        const point = this.trail[i];
        const nextPoint = this.trail[i + 1];
        const trailAlpha = (i / this.trail.length) * this.alpha * 0.5;
        
        ctx.strokeStyle = this.color.replace(')', `, ${trailAlpha})`).replace('rgb', 'rgba');
        ctx.lineWidth = this.size * (i / this.trail.length);
        ctx.beginPath();
        ctx.moveTo(point.x, point.y);
        ctx.lineTo(nextPoint.x, nextPoint.y);
        ctx.stroke();
      }
    }
    
    // Draw particle with glow
    if (this.glow) {
      ctx.shadowColor = this.color;
      ctx.shadowBlur = this.size * 3;
    }
    
    ctx.fillStyle = this.color.includes('rgba') 
      ? this.color.replace(/[\d.]+\)$/g, `${this.alpha})`)
      : this.color.replace(')', `, ${this.alpha})`).replace('rgb', 'rgba');
    
    ctx.beginPath();
    ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
    ctx.fill();
    
    ctx.restore();
  }
}

/* ===================================================================
   PARTICLE SYSTEM
   =================================================================== */

/**
 * @brief Manages collection of particles with emission and physics
 */
class ParticleSystem {
  constructor(canvas, options = {}) {
    this.canvas = canvas;
    this.ctx = canvas.getContext('2d');
    this.particles = [];
    this.options = {
      maxParticles: options.maxParticles || 500,
      emissionRate: options.emissionRate || 0, // particles per frame
      emitterX: options.emitterX || 0,
      emitterY: options.emitterY || 0,
      ...options
    };
    this.time = 0;
  }
  
  /**
   * @brief Add a single particle
   * @param {Particle} particle - Particle instance
   */
  addParticle(particle) {
    if (this.particles.length < this.options.maxParticles) {
      this.particles.push(particle);
    }
  }
  
  /**
   * @brief Emit particles from emitter position
   * @param {number} count - Number of particles to emit
   * @param {Object} particleOptions - Options for new particles
   */
  emit(count, particleOptions = {}) {
    for (let i = 0; i < count; i++) {
      const angle = particleOptions.angle || Math.random() * Math.PI * 2;
      const speed = particleOptions.speed || 1 + Math.random() * 2;
      const spread = particleOptions.spread || 0;
      
      const particle = new Particle(
        this.options.emitterX + (Math.random() - 0.5) * spread,
        this.options.emitterY + (Math.random() - 0.5) * spread,
        {
          vx: Math.cos(angle) * speed,
          vy: Math.sin(angle) * speed,
          size: particleOptions.size || 2,
          color: particleOptions.color || '#00d4ff',
          life: particleOptions.life || 1,
          maxLife: particleOptions.life || 1,
          decay: particleOptions.decay || 0.01,
          glow: particleOptions.glow !== undefined ? particleOptions.glow : true,
          trailLength: particleOptions.trailLength || 0,
          ax: particleOptions.ax || 0,
          ay: particleOptions.ay || 0
        }
      );
      
      this.addParticle(particle);
    }
  }
  
  /**
   * @brief Update all particles
   * @param {number} deltaTime - Time since last update (ms)
   */
  update(deltaTime) {
    this.time += deltaTime;
    
    // Automatic emission if rate is set
    if (this.options.emissionRate > 0) {
      const emitCount = Math.random() < this.options.emissionRate ? 1 : 0;
      if (emitCount > 0) {
        this.emit(emitCount, this.options.particleDefaults || {});
      }
    }
    
    // Update all particles and remove dead ones
    this.particles = this.particles.filter(particle => particle.update(deltaTime));
  }
  
  /**
   * @brief Draw all particles
   */
  draw() {
    this.particles.forEach(particle => particle.draw(this.ctx));
  }
  
  /**
   * @brief Clear all particles
   */
  clear() {
    this.particles = [];
  }
  
  /**
   * @brief Get particle count
   * @returns {number} Number of active particles
   */
  count() {
    return this.particles.length;
  }
  
  /**
   * @brief Set emitter position
   * @param {number} x - X coordinate
   * @param {number} y - Y coordinate
   */
  setEmitter(x, y) {
    this.options.emitterX = x;
    this.options.emitterY = y;
  }
}

/* ===================================================================
   PRESET PARTICLE EFFECTS
   =================================================================== */

/**
 * @brief Common particle effect presets
 */
const ParticlePresets = {
  /**
   * @brief Solar flare effect (for Perihelion)
   */
  solarFlare(particleSystem, x, y, intensity = 1) {
    particleSystem.emit(Math.floor(10 * intensity), {
      angle: Math.random() * Math.PI * 2,
      speed: 2 + Math.random() * 3,
      spread: 5,
      size: 1 + Math.random() * 2,
      color: `hsl(${30 + Math.random() * 30}, 100%, ${50 + Math.random() * 20}%)`,
      life: 0.5 + Math.random() * 0.5,
      decay: 0.02,
      glow: true,
      trailLength: 5
    });
  },
  
  /**
   * @brief Orbital trail effect
   */
  orbitalTrail(particleSystem, x, y, color = '#00d4ff') {
    particleSystem.emit(1, {
      vx: 0,
      vy: 0,
      size: 1.5,
      color: color,
      life: 0.8,
      decay: 0.015,
      glow: true,
      trailLength: 0
    });
  },
  
  /**
   * @brief Stardust sparkle effect
   */
  stardust(particleSystem, x, y, count = 5) {
    particleSystem.emit(count, {
      angle: Math.random() * Math.PI * 2,
      speed: 0.5 + Math.random() * 1.5,
      spread: 10,
      size: 1,
      color: `rgba(255, 255, 255, ${0.8 + Math.random() * 0.2})`,
      life: 0.3 + Math.random() * 0.4,
      decay: 0.03,
      glow: true
    });
  },
  
  /**
   * @brief Velocity indicator (color-coded)
   */
  velocityParticle(particleSystem, x, y, velocity) {
    // Color from blue (low) to red (high)
    const hue = 240 - (velocity / 127) * 240; // 240 = blue, 0 = red
    
    particleSystem.emit(1, {
      vx: (Math.random() - 0.5) * 2,
      vy: (Math.random() - 0.5) * 2,
      size: 2 + (velocity / 127) * 2,
      color: `hsl(${hue}, 100%, 60%)`,
      life: 1,
      decay: 0.01,
      glow: true,
      trailLength: 3
    });
  },
  
  /**
   * @brief Wave ripple effect (for Tidal)
   */
  waveRipple(particleSystem, x, y, phase) {
    const count = 8;
    for (let i = 0; i < count; i++) {
      const angle = (i / count) * Math.PI * 2 + phase;
      particleSystem.emit(1, {
        vx: Math.cos(angle) * 0.5,
        vy: Math.sin(angle) * 0.5,
        size: 1.5,
        color: 'rgba(59, 130, 246, 0.8)',
        life: 0.6,
        decay: 0.02,
        glow: true
      });
    }
  },
  
  /**
   * @brief Magnetic field lines (for Ion)
   */
  magneticPulse(particleSystem, x, y, toX, toY) {
    const dx = toX - x;
    const dy = toY - y;
    const angle = Math.atan2(dy, dx);
    
    particleSystem.emit(3, {
      vx: Math.cos(angle) * 3,
      vy: Math.sin(angle) * 3,
      size: 1,
      color: 'rgba(139, 92, 246, 0.9)',
      life: 0.4,
      decay: 0.025,
      glow: true,
      trailLength: 5
    });
  },
  
  /**
   * @brief Eclipse shadow particles
   */
  shadowParticle(particleSystem, x, y, hidden = false) {
    particleSystem.emit(1, {
      vx: 0,
      vy: 0,
      size: 2,
      color: hidden ? 'rgba(0, 0, 0, 0.8)' : 'rgba(251, 191, 36, 0.9)',
      life: 0.5,
      decay: 0.02,
      glow: !hidden
    });
  }
};

/* ===================================================================
   GRAVITATIONAL PARTICLE SYSTEM
   =================================================================== */

/**
 * @brief Particle system with gravitational attraction
 * @note Used for Perihelion, Lagrange, etc.
 */
class GravitationalParticleSystem extends ParticleSystem {
  constructor(canvas, options = {}) {
    super(canvas, options);
    this.attractors = [];
  }
  
  /**
   * @brief Add gravitational attractor point
   * @param {number} x - X coordinate
   * @param {number} y - Y coordinate
   * @param {number} strength - Gravitational strength
   */
  addAttractor(x, y, strength = 0.1) {
    this.attractors.push({ x, y, strength });
  }
  
  /**
   * @brief Clear all attractors
   */
  clearAttractors() {
    this.attractors = [];
  }
  
  /**
   * @brief Update with gravitational physics
   * @param {number} deltaTime - Time since last update
   */
  update(deltaTime) {
    // Apply gravitational forces
    this.particles.forEach(particle => {
      this.attractors.forEach(attractor => {
        const dx = attractor.x - particle.x;
        const dy = attractor.y - particle.y;
        const distSq = dx * dx + dy * dy;
        const dist = Math.sqrt(distSq);
        
        if (dist > 1) {
          const force = attractor.strength / distSq;
          particle.ax = (dx / dist) * force;
          particle.ay = (dy / dist) * force;
        }
      });
    });
    
    // Call parent update
    super.update(deltaTime);
  }
}

/* ===================================================================
   EXPORT FOR USE
   =================================================================== */

if (typeof window !== 'undefined') {
  window.Particle = Particle;
  window.ParticleSystem = ParticleSystem;
  window.GravitationalParticleSystem = GravitationalParticleSystem;
  window.ParticlePresets = ParticlePresets;
}
