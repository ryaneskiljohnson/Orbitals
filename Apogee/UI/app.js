/**
 * @brief Ascending particle for Apogee - rises along trajectory curves
 */
class AscendingParticle {
  constructor(x, y, options = {}) {
    this.x = x;
    this.y = y;
    this.startY = y;
    this.progress = 0; // 0-1 along trajectory
    this.speed = options.speed || 0.005;
    this.curve = options.curve || 'exp'; // linear, exp, s-curve, log
    this.size = options.size || 2;
    this.color = options.color || '#00d4ff';
    this.alpha = 1;
    this.trail = [];
    this.trailLength = options.trailLength || 40;
  }
  
  update(deltaTime) {
    const dt = deltaTime / 16.67;
    this.progress += this.speed * dt;
    
    // Calculate Y position based on curve type
    let curveProgress = this.progress;
    if (this.curve === 'exp') {
      curveProgress = Math.pow(this.progress, 2);
    } else if (this.curve === 's') {
      curveProgress = 0.5 - Math.cos(this.progress * Math.PI) / 2;
    } else if (this.curve === 'log') {
      curveProgress = Math.log(1 + this.progress * 9) / Math.log(10);
    }
    
    this.y = this.startY - curveProgress * this.startY;
    
    // Update trail
    this.trail.push({ x: this.x, y: this.y });
    if (this.trail.length > this.trailLength) {
      this.trail.shift();
    }
    
    // Fade as it reaches top
    this.alpha = 1 - curveProgress;
    
    return this.progress < 1;
  }
  
  draw(ctx) {
    // Draw trail
    if (this.trail.length > 1) {
      ctx.save();
      this.trail.forEach((point, i) => {
        const trailAlpha = (i / this.trail.length) * this.alpha * 0.4;
        ctx.fillStyle = `rgba(0, 212, 255, ${trailAlpha})`;
        const trailSize = this.size * (i / this.trail.length);
        ctx.beginPath();
        ctx.arc(point.x, point.y, trailSize, 0, Math.PI * 2);
        ctx.fill();
      });
      ctx.restore();
    }
    
    // Draw particle
    ctx.save();
    ctx.globalAlpha = this.alpha;
    ctx.shadowColor = this.color;
    ctx.shadowBlur = this.size * 4;
    ctx.fillStyle = this.color;
    ctx.beginPath();
    ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
    ctx.fill();
    ctx.restore();
  }
}

document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('trajectoryCanvas');
  if (canvas) setupApogeeVisualization(canvas);
  
  setupControls();
});

function setupApogeeVisualization(canvas) {
  const initCanvas = () => {
    let ctx = CanvasUtils.setupHiDPI(canvas);
    
    const getCanvasSize = () => {
      const rect = canvas.getBoundingClientRect();
      return { width: rect.width, height: rect.height };
    };
    
    let canvasSize = getCanvasSize();
    const particles = [];
    let currentCurve = 'exp';
    let liftValue = 50;
    let momentumValue = 50;
    let ceilingValue = 127;
    
    // Handle resize
    const resizeObserver = new ResizeObserver(() => {
      ctx = CanvasUtils.setupHiDPI(canvas);
      requestAnimationFrame(() => {
        canvasSize = getCanvasSize();
      });
    });
    resizeObserver.observe(canvas);
    
    // Expose control updaters
    window.updateLiftValue = (value) => {
      liftValue = Math.max(0, Math.min(100, value));
    };
    
    window.updateMomentumValue = (value) => {
      momentumValue = Math.max(0, Math.min(100, value));
    };
    
    window.updateCeilingValue = (value) => {
      ceilingValue = Math.max(1, Math.min(127, value));
    };
    
    window.updateCurveType = (curve) => {
      currentCurve = curve;
    };
    
    animationManager.add('apogee-trajectory', (deltaTime, time) => {
      canvasSize = getCanvasSize();
      
      ctx.fillStyle = 'rgba(10, 10, 15, 0.15)';
      ctx.fillRect(0, 0, canvasSize.width, canvasSize.height);
      
      // Draw trajectory curve - opacity affected by ceiling
      const ceilingOpacity = 0.3 + (ceilingValue / 127) * 0.4; // 0.3 to 0.7 opacity
      const gradient = ctx.createLinearGradient(0, canvasSize.height, 0, 0);
      gradient.addColorStop(0, `rgba(0, 148, 255, ${ceilingOpacity * 0.5})`);
      gradient.addColorStop(0.5, `rgba(0, 212, 255, ${ceilingOpacity * 0.7})`);
      gradient.addColorStop(1, `rgba(255, 255, 255, ${ceilingOpacity})`);
      ctx.strokeStyle = gradient;
      ctx.lineWidth = 2;
      ctx.setLineDash([5, 10]);
      ctx.beginPath();
      
      for (let i = 0; i <= 100; i++) {
        const progress = i / 100;
        let curveProgress = progress;
        
        if (currentCurve === 'exp') {
          curveProgress = Math.pow(progress, 2);
        } else if (currentCurve === 's') {
          curveProgress = 0.5 - Math.cos(progress * Math.PI) / 2;
        } else if (currentCurve === 'log') {
          curveProgress = Math.log(1 + progress * 9) / Math.log(10);
        }
        
        const x = progress * canvasSize.width;
        const y = canvasSize.height - curveProgress * canvasSize.height * 0.85;
        
        if (i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
      ctx.stroke();
      
      // Emit particles based on lift value
      const emissionChance = 0.05 + (liftValue / 100) * 0.25;
      if (Math.random() < emissionChance) {
        const x = Math.random() * canvasSize.width;
        // Speed is affected by both lift (primary) and momentum (secondary)
        const baseSpeed = 0.002;
        const liftSpeed = (liftValue / 100) * 0.012; // Lift has strong effect on speed
        const momentumSpeed = (momentumValue / 100) * 0.004; // Momentum has smaller effect
        const speed = baseSpeed + liftSpeed + momentumSpeed;
        // Particle size is affected by momentum - higher momentum = larger particles
        const baseSize = 1.5;
        const momentumSize = (momentumValue / 100) * 3.5; // Momentum affects size from 1.5 to 5
        const randomVariation = Math.random() * 1.5;
        const size = baseSize + momentumSize + randomVariation;
        
        // Particle brightness/intensity affected by ceiling - higher ceiling = brighter particles
        const ceilingBrightness = 50 + (ceilingValue / 127) * 50; // 50 to 100 brightness
        const ceilingSaturation = 80 + (ceilingValue / 127) * 20; // 80 to 100 saturation
        const hue = 180 + Math.random() * 40;
        const color = ColorUtils.hslToRgb(hue, ceilingSaturation, ceilingBrightness);
        
        particles.push(new AscendingParticle(x, canvasSize.height, {
          speed: speed,
          curve: currentCurve,
          size: size,
          color: color,
          trailLength: 30 + Math.floor((momentumValue / 100) * 30)
        }));
      }
      
      // Update speed and size of existing particles based on current values
      const currentBaseSpeed = 0.002;
      const currentLiftSpeed = (liftValue / 100) * 0.012;
      const currentMomentumSpeed = (momentumValue / 100) * 0.004;
      const targetSpeed = currentBaseSpeed + currentLiftSpeed + currentMomentumSpeed;
      
      const currentBaseSize = 1.5;
      const currentMomentumSize = (momentumValue / 100) * 3.5;
      const targetSize = currentBaseSize + currentMomentumSize;
      
      particles.forEach(particle => {
        // Smoothly adjust speed of existing particles
        particle.speed += (targetSpeed - particle.speed) * 0.1;
        // Smoothly adjust size of existing particles
        particle.size += (targetSize - particle.size) * 0.1;
      });
      
      // Update and draw particles
      for (let i = particles.length - 1; i >= 0; i--) {
        if (!particles[i].update(deltaTime)) {
          particles.splice(i, 1);
        } else {
          particles[i].draw(ctx);
        }
      }
    });
  };
  
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initCanvas);
  } else {
    requestAnimationFrame(() => setTimeout(initCanvas, 0));
  }
}

function setupControls() {
  const liftSlider = new OrbitalsSlider(document.getElementById('liftSlider'), {
    min: 0, max: 100, value: 50,
    orientation: 'vertical',
    onChange: (v) => {
      document.getElementById('liftValue').textContent = Math.round(v) + '%';
      if (window.updateLiftValue) window.updateLiftValue(v);
    }
  });
  // Ensure initial value is set for fill
  liftSlider.updatePosition();
  
  new OrbitalsSlider(document.getElementById('ceilingSlider'), {
    min: 1, max: 127, value: 127,
    orientation: 'horizontal',
    onChange: (v) => {
      document.getElementById('ceilingValue').textContent = Math.round(v);
      if (window.updateCeilingValue) window.updateCeilingValue(v);
    }
  });
  
  new OrbitalsKnob(document.getElementById('momentumKnob'), {
    min: 0, max: 100, value: 50,
    onChange: (v) => {
      document.getElementById('momentumValue').textContent = Math.round(v) + '%';
      if (window.updateMomentumValue) window.updateMomentumValue(v);
    }
  });
  
  // Curve selector
  document.querySelectorAll('.curve-selector button').forEach(btn => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.curve-selector button').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      const curve = btn.dataset.curve;
      if (window.updateCurveType) window.updateCurveType(curve);
    });
  });
  
  // Setup bypass toggle
  if (window.setupBypassToggle) {
    window.setupBypassToggle(sendToJUCE);
  }
}
