/**
 * @fileoverview Mandelbrot Animations Library - Canvas Animation Utilities
 * @module Mandelbrot Animations
 * 
 * Provides animation helpers and easing functions for smooth visual effects
 */

/* ===================================================================
   EASING FUNCTIONS
   =================================================================== */

/**
 * @brief Collection of easing functions for smooth animations
 */
const Easing = {
  linear: t => t,
  
  easeInQuad: t => t * t,
  easeOutQuad: t => t * (2 - t),
  easeInOutQuad: t => t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t,
  
  easeInCubic: t => t * t * t,
  easeOutCubic: t => (--t) * t * t + 1,
  easeInOutCubic: t => t < 0.5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1,
  
  easeInSine: t => 1 - Math.cos((t * Math.PI) / 2),
  easeOutSine: t => Math.sin((t * Math.PI) / 2),
  easeInOutSine: t => -(Math.cos(Math.PI * t) - 1) / 2,
  
  easeInExpo: t => t === 0 ? 0 : Math.pow(2, 10 * t - 10),
  easeOutExpo: t => t === 1 ? 1 : 1 - Math.pow(2, -10 * t),
  
  easeInCirc: t => 1 - Math.sqrt(1 - Math.pow(t, 2)),
  easeOutCirc: t => Math.sqrt(1 - Math.pow(t - 1, 2)),
  
  easeInBack: t => {
    const c1 = 1.70158;
    const c3 = c1 + 1;
    return c3 * t * t * t - c1 * t * t;
  },
  easeOutBack: t => {
    const c1 = 1.70158;
    const c3 = c1 + 1;
    return 1 + c3 * Math.pow(t - 1, 3) + c1 * Math.pow(t - 1, 2);
  },
  
  easeInElastic: t => {
    const c4 = (2 * Math.PI) / 3;
    return t === 0 ? 0 : t === 1 ? 1 : -Math.pow(2, 10 * t - 10) * Math.sin((t * 10 - 10.75) * c4);
  },
  easeOutElastic: t => {
    const c4 = (2 * Math.PI) / 3;
    return t === 0 ? 0 : t === 1 ? 1 : Math.pow(2, -10 * t) * Math.sin((t * 10 - 0.75) * c4) + 1;
  }
};

/* ===================================================================
   ANIMATION FRAME MANAGER
   =================================================================== */

/**
 * @brief Manages animation loop with RAF
 */
class AnimationManager {
  constructor() {
    this.animations = new Map();
    this.isRunning = false;
    this.rafId = null;
  }
  
  /**
   * @brief Add an animation to the loop
   * @param {string} id - Unique identifier for the animation
   * @param {function} callback - Animation callback function
   */
  add(id, callback) {
    this.animations.set(id, callback);
    if (!this.isRunning) {
      this.start();
    }
  }
  
  /**
   * @brief Remove an animation from the loop
   * @param {string} id - Animation identifier to remove
   */
  remove(id) {
    this.animations.delete(id);
    if (this.animations.size === 0) {
      this.stop();
    }
  }
  
  start() {
    this.isRunning = true;
    this.lastTime = performance.now();
    this.loop();
  }
  
  stop() {
    this.isRunning = false;
    if (this.rafId) {
      cancelAnimationFrame(this.rafId);
      this.rafId = null;
    }
  }
  
  loop() {
    if (!this.isRunning) return;
    
    const currentTime = performance.now();
    const deltaTime = currentTime - this.lastTime;
    this.lastTime = currentTime;
    
    // Call all registered animations
    this.animations.forEach((callback) => {
      callback(deltaTime, currentTime);
    });
    
    this.rafId = requestAnimationFrame(() => this.loop());
  }
}

// Global animation manager instance
const animationManager = new AnimationManager();

/* ===================================================================
   TWEEN ANIMATION
   =================================================================== */

/**
 * @brief Animate a value over time with easing
 * @param {Object} options - Animation configuration
 * @example
 * tween({
 *   from: 0,
 *   to: 100,
 *   duration: 1000,
 *   easing: Easing.easeOutCubic,
 *   onUpdate: (value) => { element.style.left = value + 'px'; },
 *   onComplete: () => { console.log('done'); }
 * });
 */
function tween(options) {
  const {
    from = 0,
    to = 1,
    duration = 1000,
    easing = Easing.easeOutCubic,
    onUpdate = null,
    onComplete = null
  } = options;
  
  let startTime = null;
  const id = `tween_${Math.random()}`;
  
  const animate = (deltaTime, currentTime) => {
    if (!startTime) startTime = currentTime;
    
    const elapsed = currentTime - startTime;
    const progress = Math.min(elapsed / duration, 1);
    const easedProgress = easing(progress);
    const currentValue = from + (to - from) * easedProgress;
    
    if (onUpdate) {
      onUpdate(currentValue, progress);
    }
    
    if (progress >= 1) {
      animationManager.remove(id);
      if (onComplete) {
        onComplete();
      }
    }
  };
  
  animationManager.add(id, animate);
  
  // Return function to cancel the tween
  return () => animationManager.remove(id);
}

/* ===================================================================
   CANVAS UTILITIES
   =================================================================== */

/**
 * @brief Helper functions for canvas drawing
 */
const CanvasUtils = {
  /**
   * @brief Clear entire canvas
   */
  clear(ctx) {
    ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
  },
  
  /**
   * @brief Draw a circle
   */
  circle(ctx, x, y, radius, fillStyle) {
    ctx.beginPath();
    ctx.arc(x, y, radius, 0, Math.PI * 2);
    if (fillStyle) {
      ctx.fillStyle = fillStyle;
      ctx.fill();
    } else {
      ctx.stroke();
    }
  },
  
  /**
   * @brief Draw a line
   */
  line(ctx, x1, y1, x2, y2, strokeStyle, lineWidth = 1) {
    ctx.beginPath();
    ctx.moveTo(x1, y1);
    ctx.lineTo(x2, y2);
    if (strokeStyle) ctx.strokeStyle = strokeStyle;
    ctx.lineWidth = lineWidth;
    ctx.stroke();
  },
  
  /**
   * @brief Draw an arc
   */
  arc(ctx, x, y, radius, startAngle, endAngle, strokeStyle, lineWidth = 2) {
    ctx.beginPath();
    ctx.arc(x, y, radius, startAngle, endAngle);
    if (strokeStyle) ctx.strokeStyle = strokeStyle;
    ctx.lineWidth = lineWidth;
    ctx.stroke();
  },
  
  /**
   * @brief Draw text with glow effect
   */
  glowText(ctx, text, x, y, color, glowColor, fontSize = 14) {
    ctx.font = `${fontSize}px 'JetBrains Mono', monospace`;
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    
    // Glow
    ctx.shadowColor = glowColor;
    ctx.shadowBlur = 10;
    ctx.fillStyle = color;
    ctx.fillText(text, x, y);
    
    // Reset shadow
    ctx.shadowBlur = 0;
  },
  
  /**
   * @brief Setup high-DPI canvas
   */
  setupHiDPI(canvas) {
    const dpr = window.devicePixelRatio || 1;
    const rect = canvas.getBoundingClientRect();
    
    canvas.width = rect.width * dpr;
    canvas.height = rect.height * dpr;
    
    const ctx = canvas.getContext('2d');
    ctx.scale(dpr, dpr);
    
    canvas.style.width = rect.width + 'px';
    canvas.style.height = rect.height + 'px';
    
    return ctx;
  },
  
  /**
   * @brief Get gradient (linear)
   */
  linearGradient(ctx, x1, y1, x2, y2, colorStops) {
    const gradient = ctx.createLinearGradient(x1, y1, x2, y2);
    colorStops.forEach(stop => {
      gradient.addColorStop(stop.offset, stop.color);
    });
    return gradient;
  },
  
  /**
   * @brief Get gradient (radial)
   */
  radialGradient(ctx, x, y, r1, r2, colorStops) {
    const gradient = ctx.createRadialGradient(x, y, r1, x, y, r2);
    colorStops.forEach(stop => {
      gradient.addColorStop(stop.offset, stop.color);
    });
    return gradient;
  }
};

/* ===================================================================
   ORBITAL MOTION CALCULATOR
   =================================================================== */

/**
 * @brief Calculate orbital positions for animations
 */
const OrbitalMotion = {
  /**
   * @brief Calculate position on circular orbit
   * @param {number} centerX - Center X coordinate
   * @param {number} centerY - Center Y coordinate
   * @param {number} radius - Orbit radius
   * @param {number} angle - Angle in radians
   * @returns {Object} {x, y} position
   */
  circular(centerX, centerY, radius, angle) {
    return {
      x: centerX + radius * Math.cos(angle),
      y: centerY + radius * Math.sin(angle)
    };
  },
  
  /**
   * @brief Calculate position on elliptical orbit
   * @param {number} centerX - Center X coordinate
   * @param {number} centerY - Center Y coordinate
   * @param {number} radiusX - Horizontal radius
   * @param {number} radiusY - Vertical radius
   * @param {number} angle - Angle in radians
   * @returns {Object} {x, y} position
   */
  elliptical(centerX, centerY, radiusX, radiusY, angle) {
    return {
      x: centerX + radiusX * Math.cos(angle),
      y: centerY + radiusY * Math.sin(angle)
    };
  },
  
  /**
   * @brief Calculate Kepler's orbital velocity (faster at perihelion)
   * @param {number} eccentricity - Orbit eccentricity (0-1)
   * @param {number} angle - True anomaly in radians
   * @returns {number} Velocity multiplier
   */
  keplerVelocity(eccentricity, angle) {
    // Kepler's second law: speed varies with distance
    const r = (1 - eccentricity * eccentricity) / (1 + eccentricity * Math.cos(angle));
    return 1 / (r * r);
  }
};

/* ===================================================================
   COLOR UTILITIES
   =================================================================== */

/**
 * @brief Color manipulation utilities
 */
const ColorUtils = {
  /**
   * @brief Convert HSL to RGB color string
   */
  hslToRgb(h, s, l) {
    s /= 100;
    l /= 100;
    
    const c = (1 - Math.abs(2 * l - 1)) * s;
    const x = c * (1 - Math.abs((h / 60) % 2 - 1));
    const m = l - c / 2;
    let r = 0, g = 0, b = 0;
    
    if (0 <= h && h < 60) {
      r = c; g = x; b = 0;
    } else if (60 <= h && h < 120) {
      r = x; g = c; b = 0;
    } else if (120 <= h && h < 180) {
      r = 0; g = c; b = x;
    } else if (180 <= h && h < 240) {
      r = 0; g = x; b = c;
    } else if (240 <= h && h < 300) {
      r = x; g = 0; b = c;
    } else if (300 <= h && h < 360) {
      r = c; g = 0; b = x;
    }
    
    r = Math.round((r + m) * 255);
    g = Math.round((g + m) * 255);
    b = Math.round((b + m) * 255);
    
    return `rgb(${r}, ${g}, ${b})`;
  },
  
  /**
   * @brief Interpolate between two colors
   */
  lerp(color1, color2, t) {
    // Simple RGB interpolation (expects rgba strings)
    const c1 = color1.match(/\d+/g).map(Number);
    const c2 = color2.match(/\d+/g).map(Number);
    
    const r = Math.round(c1[0] + (c2[0] - c1[0]) * t);
    const g = Math.round(c1[1] + (c2[1] - c1[1]) * t);
    const b = Math.round(c1[2] + (c2[2] - c1[2]) * t);
    const a = c1[3] !== undefined ? (c1[3] + (c2[3] - c1[3]) * t) : 1;
    
    return `rgba(${r}, ${g}, ${b}, ${a})`;
  }
};

/* ===================================================================
   EXPORT FOR USE
   =================================================================== */

if (typeof window !== 'undefined') {
  window.Easing = Easing;
  window.animationManager = animationManager;
  window.tween = tween;
  window.CanvasUtils = CanvasUtils;
  window.OrbitalMotion = OrbitalMotion;
  window.ColorUtils = ColorUtils;
}
