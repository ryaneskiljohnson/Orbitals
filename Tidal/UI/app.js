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
    let amplitudeValue = 50;
    let phaseValue = 0;
    let rateValue = '1/4'; // Tempo rate
    let shapeValue = 'sine'; // Wave shape
    
    // Handle resize
    const resizeObserver = new ResizeObserver(() => {
      ctx = CanvasUtils.setupHiDPI(canvas);
      requestAnimationFrame(() => {
        canvasSize = getCanvasSize();
      });
    });
    resizeObserver.observe(canvas);
    
    // Expose control updaters
    window.updateAmplitude = (value) => { amplitudeValue = value; };
    window.updatePhaseValue = (value) => { phaseValue = value; };
    window.updateRate = (value) => { rateValue = value; };
    window.updateShape = (value) => { shapeValue = value; };
    
    animationManager.add('tidal-wave', (deltaTime, time) => {
      canvasSize = getCanvasSize();
      
      ctx.fillStyle = 'rgba(10, 10, 15, 0.12)';
      ctx.fillRect(0, 0, canvasSize.width, canvasSize.height);
      
      // Calculate rate multiplier (faster rates = faster animation)
      let rateMultiplier = 1;
      if (rateValue === '1/4') rateMultiplier = 0.5;  // Slowest
      else if (rateValue === '1/8') rateMultiplier = 1;
      else if (rateValue === '1/16') rateMultiplier = 2;
      else if (rateValue === '1/32') rateMultiplier = 4;  // Fastest
      
      // Update phase based on rate
      phase += 0.015 * rateMultiplier;
      const globalPhase = phase + (phaseValue / 360) * Math.PI * 2;
      
      // Draw wave lines (multiple layers)
      const amplitude = 30 + (amplitudeValue / 100) * 40; // 30-70px based on amplitude
      const centerY = canvasSize.height / 2;
      
      // Wave function based on shape
      const getWaveY = (x, phaseOffset) => {
        const normalizedX = (x / canvasSize.width) * Math.PI * 4 + globalPhase + phaseOffset;
        let y;
        
        if (shapeValue === 'sine') {
          y = Math.sin(normalizedX) * amplitude;
        } else if (shapeValue === 'triangle') {
          const trianglePhase = (normalizedX / (Math.PI * 2)) % 1;
          y = (trianglePhase < 0.5 ? trianglePhase * 4 - 1 : 3 - trianglePhase * 4) * amplitude;
        } else if (shapeValue === 'saw') {
          const sawPhase = (normalizedX / (Math.PI * 2)) % 1;
          y = (sawPhase * 2 - 1) * amplitude;
        } else if (shapeValue === 'square') {
          const squarePhase = (normalizedX / (Math.PI * 2)) % 1;
          y = (squarePhase < 0.5 ? -1 : 1) * amplitude;
        } else {
          y = Math.sin(normalizedX) * amplitude; // Default to sine
        }
        
        return centerY + y;
      };
      
      for (let layer = 0; layer < 3; layer++) {
        const layerAlpha = 0.4 - layer * 0.1;
        const layerOffset = layer * 20;
        
        ctx.save();
        ctx.strokeStyle = `rgba(59, 130, 246, ${layerAlpha})`;
        ctx.lineWidth = 2 - layer * 0.5;
        ctx.beginPath();
        
        for (let x = 0; x < canvasSize.width; x += 2) {
          const y = getWaveY(x, layer * 0.5) + layerOffset;
          if (x === 0) ctx.moveTo(x, y);
          else ctx.lineTo(x, y);
        }
        ctx.stroke();
        ctx.restore();
      }
      
      // Emit wave particles - affected by amplitude and rate
      const emissionChance = (0.1 + (amplitudeValue / 100) * 0.2) * rateMultiplier;
      if (Math.random() < emissionChance) {
        particles.push(new WaveParticle(0, centerY, {
          vx: (1.5 + Math.random() * 1) * rateMultiplier, // Faster particles for faster rates
          amplitude: amplitude,
          frequency: 0.015 * rateMultiplier, // Frequency affected by rate
          phase: Math.random() * Math.PI * 2,
          size: 2 + Math.random() * 2,
          color: ColorUtils.hslToRgb(200 + Math.random() * 20, 80, 50 + Math.random() * 20),
          life: 5 + Math.random() * 3,
          decay: 0.003
        }));
      }
      
      // Update and draw particles - particles follow the wave shape
      for (let i = particles.length - 1; i >= 0; i--) {
        const particle = particles[i];
        if (particle.update) {
          // Update particle
          const stillAlive = particle.update(deltaTime, globalPhase);
          if (!stillAlive || particle.x > canvasSize.width + 50) {
            particles.splice(i, 1);
          } else {
            // Adjust particle baseY to follow wave shape
            const waveY = getWaveY(particle.x, particle.phase || 0);
            particle.baseY = waveY;
            particle.draw(ctx);
          }
        } else {
          particles.splice(i, 1);
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
  let currentShape = 'sine';
  let currentAmplitude = 50;
  let currentPhase = 0;
  
  // Setup wave shape canvas
  const waveShapeCanvas = document.getElementById('waveShapeCanvas');
  if (waveShapeCanvas) {
    let waveCtx = CanvasUtils.setupHiDPI(waveShapeCanvas);
    
    const drawWaveShape = () => {
      const rect = waveShapeCanvas.getBoundingClientRect();
      waveCtx.clearRect(0, 0, rect.width, rect.height);
      
      const centerY = rect.height / 2;
      const amplitude = (currentAmplitude / 100) * (rect.height * 0.35);
      const phase = (currentPhase / 360) * Math.PI * 2;
      
      // Draw grid lines
      waveCtx.strokeStyle = 'rgba(59, 130, 246, 0.15)';
      waveCtx.lineWidth = 1;
      waveCtx.beginPath();
      waveCtx.moveTo(0, centerY);
      waveCtx.lineTo(rect.width, centerY);
      waveCtx.stroke();
      
      // Draw wave
      waveCtx.strokeStyle = '#60a5fa';
      waveCtx.lineWidth = 3;
      waveCtx.shadowColor = '#60a5fa';
      waveCtx.shadowBlur = 10;
      waveCtx.beginPath();
      
      for (let x = 0; x < rect.width; x++) {
        const progress = x / rect.width;
        let y;
        
        if (currentShape === 'sine') {
          y = centerY + Math.sin(progress * Math.PI * 4 + phase) * amplitude;
        } else if (currentShape === 'triangle') {
          const trianglePhase = (progress * 4 + phase / (Math.PI * 2)) % 1;
          y = centerY + (trianglePhase < 0.5 ? trianglePhase * 4 - 1 : 3 - trianglePhase * 4) * amplitude;
        } else if (currentShape === 'saw') {
          const sawPhase = (progress * 4 + phase / (Math.PI * 2)) % 1;
          y = centerY + (sawPhase * 2 - 1) * amplitude;
        } else if (currentShape === 'square') {
          const squarePhase = (progress * 4 + phase / (Math.PI * 2)) % 1;
          y = centerY + (squarePhase < 0.5 ? -1 : 1) * amplitude;
        }
        
        if (x === 0) waveCtx.moveTo(x, y);
        else waveCtx.lineTo(x, y);
      }
      waveCtx.stroke();
      
      // Draw fill
      waveCtx.fillStyle = 'rgba(96, 165, 250, 0.1)';
      waveCtx.lineTo(rect.width, centerY);
      waveCtx.lineTo(0, centerY);
      waveCtx.fill();
    };
    
    // Animate wave shape
    const animateWave = () => {
      drawWaveShape();
      requestAnimationFrame(animateWave);
    };
    animateWave();
  }
  
  // Amplitude Knob
  const amplitudeKnob = document.getElementById('amplitudeKnob');
  if (amplitudeKnob) {
    const knob = new OrbitalsKnob(amplitudeKnob, {
      min: 0, max: 100, value: 50,
      onChange: (v) => {
        currentAmplitude = v;
        document.getElementById('amplitudeValue').textContent = Math.round(v) + '%';
        if (window.updateAmplitude) window.updateAmplitude(v);
        sendToJUCE('amplitude', v);
      }
    });
    // Mark as initialized to prevent initial snap animation
    amplitudeKnob.classList.add('initialized');
  }
  
  // Phase Slider
  const phaseSlider = document.getElementById('phaseSlider');
  if (phaseSlider) {
    const slider = new OrbitalsSlider(phaseSlider, {
      min: 0, max: 360, value: 0,
      orientation: 'horizontal',
      onChange: (v) => {
        currentPhase = v;
        document.getElementById('phaseValue').textContent = Math.round(v) + '°';
        if (window.updatePhaseValue) window.updatePhaseValue(v);
        sendToJUCE('phase', v);
      }
    });
    // Ensure initial value is set for fill
    slider.updatePosition();
  }
  
  // Rate/Tempo Selector
  const tempoButtons = document.querySelectorAll('.tempo-selector[data-help-title="CYCLE LENGTH"] button');
  tempoButtons.forEach(button => {
    button.addEventListener('click', () => {
      tempoButtons.forEach(b => b.classList.remove('active'));
      button.classList.add('active');
      const tempo = button.dataset.tempo;
      if (window.updateRate) window.updateRate(tempo);
      sendToJUCE('rate', tempo);
    });
  });
  
  // Shape Selector
  const shapeButtons = document.querySelectorAll('.tempo-selector[data-help-title="FLOW"] button');
  shapeButtons.forEach(button => {
    button.addEventListener('click', () => {
      shapeButtons.forEach(b => b.classList.remove('active'));
      button.classList.add('active');
      currentShape = button.dataset.shape;
      if (window.updateShape) window.updateShape(currentShape);
      sendToJUCE('shape', currentShape);
    });
  });
  
  // Setup bypass toggle
  if (window.setupBypassToggle) {
    window.setupBypassToggle(sendToJUCE);
  }
}

/**
 * @brief Sends parameter changes to JUCE
 * @param {string} param - Parameter name
 * @param {*} value - Parameter value
 */
/**
 * @brief Sends parameter changes to C++ via JUCE native bridge (macOS + Windows).
 * @param {string} param Parameter ID.
 * @param {*} value Raw parameter value.
 */
function sendToJUCE(param, value) {
    if (typeof window.postMessageToJUCE !== 'function') return;
    window.postMessageToJUCE({ type: 'parameterChange', parameter: param, value: value });
}
