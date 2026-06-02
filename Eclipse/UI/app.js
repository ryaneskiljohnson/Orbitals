document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('eclipseCanvas');
  if (canvas) setupEclipseVisualization(canvas);
  
  setupControls();
  setupBackgroundImageMonitoring();
});

/**
 * @brief Monitor background image changes to detect scaling, aspect ratio, or position shifts
 * @note Creates test elements that mirror ::before and ::after pseudo-elements to detect changes
 */
function setupBackgroundImageMonitoring() {
  const container = document.querySelector('.plugin-container');
  if (!container) return;
  
  // Create test elements that mirror the pseudo-elements
  const testBefore = document.createElement('div');
  const testAfter = document.createElement('div');
  
  // Copy styles from computed styles (we'll set them manually to match pseudo-elements)
  testBefore.style.cssText = `
    position: absolute;
    top: 0;
    left: 0;
    width: 1200px;
    height: 750px;
    background: var(--deep-space);
    background-image: var(--plugin-background-image, none);
    background-size: cover;
    background-position: center;
    background-repeat: no-repeat;
    opacity: 1;
    pointer-events: none;
    z-index: -1000;
    visibility: hidden;
  `;
  
  testAfter.style.cssText = `
    position: absolute;
    top: 0;
    left: 0;
    width: 1200px;
    height: 750px;
    background: var(--deep-space);
    background-image: var(--plugin-background-image, none);
    background-size: cover;
    background-position: center;
    background-repeat: no-repeat;
    opacity: 0.6;
    pointer-events: none;
    z-index: -1000;
    visibility: hidden;
  `;
  
  container.appendChild(testBefore);
  container.appendChild(testAfter);
  
  let lastBeforeSize = null;
  let lastBeforePosition = null;
  let lastAfterSize = null;
  let lastAfterPosition = null;
  
  function checkChanges() {
    const beforeStyle = window.getComputedStyle(testBefore);
    const afterStyle = window.getComputedStyle(testAfter);
    
    const beforeSize = beforeStyle.backgroundSize;
    const beforePosition = beforeStyle.backgroundPosition;
    const afterSize = afterStyle.backgroundSize;
    const afterPosition = afterStyle.backgroundPosition;
    
    // Check for changes
    if (lastBeforeSize && (beforeSize !== lastBeforeSize || beforePosition !== lastBeforePosition)) {
      console.log('🔴 ::before BACKGROUND CHANGED:', {
        size: { from: lastBeforeSize, to: beforeSize },
        position: { from: lastBeforePosition, to: beforePosition },
        timestamp: new Date().toISOString()
      });
    }
    
    if (lastAfterSize && (afterSize !== lastAfterSize || afterPosition !== lastAfterPosition)) {
      console.log('🔴 ::after BACKGROUND CHANGED:', {
        size: { from: lastAfterSize, to: afterSize },
        position: { from: lastAfterPosition, to: afterPosition },
        timestamp: new Date().toISOString()
      });
    }
    
    // Log initial values and differences
    if (!lastBeforeSize) {
      console.log('📊 Initial ::before background:', { size: beforeSize, position: beforePosition });
      console.log('📊 Initial ::after background:', { size: afterSize, position: afterPosition });
      
      if (beforeSize !== afterSize || beforePosition !== afterPosition) {
        console.warn('⚠️ INITIAL MISMATCH between ::before and ::after:', {
          before: { size: beforeSize, position: beforePosition },
          after: { size: afterSize, position: afterPosition }
        });
      }
    } else {
      // Check if they differ
      if (beforeSize !== afterSize || beforePosition !== afterPosition) {
        console.warn('⚠️ CURRENT MISMATCH between ::before and ::after:', {
          before: { size: beforeSize, position: beforePosition },
          after: { size: afterSize, position: afterPosition },
          timestamp: new Date().toISOString()
        });
      }
    }
    
    lastBeforeSize = beforeSize;
    lastBeforePosition = beforePosition;
    lastAfterSize = afterSize;
    lastAfterPosition = afterPosition;
  }
  
  // Check immediately and then periodically
  setTimeout(checkChanges, 100); // Initial check after styles are applied
  setTimeout(checkChanges, 1600); // Check right before fade starts (1.5s + 100ms)
  setTimeout(checkChanges, 2000); // Check during fade (1.5s + 500ms)
  setTimeout(checkChanges, 4000); // Check after fade completes (1.5s + 2s + 500ms)
  
  // Also monitor continuously during the fade period
  let checkInterval = null;
  setTimeout(() => {
    checkInterval = setInterval(checkChanges, 100); // Check every 100ms during fade
    setTimeout(() => {
      if (checkInterval) clearInterval(checkInterval);
    }, 4000); // Stop after fade completes
  }, 1500); // Start monitoring right before fade
}

function setupEclipseVisualization(canvas) {
  const initCanvas = () => {
    let ctx = CanvasUtils.setupHiDPI(canvas);
    
    const getCanvasSize = () => {
    const rect = canvas.getBoundingClientRect();
      return { width: rect.width, height: rect.height };
    };
    
    // Get center based on Threshold slider or center control
    const getCenter = () => {
      const size = getCanvasSize();
      return { x: size.width / 2, y: size.height / 2 - 20 }; // Move animation up 20px
    };
    
    let canvasSize = getCanvasSize();
    let center = getCenter();
    let centerX = center.x;
    let centerY = center.y;
    
    // Control values
    let eclipsePhase = 0.5; // Start at partial eclipse
    let thresholdValue = 64; // Totality
    let shadowZoneMin = 40; // Shadow zone range min
    let shadowZoneMax = 80; // Shadow zone range max
    let umbraValue = 100; // Full shadow intensity (0-100%)
    let penumbraValue = 50; // Partial shadow intensity (0-100%)
    let eclipseMode = 'reveal'; // 'reveal' or 'hide'
    let time = 0;
    
    // Velocity particles for visualization
    const velocityParticles = [];
    
    // Particle class for velocity visualization
    class VelocityParticle {
      constructor() {
        this.reset();
      }
      
      reset() {
        this.x = Math.random() * (canvasSize.width || 1200);
        this.velocity = Math.random() * 127; // MIDI velocity 0-127
        this.y = centerY + (Math.random() - 0.5) * 150;
        this.speed = 0.5 + Math.random() * 1.5;
        this.size = 2 + Math.random() * 3;
        this.alpha = 0.8;
      }
      
      update(deltaTime) {
        this.x -= this.speed * deltaTime * 0.05;
        
        if (this.x < -20) {
          this.reset();
          this.x = canvasSize.width + 20;
        }
        
        // Calculate if particle is in shadow based on velocity and threshold
        const inFullShadow = (eclipseMode === 'hide') ? 
          (this.velocity > thresholdValue) : 
          (this.velocity < thresholdValue);
        
        const inPenumbra = Math.abs(this.velocity - thresholdValue) < (shadowZoneMax - shadowZoneMin) / 2;
        
        // Fade based on shadow state
        if (inFullShadow) {
          this.alpha = 1 - (umbraValue / 100) * 0.9; // Umbra darkens particles
        } else if (inPenumbra) {
          this.alpha = 1 - (penumbraValue / 100) * 0.5; // Penumbra partially darkens
        } else {
          this.alpha = 0.8;
        }
      }
      
      draw(ctx) {
        // Color based on velocity
        const hue = (this.velocity / 127) * 60; // 0-60 (gold to yellow range)
        const brightness = 50 + (this.velocity / 127) * 30;
        
        ctx.save();
        ctx.globalAlpha = this.alpha;
        
        // Glow
        const gradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, this.size * 2);
        gradient.addColorStop(0, `hsl(${hue + 40}, 100%, ${brightness}%)`);
        gradient.addColorStop(0.5, `hsl(${hue + 40}, 100%, ${brightness}%, 0.5)`);
        gradient.addColorStop(1, `hsl(${hue + 40}, 100%, ${brightness}%, 0)`);
        
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.size * 2, 0, Math.PI * 2);
        ctx.fill();
        
        // Core
        ctx.fillStyle = `hsl(${hue + 40}, 100%, ${brightness + 20}%)`;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
        ctx.fill();
        
        ctx.restore();
      }
    }
    
    // Initialize particles
    for (let i = 0; i < 40; i++) {
      velocityParticles.push(new VelocityParticle());
    }
    
    // Handle resize
    const resizeObserver = new ResizeObserver(() => {
      ctx = CanvasUtils.setupHiDPI(canvas);
      requestAnimationFrame(() => {
        canvasSize = getCanvasSize();
        center = getCenter();
        centerX = center.x;
        centerY = center.y;
      });
    });
    resizeObserver.observe(canvas);
    
    // Expose control updaters
    window.updateThresholdValue = (value) => {
      thresholdValue = value;
      eclipsePhase = value / 127; // 0-127 maps to 0-1 eclipse phase
    };
    window.updateShadowZone = (min, max) => {
      shadowZoneMin = min;
      shadowZoneMax = max;
    };
    window.updateUmbra = (value) => {
      umbraValue = value;
    };
    window.updatePenumbra = (value) => {
      penumbraValue = value;
    };
    window.updateMode = (mode) => {
      eclipseMode = mode;
    };
    
    animationManager.add('eclipse-animation', (deltaTime) => {
      time += deltaTime * 0.001;
      canvasSize = getCanvasSize();
      center = getCenter();
      centerX = center.x;
      centerY = center.y;
      
      // Clear with trail effect
      ctx.fillStyle = 'rgba(10, 10, 15, 0.3)';
      ctx.fillRect(0, 0, canvasSize.width, canvasSize.height);
      
      const sunRadius = 80;
      const moonRadius = 75;
      
      // Update and draw velocity particles
      velocityParticles.forEach(particle => {
        particle.update(deltaTime);
        particle.draw(ctx);
      });
      
      // Draw shadow zone visualization (fade zone rings)
      const shadowZoneRange = shadowZoneMax - shadowZoneMin;
      if (shadowZoneRange > 5) {
        ctx.save();
        ctx.strokeStyle = `rgba(120, 113, 108, ${0.2 + (penumbraValue / 100) * 0.3})`;
        ctx.lineWidth = 2;
        ctx.setLineDash([4, 8]);
        const zoneRadius = 100 + shadowZoneRange * 0.5;
        ctx.beginPath();
        ctx.arc(centerX, centerY, zoneRadius, 0, Math.PI * 2);
        ctx.stroke();
        ctx.restore();
      }
      
      // Draw corona rays (intensity affected by penumbra)
      const rayCount = 24;
      const coronaIntensity = 1 - (eclipsePhase * (umbraValue / 100));
      for (let i = 0; i < rayCount; i++) {
        const angle = (i / rayCount) * Math.PI * 2 + time * 0.5;
        const rayLength = (40 + Math.sin(time * 2 + i) * 15) * (0.5 + coronaIntensity * 0.5);
        const x1 = centerX + Math.cos(angle) * sunRadius;
        const y1 = centerY + Math.sin(angle) * sunRadius;
        const x2 = centerX + Math.cos(angle) * (sunRadius + rayLength);
        const y2 = centerY + Math.sin(angle) * (sunRadius + rayLength);
        
        const gradient = ctx.createLinearGradient(x1, y1, x2, y2);
        gradient.addColorStop(0, `rgba(251, 191, 36, ${0.6 * coronaIntensity})`);
        gradient.addColorStop(1, 'rgba(251, 191, 36, 0)');
        
        ctx.save();
        ctx.strokeStyle = gradient;
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(x1, y1);
        ctx.lineTo(x2, y2);
        ctx.stroke();
        ctx.restore();
      }
      
      // Draw sun (brightness affected by umbra during eclipse)
      const sunBrightness = 1 - (eclipsePhase * (umbraValue / 100) * 0.5);
      const sunGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, sunRadius);
      sunGradient.addColorStop(0, `rgba(255, 255, 255, ${sunBrightness})`);
      sunGradient.addColorStop(0.3, `rgba(251, 191, 36, ${sunBrightness})`);
      sunGradient.addColorStop(0.7, `rgba(245, 158, 11, ${sunBrightness * 0.8})`);
      sunGradient.addColorStop(1, 'rgba(245, 158, 11, 0)');
      
      ctx.save();
      ctx.shadowColor = '#fbbf24';
      ctx.shadowBlur = 40 * sunBrightness;
      ctx.fillStyle = sunGradient;
      ctx.beginPath();
      ctx.arc(centerX, centerY, sunRadius, 0, Math.PI * 2);
      ctx.fill();
      ctx.restore();
      
      // Eclipse direction depends on mode
      // REVEAL mode: eclipse from left (hiding low velocities, revealing high)
      // HIDE mode: eclipse from right (hiding high velocities, revealing low)
      const baseOffset = eclipseMode === 'reveal' ? 
        (eclipsePhase - 0.5) * sunRadius * 2.5 :  // Left to right
        ((1 - eclipsePhase) - 0.5) * sunRadius * 2.5; // Right to left (inverted)
      
      const moonX = centerX + baseOffset;
      
      // Moon shadow (opacity affected by umbra)
      ctx.save();
      ctx.globalAlpha = umbraValue / 100;
      ctx.globalCompositeOperation = 'destination-out';
      ctx.fillStyle = '#000000';
      ctx.beginPath();
      ctx.arc(moonX, centerY, moonRadius, 0, Math.PI * 2);
      ctx.fill();
      ctx.restore();
      
      // Draw penumbra effect (soft shadow around moon - direction depends on mode)
      if (penumbraValue > 10) {
        ctx.save();
        ctx.globalAlpha = (penumbraValue / 100) * 0.4;
        ctx.globalCompositeOperation = 'source-over';
        const penumbraGradient = ctx.createRadialGradient(moonX, centerY, moonRadius, moonX, centerY, moonRadius + 30);
        penumbraGradient.addColorStop(0, 'rgba(120, 113, 108, 0.8)');
        penumbraGradient.addColorStop(1, 'rgba(120, 113, 108, 0)');
        ctx.fillStyle = penumbraGradient;
        ctx.beginPath();
        ctx.arc(moonX, centerY, moonRadius + 30, 0, Math.PI * 2);
        ctx.fill();
        ctx.restore();
      }
      
      // Moon outline (corona edge - brighter with less umbra)
      ctx.save();
      ctx.strokeStyle = `rgba(251, 191, 36, ${0.8 * (1 - (umbraValue / 100) * 0.5)})`;
      ctx.lineWidth = 2;
      ctx.shadowColor = '#fbbf24';
      ctx.shadowBlur = 15 * (1 - (umbraValue / 100) * 0.3);
      ctx.beginPath();
      ctx.arc(moonX, centerY, moonRadius, 0, Math.PI * 2);
      ctx.stroke();
      ctx.restore();
      
      // Draw threshold line (vertical indicator) with mode label
      const thresholdPercent = thresholdValue / 127;
      const lineX = centerX + (thresholdPercent - 0.5) * 300;
      ctx.save();
      ctx.strokeStyle = `rgba(251, 191, 36, 0.4)`;
      ctx.lineWidth = 2;
      ctx.setLineDash([4, 4]);
      ctx.beginPath();
      ctx.moveTo(lineX, centerY - 120);
      ctx.lineTo(lineX, centerY + 120);
      ctx.stroke();
      
      // Label the masked side
      ctx.setLineDash([]);
      ctx.font = 'bold 12px Inter';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillStyle = 'rgba(0, 0, 0, 0.8)';
      
      if (eclipseMode === 'reveal') {
        // Left side = MASKED (hidden)
        ctx.fillRect(lineX - 80, centerY - 130, 60, 20);
        ctx.fillStyle = '#fbbf24';
        ctx.shadowColor = '#fbbf24';
        ctx.shadowBlur = 8;
        ctx.fillText('MASKED', lineX - 50, centerY - 120);
        
        // Right side = REVEALED (shown)
        ctx.fillStyle = 'rgba(0, 0, 0, 0.8)';
        ctx.fillRect(lineX + 20, centerY - 130, 70, 20);
        ctx.fillStyle = '#00d4ff';
        ctx.shadowColor = '#00d4ff';
        ctx.fillText('REVEALED', lineX + 55, centerY - 120);
      } else {
        // Left side = REVEALED (shown)
        ctx.fillRect(lineX - 80, centerY - 130, 70, 20);
        ctx.fillStyle = '#00d4ff';
        ctx.shadowColor = '#00d4ff';
        ctx.shadowBlur = 8;
        ctx.fillText('REVEALED', lineX - 45, centerY - 120);
        
        // Right side = MASKED (hidden)
        ctx.fillStyle = 'rgba(0, 0, 0, 0.8)';
        ctx.fillRect(lineX + 20, centerY - 130, 60, 20);
        ctx.fillStyle = '#fbbf24';
        ctx.shadowColor = '#fbbf24';
        ctx.fillText('MASKED', lineX + 50, centerY - 120);
      }
      
      ctx.restore();
    });
  };
  
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initCanvas);
  } else {
    requestAnimationFrame(() => setTimeout(initCanvas, 0));
  }
}

function setupControls() {
  // Shadow Zone Range Slider
  const shadowZoneSlider = document.getElementById('shadowZoneSlider');
  if (shadowZoneSlider) {
    new OrbitalsRangeSlider(shadowZoneSlider, {
      min: 0, max: 127, valueMin: 40, valueMax: 80,
      onChange: (min, max) => {
        document.getElementById('shadowZoneValue').textContent = `${Math.round(min)} - ${Math.round(max)}`;
        if (window.updateShadowZone) window.updateShadowZone(min, max);
        sendToJUCE('shadowZone', { min, max });
      }
    });
  }
  
  // Umbra Control (Knob)
  const umbraControl = document.getElementById('umbraControl');
  if (umbraControl) {
    new OrbitalsKnob(umbraControl, {
      min: 0, max: 100, value: 100,
      onChange: (v) => {
        document.getElementById('umbraValue').textContent = Math.round(v) + '%';
        if (window.updateUmbra) window.updateUmbra(v);
        sendToJUCE('umbra', v);
      }
    });
  }
  
  // Totality Knob (Main Threshold)
  const totalityKnob = document.getElementById('totalityKnob');
  if (totalityKnob) {
    const knob = new OrbitalsKnob(totalityKnob, {
      min: 0, max: 127, value: 64,
      onChange: (v) => {
        const percent = Math.round((v / 127) * 100);
        document.getElementById('totalityValue').textContent = percent + '%';
        if (window.updateThresholdValue) window.updateThresholdValue(v);
        sendToJUCE('totality', v);
      }
    });
    
    // Mark as initialized after a brief delay to prevent initial snap
    requestAnimationFrame(() => {
      totalityKnob.classList.add('initialized');
    });
  }
  
  // Penumbra Control (Knob)
  const penumbraControl = document.getElementById('penumbraControl');
  if (penumbraControl) {
    new OrbitalsKnob(penumbraControl, {
      min: 0, max: 100, value: 50,
      onChange: (v) => {
        document.getElementById('penumbraValue').textContent = Math.round(v) + '%';
        if (window.updatePenumbra) window.updatePenumbra(v);
        sendToJUCE('penumbra', v);
      }
    });
  }
  
  // Mode Toggle Buttons (now in footer)
  const modeToggle = document.querySelector('.eclipse-mode-toggle');
  if (modeToggle) {
    const modeButtons = modeToggle.querySelectorAll('button');
    modeButtons.forEach(button => {
      button.addEventListener('click', () => {
        modeButtons.forEach(b => b.classList.remove('active'));
        button.classList.add('active');
        const mode = button.dataset.mode;
        if (window.updateMode) window.updateMode(mode);
        sendToJUCE('mode', mode);
  });
});
  }
}/**
 * @brief Sends parameter changes to C++ via JUCE native bridge (macOS + Windows).
 * @param {string} param Parameter ID.
 * @param {*} value Raw parameter value.
 */
function sendToJUCE(param, value) {
    if (typeof window.postMessageToJUCE !== 'function') return;
    window.postMessageToJUCE({ type: 'parameterChange', parameter: param, value: value });
}
