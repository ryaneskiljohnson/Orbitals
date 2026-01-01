/**
 * @brief Wave particle for Tidal - flows with sine wave motion
 */
class WaveParticle {
  constructor(x, y, options = {}) {
    this.x = x;
    this.baseY = y;
    this.y = y;
    this.vx = options.vx || 1;
    this.amplitude = options.amplitude || 30;
    this.frequency = options.frequency || 0.02;
    this.phase = options.phase || 0;
    this.size = options.size || 2;
    this.color = options.color || '#3b82f6';
    this.alpha = 1;
    this.life = options.life || 5;
    this.decay = options.decay || 0.005;
    this.trail = [];
    this.trailLength = 30;
  }
  
  update(deltaTime, globalPhase) {
    const dt = deltaTime / 16.67;
    
    // Move horizontally
    this.x += this.vx * dt;
    
    // Oscillate vertically with wave
    this.y = this.baseY + Math.sin(this.x * this.frequency + globalPhase + this.phase) * this.amplitude;
    
    // Update trail
    this.trail.push({ x: this.x, y: this.y });
    if (this.trail.length > this.trailLength) {
      this.trail.shift();
    }
    
    // Life decay
    this.life -= this.decay * dt;
    this.alpha = Math.max(0, Math.min(1, this.life));
    
    return this.life > 0;
  }
  
  draw(ctx) {
    // Draw trail
    if (this.trail.length > 1) {
      ctx.save();
      this.trail.forEach((point, i) => {
        const trailAlpha = (i / this.trail.length) * this.alpha * 0.5;
        ctx.fillStyle = `rgba(59, 130, 246, ${trailAlpha})`;
        ctx.beginPath();
        ctx.arc(point.x, point.y, this.size * 0.7, 0, Math.PI * 2);
        ctx.fill();
      });
      ctx.restore();
    }
    
    // Draw particle
    ctx.save();
    ctx.globalAlpha = this.alpha;
    ctx.shadowColor = this.color;
    ctx.shadowBlur = this.size * 3;
    ctx.fillStyle = this.color;
    ctx.beginPath();
    ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
    ctx.fill();
    ctx.restore();
  }
}

document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('tidalCanvas');
  if (canvas) setupTidalVisualization(canvas);
  
  setupControls();
});

function setupTidalVisualization(canvas) {
  const initCanvas = () => {
    let ctx = CanvasUtils.setupHiDPI(canvas);
    
    const getCanvasSize = () => {
      const rect = canvas.getBoundingClientRect();
      return { width: rect.width, height: rect.height };
    };
    
    let canvasSize = getCanvasSize();
    let phase = 0;
    const particles = [];
    let strengthValue = 50;
    let phaseValue = 0;
    
    // Handle resize
    const resizeObserver = new ResizeObserver(() => {
      ctx = CanvasUtils.setupHiDPI(canvas);
      requestAnimationFrame(() => {
        canvasSize = getCanvasSize();
      });
    });
    resizeObserver.observe(canvas);
    
    // Expose control updaters
    window.updateStrengthValue = (value) => { strengthValue = value; };
    window.updatePhaseValue = (value) => { phaseValue = value; };
    
    animationManager.add('tidal-wave', (deltaTime, time) => {
      canvasSize = getCanvasSize();
      
      ctx.fillStyle = 'rgba(10, 10, 15, 0.12)';
      ctx.fillRect(0, 0, canvasSize.width, canvasSize.height);
      
      // Update phase
      phase += 0.015;
      const globalPhase = phase + (phaseValue / 360) * Math.PI * 2;
      
      // Draw wave lines (multiple layers)
      const amplitude = 30 + (strengthValue / 100) * 40; // 30-70px based on strength
      const centerY = canvasSize.height / 2;
      
      for (let layer = 0; layer < 3; layer++) {
        const layerAlpha = 0.4 - layer * 0.1;
        const layerOffset = layer * 20;
        
        ctx.save();
        ctx.strokeStyle = `rgba(59, 130, 246, ${layerAlpha})`;
        ctx.lineWidth = 2 - layer * 0.5;
        ctx.beginPath();
        
        for (let x = 0; x < canvasSize.width; x += 2) {
          const y = centerY + layerOffset + Math.sin(x * 0.015 + globalPhase + layer * 0.5) * amplitude;
          if (x === 0) ctx.moveTo(x, y);
          else ctx.lineTo(x, y);
        }
        ctx.stroke();
        ctx.restore();
      }
      
      // Draw moon (lunar influence)
      const moonX = canvasSize.width - 80;
      const moonY = 60 + Math.sin(time * 0.0005) * 10;
      const moonGradient = ctx.createRadialGradient(moonX, moonY, 0, moonX, moonY, 35);
      moonGradient.addColorStop(0, '#cbd5e1');
      moonGradient.addColorStop(0.6, 'rgba(203, 213, 225, 0.4)');
      moonGradient.addColorStop(1, 'rgba(203, 213, 225, 0)');
      ctx.fillStyle = moonGradient;
      ctx.beginPath();
      ctx.arc(moonX, moonY, 35, 0, Math.PI * 2);
      ctx.fill();
      
      // Moon core
      ctx.save();
      ctx.shadowColor = '#cbd5e1';
      ctx.shadowBlur = 20;
      ctx.fillStyle = '#cbd5e1';
      ctx.beginPath();
      ctx.arc(moonX, moonY, 20, 0, Math.PI * 2);
      ctx.fill();
      ctx.restore();
      
      // Emit wave particles
      const emissionChance = 0.1 + (strengthValue / 100) * 0.2;
      if (Math.random() < emissionChance) {
        particles.push(new WaveParticle(0, centerY, {
          vx: 1.5 + Math.random() * 1,
          amplitude: amplitude,
          frequency: 0.015,
          phase: Math.random() * Math.PI * 2,
          size: 2 + Math.random() * 2,
          color: ColorUtils.hslToRgb(200 + Math.random() * 20, 80, 50 + Math.random() * 20),
          life: 5 + Math.random() * 3,
          decay: 0.003
        }));
      }
      
      // Update and draw particles
      for (let i = particles.length - 1; i >= 0; i--) {
        if (!particles[i].update(deltaTime, globalPhase) || particles[i].x > canvasSize.width + 50) {
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
  new OrbitalsKnob(document.getElementById('phaseKnob'), {
    min: 0, max: 360, value: 0,
    onChange: (v) => {
      document.getElementById('phaseValue').textContent = Math.round(v) + '°';
      if (window.updatePhaseValue) window.updatePhaseValue(v);
    }
  });
  
  new OrbitalsKnob(document.getElementById('strengthKnob'), {
    min: 0, max: 100, value: 50,
    onChange: (v) => {
      document.getElementById('strengthValue').textContent = Math.round(v) + '%';
      if (window.updateStrengthValue) window.updateStrengthValue(v);
    }
  });
}
