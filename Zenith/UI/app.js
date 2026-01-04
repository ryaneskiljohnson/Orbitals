/**
 * @fileoverview Zenith - Velocity Expander Application Logic
 * @module Zenith App
 */

/* ===================================================================
   INITIALIZATION
   =================================================================== */

document.addEventListener('DOMContentLoaded', () => {
  setupControls();
  
  // Setup canvas
  const canvas = document.getElementById('zenithCanvas');
  if (canvas) {
    setupZenithVisualization(canvas);
  }
});

/* ===================================================================
   ZENITH STORM VISUALIZATION
   =================================================================== */

class CloudParticle {
  constructor(x, y, options = {}) {
    this.x = x;
    this.y = y;
    this.baseX = x;
    this.baseY = y;
    this.vx = (Math.random() - 0.5) * 0.5;
    this.vy = (Math.random() - 0.5) * 0.3;
    this.size = options.size || (20 + Math.random() * 40);
    this.alpha = options.alpha || (0.5 + Math.random() * 0.5);
    this.life = Math.random() * Math.PI * 2;
    this.speed = 0.01 + Math.random() * 0.02;
  }
  
  update(params, centerX, centerY) {
    this.life += this.speed;
    
    // Expansion affects outward spread
    const dx = this.baseX - centerX;
    const dy = this.baseY - centerY;
    const angle = Math.atan2(dy, dx);
    const expansionForce = (params.expansion / 100) * 1.5;
    const targetX = this.baseX + Math.cos(angle) * expansionForce * 50;
    const targetY = this.baseY + Math.sin(angle) * expansionForce * 50;
    
    this.x += (targetX - this.x) * 0.03;
    this.y += (targetY - this.y) * 0.03;
    
    // Curve affects turbulence
    const turbulence = (params.curve / 100) * 3;
    this.vx += (Math.random() - 0.5) * turbulence * 0.1;
    this.vy += (Math.random() - 0.5) * turbulence * 0.1;
    
    this.x += this.vx;
    this.y += this.vy;
    
    this.vx *= 0.95;
    this.vy *= 0.95;
    
    // Ceiling affects brightness - keep sizes large and diffuse
    const ceilingFactor = (params.ceiling / 127);
    const energyPulse = Math.sin(this.life) * 0.15;
    this.currentSize = this.size * (0.9 + ceilingFactor * 0.3) * (1 + energyPulse);
    this.currentAlpha = this.alpha * (0.6 + ceilingFactor * 0.4);
  }
  
  draw(ctx, params) {
    // Ultra-diffuse cloud rendering - very soft, wispy
    // Presunset colors: warm yellows/oranges with greys
    const ceilingFactor = params.ceiling / 127;
    
    // Warm yellow/orange tones mixed with grey
    const warmYellow = 220 + (ceilingFactor * 35); // 220-255
    const warmOrange = 180 + (ceilingFactor * 50);  // 180-230
    const greyBase = 140 + (ceilingFactor * 40);    // 140-180
    
    // Very large, very soft gradient - extends far beyond particle center
    const gradientSize = this.currentSize * 2.5;
    const gradient = ctx.createRadialGradient(
      this.x, this.y, 0,
      this.x, this.y, gradientSize
    );
    
    // Presunset color stops: warm yellow center fading to grey
    gradient.addColorStop(0, `rgba(${warmYellow}, ${warmOrange}, ${greyBase}, ${this.currentAlpha * 0.3})`);
    gradient.addColorStop(0.2, `rgba(${warmYellow - 15}, ${warmOrange - 10}, ${greyBase - 5}, ${this.currentAlpha * 0.2})`);
    gradient.addColorStop(0.4, `rgba(${warmYellow - 40}, ${warmOrange - 30}, ${greyBase - 10}, ${this.currentAlpha * 0.1})`);
    gradient.addColorStop(0.6, `rgba(${greyBase - 20}, ${greyBase - 15}, ${greyBase - 5}, ${this.currentAlpha * 0.05})`);
    gradient.addColorStop(1, `rgba(100, 100, 110, 0)`);
    
    ctx.fillStyle = gradient;
    ctx.beginPath();
    ctx.arc(this.x, this.y, gradientSize, 0, Math.PI * 2);
    ctx.fill();
  }
}

class LightningBolt {
  constructor(startX, startY, endX, endY, intensity) {
    this.segments = [];
    this.alpha = 1;
    this.life = 0.8;
    
    let x = startX;
    let y = startY;
    const steps = 8 + Math.floor(intensity * 12);
    const stepX = (endX - startX) / steps;
    const stepY = (endY - startY) / steps;
    
    for (let i = 0; i < steps; i++) {
      this.segments.push({ x, y });
      x += stepX + (Math.random() - 0.5) * 40 * intensity;
      y += stepY + (Math.random() - 0.5) * 20 * intensity;
    }
    this.segments.push({ x: endX, y: endY });
  }
  
  update() {
    this.life -= 0.15;
    this.alpha = Math.max(0, this.life);
    return this.life > 0;
  }
  
  draw(ctx) {
    if (this.alpha <= 0) return;
    
    // Warm presunset lightning - golden/yellow tones
    ctx.strokeStyle = `rgba(255, 240, 200, ${this.alpha})`;
    ctx.lineWidth = 3 + this.alpha * 3;
    ctx.shadowBlur = 25;
    ctx.shadowColor = `rgba(240, 200, 150, ${this.alpha * 0.8})`;
    ctx.beginPath();
    ctx.moveTo(this.segments[0].x, this.segments[0].y);
    for (let i = 1; i < this.segments.length; i++) {
      ctx.lineTo(this.segments[i].x, this.segments[i].y);
    }
    ctx.stroke();
    ctx.shadowBlur = 0;
  }
}

function setupZenithVisualization(canvas) {
  const initCanvas = () => {
    let ctx = CanvasUtils.setupHiDPI(canvas);
    const container = canvas.parentElement;
    
    const getCanvasSize = () => {
      const rect = canvas.getBoundingClientRect();
      return { width: rect.width, height: rect.height };
    };
    
    let canvasSize = getCanvasSize();
    let centerX = canvasSize.width / 2;
    let centerY = canvasSize.height / 2;
    let time = 0;
    
    // Control values
    let expansionValue = 50;
    let thresholdValue = 64;
    let ceilingValue = 127;
    let curveValue = 50;
    
    window.updateExpansionValue = (value) => { expansionValue = value; };
    window.updateThresholdValue = (value) => { thresholdValue = value; };
    window.updateCeilingValue = (value) => { ceilingValue = value; };
    window.updateCurveValue = (value) => { curveValue = value; };
    
    // Create clouds - fewer, larger, more diffuse particles
    const clouds = [];
    for (let i = 0; i < 50; i++) {
      const angle = (i / 50) * Math.PI * 2;
      const radius = 40 + Math.random() * 120;
      clouds.push(new CloudParticle(
        centerX + Math.cos(angle) * radius,
        centerY + Math.sin(angle) * radius * 0.5,
        { size: 80 + Math.random() * 100, alpha: 0.1 + Math.random() * 0.2 }
      ));
    }
    
    const lightningBolts = [];
    let lightningTimer = 0;
    
    // Handle resize
    const resizeObserver = new ResizeObserver(() => {
      ctx = CanvasUtils.setupHiDPI(canvas);
      requestAnimationFrame(() => {
        canvasSize = getCanvasSize();
        centerX = canvasSize.width / 2;
        centerY = canvasSize.height / 2;
      });
    });
    resizeObserver.observe(canvas);
    if (container) {
      resizeObserver.observe(container);
    }
    
    // Animation loop - EXACTLY like Perihelion
    animationManager.add('zenith-storm', (deltaTime) => {
      try {
        time += deltaTime * 0.001;
        
        canvasSize = getCanvasSize();
        centerX = canvasSize.width / 2;
        centerY = canvasSize.height / 2;
        
        // Clear canvas
        ctx.clearRect(0, 0, canvasSize.width, canvasSize.height);
      
      const params = { expansion: expansionValue, threshold: thresholdValue, ceiling: ceilingValue, curve: curveValue };
      
      // Atmospheric zones - scaled smaller
      const thresholdRadius = 60 + (thresholdValue / 127) * 80;
      const ceilingRadius = 100 + (ceilingValue / 127) * 100;
      
      // Very diffuse base layer - presunset atmospheric haze (warm grey-yellow)
      const cloudBaseGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, ceilingRadius * 1.5);
      cloudBaseGradient.addColorStop(0, `rgba(220, 200, 160, ${0.05 + (expansionValue / 100) * 0.08})`);
      cloudBaseGradient.addColorStop(0.4, `rgba(180, 170, 150, ${0.03 + (curveValue / 100) * 0.05})`);
      cloudBaseGradient.addColorStop(0.7, `rgba(140, 130, 120, ${0.02})`);
      cloudBaseGradient.addColorStop(1, 'rgba(100, 100, 110, 0)');
      ctx.fillStyle = cloudBaseGradient;
      ctx.beginPath();
      ctx.arc(centerX, centerY, ceilingRadius * 1.5, 0, Math.PI * 2);
      ctx.fill();
      
      // Update clouds
      clouds.forEach((cloud) => {
        const dx = cloud.x - centerX;
        const dy = cloud.y - centerY;
        const dist = Math.sqrt(dx * dx + dy * dy);
        
        if (dist > 400) {
          const angle = Math.random() * Math.PI * 2;
          const radius = 50 + Math.random() * 150;
          cloud.x = centerX + Math.cos(angle) * radius;
          cloud.y = centerY + Math.sin(angle) * radius * 0.6;
          cloud.baseX = cloud.x;
          cloud.baseY = cloud.y;
        }
        
        cloud.update(params, centerX, centerY);
        cloud.draw(ctx, params);
      });
      
      // Lightning - more frequent and dramatic
      lightningTimer++;
      const lightningChance = (curveValue / 100) * (ceilingValue / 127) * 0.03;
      if (lightningTimer > 15 && Math.random() < lightningChance) {
        // Multiple fork starting points for more dramatic effect
        const forkCount = 1 + Math.floor((curveValue / 100) * 2);
        for (let f = 0; f < forkCount; f++) {
          const startX = centerX + (Math.random() - 0.5) * ceilingRadius * 0.8;
          const startY = centerY - ceilingRadius * 0.9;
          const endX = centerX + (Math.random() - 0.5) * thresholdRadius * 0.8;
          const endY = centerY + thresholdRadius * 0.3;
          lightningBolts.push(new LightningBolt(startX, startY, endX, endY, 0.6 + (curveValue / 100) * 0.4));
        }
        lightningTimer = 0;
      }
      
      for (let i = lightningBolts.length - 1; i >= 0; i--) {
        if (!lightningBolts[i].update()) {
          lightningBolts.splice(i, 1);
        } else {
          lightningBolts[i].draw(ctx);
        }
      }
      
      // White energy ball at center - pulsing and glowing
      const energyBallRadius = 25 + (expansionValue / 100) * 35;
      const pulse = Math.sin(time * 3) * 0.15 + 1; // Pulsing effect
      const currentRadius = energyBallRadius * pulse;
      
      // Outer glow - bright white energy
      const outerGlowRadius = currentRadius * 2.5;
      const outerGlow = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, outerGlowRadius);
      outerGlow.addColorStop(0, `rgba(255, 255, 255, ${0.4 * (ceilingValue / 127)})`);
      outerGlow.addColorStop(0.3, `rgba(255, 255, 255, ${0.2 * (expansionValue / 100)})`);
      outerGlow.addColorStop(0.6, `rgba(240, 240, 255, ${0.1})`);
      outerGlow.addColorStop(1, 'rgba(255, 255, 255, 0)');
      ctx.fillStyle = outerGlow;
      ctx.beginPath();
      ctx.arc(centerX, centerY, outerGlowRadius, 0, Math.PI * 2);
      ctx.fill();
      
      // Main energy ball - bright white core
      const energyGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, currentRadius);
      energyGradient.addColorStop(0, `rgba(255, 255, 255, ${0.95 * (ceilingValue / 127)})`);
      energyGradient.addColorStop(0.4, `rgba(255, 255, 255, ${0.8 * (expansionValue / 100)})`);
      energyGradient.addColorStop(0.7, `rgba(240, 240, 255, ${0.5})`);
      energyGradient.addColorStop(1, 'rgba(255, 255, 255, 0)');
      ctx.fillStyle = energyGradient;
      ctx.beginPath();
      ctx.arc(centerX, centerY, currentRadius, 0, Math.PI * 2);
      ctx.fill();
      
      // Inner bright core
      const coreRadius = currentRadius * 0.4;
      ctx.fillStyle = `rgba(255, 255, 255, ${0.9 + Math.sin(time * 5) * 0.1})`;
      ctx.shadowBlur = 30;
      ctx.shadowColor = 'rgba(255, 255, 255, 0.9)';
      ctx.beginPath();
      ctx.arc(centerX, centerY, coreRadius, 0, Math.PI * 2);
      ctx.fill();
      ctx.shadowBlur = 0;
      
      // Rotating energy rays - white energy streams
      const rayCount = Math.floor(8 + (curveValue / 100) * 12);
      for (let i = 0; i < rayCount; i++) {
        const angle = (time * 2.5 + (i / rayCount) * Math.PI * 2);
        const innerR = currentRadius * 0.6;
        const outerR = thresholdRadius * 0.85;
        const rayAlpha = 0.3 + (expansionValue / 100) * 0.4;
        ctx.strokeStyle = `rgba(255, 255, 255, ${rayAlpha})`;
        ctx.lineWidth = 2.5;
        ctx.shadowBlur = 8;
        ctx.shadowColor = 'rgba(255, 255, 255, 0.6)';
        ctx.beginPath();
        ctx.moveTo(centerX + Math.cos(angle) * innerR, centerY + Math.sin(angle) * innerR);
        ctx.lineTo(centerX + Math.cos(angle) * outerR, centerY + Math.sin(angle) * outerR);
        ctx.stroke();
        ctx.shadowBlur = 0;
      }
      } catch (e) {
        // Silently handle errors to prevent crashes
        console.error('Animation error:', e);
      }
    });
  };
  
  if (window.CanvasUtils && window.animationManager) {
    initCanvas();
  }
}

/* ===================================================================
   CONTROLS SETUP
   =================================================================== */

function setupControls() {
  const expansionSlider = document.getElementById('expansionSlider');
  if (expansionSlider) {
    new OrbitalsSlider(expansionSlider, {
      min: 0, max: 100, value: 50,
      orientation: 'vertical',
      onChange: (v) => {
        document.getElementById('expansionValue').textContent = Math.round(v) + '%';
        expansionSlider.style.setProperty('--slider-value', v / 100);
        if (window.updateExpansionValue) window.updateExpansionValue(v);
        sendParameterToJUCE('expansion', v);
      }
    });
    expansionSlider.style.setProperty('--slider-value', 0.5);
  }
  
  const thresholdSlider = document.getElementById('thresholdSlider');
  if (thresholdSlider) {
    new OrbitalsSlider(thresholdSlider, {
      min: 0, max: 127, value: 64,
      orientation: 'horizontal',
      onChange: (v) => {
        document.getElementById('thresholdValue').textContent = Math.round(v);
        thresholdSlider.style.setProperty('--slider-value', v / 127);
        if (window.updateThresholdValue) window.updateThresholdValue(v);
        sendParameterToJUCE('threshold', v);
      }
    });
    thresholdSlider.style.setProperty('--slider-value', 64 / 127);
  }
  
  const curveSlider = document.getElementById('curveSlider');
  if (curveSlider) {
    new OrbitalsSlider(curveSlider, {
      min: 0, max: 100, value: 50,
      orientation: 'vertical',
      onChange: (v) => {
        document.getElementById('curveValue').textContent = Math.round(v) + '%';
        curveSlider.style.setProperty('--slider-value', v / 100);
        if (window.updateCurveValue) window.updateCurveValue(v);
        sendParameterToJUCE('curve', v);
      }
    });
    curveSlider.style.setProperty('--slider-value', 0.5);
  }
  
  const ceilingSlider = document.getElementById('ceilingSlider');
  if (ceilingSlider) {
    new OrbitalsSlider(ceilingSlider, {
      min: 0, max: 127, value: 127,
      orientation: 'horizontal',
      onChange: (v) => {
        document.getElementById('ceilingValue').textContent = Math.round(v);
        ceilingSlider.style.setProperty('--slider-value', v / 127);
        if (window.updateCeilingValue) window.updateCeilingValue(v);
        sendParameterToJUCE('ceiling', v);
      }
    });
    ceilingSlider.style.setProperty('--slider-value', 1.0);
  }
  
  if (window.setupBypassToggle) {
    window.setupBypassToggle(sendParameterToJUCE);
  }
}

function sendParameterToJUCE(param, value) {
  if (window.chrome?.webview) {
    window.chrome.webview.postMessage({ type: 'parameterChange', parameter: param, value });
  }
}

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
  }
};
