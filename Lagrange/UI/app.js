document.addEventListener('DOMContentLoaded', () => {
  console.log('Lagrange initializing...');
  
  const canvas = document.getElementById('lagrangeCanvas');
  if (canvas) setupLagrangeVisualization(canvas);
  
  setupControls();
});

function setupLagrangeVisualization(canvas) {
  const initCanvas = () => {
    let ctx = CanvasUtils.setupHiDPI(canvas);
    
    // Get canvas dimensions
    const getCanvasSize = () => {
      const rect = canvas.getBoundingClientRect();
      return {
        width: rect.width,
        height: rect.height
      };
    };
    
    // Get center of canvas (animation is on right side now)
    const getCenter = () => {
      const size = getCanvasSize();
      // Center the animation in the right panel
      return {
        x: size.width / 2,
        y: size.height / 2
      };
    };
    
    let canvasSize = getCanvasSize();
    let center = getCenter();
    let centerX = center.x;
    let centerY = center.y;
    
    // Control values
    let stabilityValue = 50;
    let massValue = 50;
    let driftMin = -25;
    let driftMax = 25;
    let chaosX = 50;
    let chaosY = 50;
    
    // Timing particles for visualization
    const timingParticles = [];
    
    // Particle class for orbital motion around Lagrange points
    class TimingParticle {
      constructor(lagrangePoint, index) {
        this.lagrangePoint = lagrangePoint;
        this.angle = (Math.PI * 2 * index) / 12 + Math.random() * 0.5;
        this.baseOrbitRadius = 45 + Math.random() * 30; // Scaled up from 30+20
        this.speed = 0.0005 + Math.random() * 0.001;
        this.phase = Math.random() * Math.PI * 2;
        this.size = 2 + Math.random() * 2;
        this.hue = 180 + Math.random() * 40;
        this.driftX = 0;
        this.driftY = 0;
      }
      
      update(deltaTime, stability, mass, drift, chaos) {
        // Stability affects orbit tightness (higher = tighter, more circular)
        const stabilityFactor = 0.3 + (stability / 100) * 0.7;
        const orbitRadius = this.baseOrbitRadius * (1.5 - stabilityFactor * 0.8);
        
        // Mass affects orbital speed (higher mass = faster orbit)
        const massFactor = 0.5 + (mass / 100) * 1.5;
        
        // Drift adds controlled randomness (based on drift range)
        const driftAmount = ((drift.max - drift.min) / 100) * 0.3;
        this.driftX += (Math.random() - 0.5) * driftAmount;
        this.driftY += (Math.random() - 0.5) * driftAmount;
        
        // Apply stability to dampen drift (higher stability = less drift)
        const driftDamping = 0.95 + (stability / 100) * 0.04;
        this.driftX *= driftDamping;
        this.driftY *= driftDamping;
        
        // Chaos adds extra jitter and affects particle properties
        const chaosAmount = (chaos.x / 100) * 4; // Increased from 2
        const chaosJitter = (chaos.y / 100) * 4; // Increased from 2
        this.driftX += (Math.random() - 0.5) * chaosAmount;
        this.driftY += (Math.random() - 0.5) * chaosJitter;
        
        // Chaos also affects orbital speed (X-axis) and orbit radius (Y-axis)
        const chaosSpeedMod = 1 + ((chaos.x / 100) - 0.5) * 0.3; // ±15% speed variation
        const chaosRadiusMod = 1 + ((chaos.y / 100) - 0.5) * 0.2; // ±10% radius variation
        this.angle += this.speed * massFactor * chaosSpeedMod * deltaTime;
        const effectiveOrbitRadius = orbitRadius * chaosRadiusMod;
        
        // Limit drift - scaled up
        const maxDrift = 22 + (driftAmount * 30); // Scaled up from 15+20
        const driftMag = Math.sqrt(this.driftX * this.driftX + this.driftY * this.driftY);
        if (driftMag > maxDrift) {
          this.driftX = (this.driftX / driftMag) * maxDrift;
          this.driftY = (this.driftY / driftMag) * maxDrift;
        }
        
        // Calculate position (using effective orbit radius that includes chaos)
        this.x = this.lagrangePoint.x + Math.cos(this.angle) * effectiveOrbitRadius + this.driftX;
        this.y = this.lagrangePoint.y + Math.sin(this.angle) * effectiveOrbitRadius + this.driftY;
      }
      
      draw(ctx, stability, chaos) {
        // Particle brightness based on stability (higher = brighter)
        const brightness = 40 + (stability / 100) * 40;
        
        // Chaos affects particle size and saturation (more chaos = larger, more saturated)
        const chaosSizeMod = 1 + ((chaos.x + chaos.y) / 200) * 0.5; // Up to 50% larger
        const chaosSaturation = 80 + ((chaos.x + chaos.y) / 200) * 20; // 80-100% saturation
        const effectiveSize = this.size * chaosSizeMod;
        
        // Glow
        const gradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, effectiveSize * 3);
        gradient.addColorStop(0, `hsla(${this.hue}, ${chaosSaturation}%, ${brightness}%, 0.8)`);
        gradient.addColorStop(0.5, `hsla(${this.hue}, ${chaosSaturation}%, ${brightness}%, 0.3)`);
        gradient.addColorStop(1, `hsla(${this.hue}, ${chaosSaturation}%, ${brightness}%, 0)`);
        
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y, effectiveSize * 3, 0, Math.PI * 2);
        ctx.fill();
        
        // Core
        ctx.fillStyle = `hsla(${this.hue}, ${chaosSaturation}%, ${brightness + 20}%, 0.9)`;
        ctx.beginPath();
        ctx.arc(this.x, this.y, effectiveSize, 0, Math.PI * 2);
        ctx.fill();
      }
    }
    
    // Initialize particles for each Lagrange point
    const initParticles = (lagrangePoints) => {
      timingParticles.length = 0;
      lagrangePoints.forEach(point => {
        for (let i = 0; i < 12; i++) {
          timingParticles.push(new TimingParticle(point, i));
        }
      });
    };
    
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
    window.updateStabilityValue = (value) => {
      stabilityValue = Math.max(0, Math.min(100, value));
    };
    
    window.updateMassValue = (value) => {
      massValue = Math.max(0, Math.min(100, value));
    };
    
    window.updateDriftRange = (min, max) => {
      driftMin = min;
      driftMax = max;
    };
    
    window.updateMicroChaos = (x, y) => {
      chaosX = x;
      chaosY = y;
    };
  
    let particlesInitialized = false;
    
    animationManager.add('lagrange-points', (deltaTime, time) => {
      // Update center
      canvasSize = getCanvasSize();
      center = getCenter();
      centerX = center.x;
      centerY = center.y;
      
      // Clear with trail effect
      ctx.fillStyle = 'rgba(10, 10, 15, 0.15)';
      ctx.fillRect(0, 0, canvasSize.width, canvasSize.height);
      
      // Calculate Lagrange points positions (spacing affected by mass) - scaled larger
      const spacing = 120 + (massValue / 100) * 60; // 120-180px based on mass (50% larger)
      const points = [
        { x: centerX - spacing, y: centerY, label: 'L1' },
        { x: centerX + spacing, y: centerY, label: 'L2' },
        { x: centerX, y: centerY - spacing, label: 'L3' },
        { x: centerX - spacing * 0.7, y: centerY + spacing * 0.7, label: 'L4' },
        { x: centerX + spacing * 0.7, y: centerY + spacing * 0.7, label: 'L5' }
      ];
      
      // Initialize particles on first frame
      if (!particlesInitialized) {
        initParticles(points);
        particlesInitialized = true;
      }
      
      // Update particle Lagrange point references (in case spacing changes)
      timingParticles.forEach((particle, i) => {
        const pointIndex = Math.floor(i / 12);
        particle.lagrangePoint = points[pointIndex];
      });
      
      // Draw gravitational field lines (affected by mass)
      const fieldStrength = massValue / 100;
      ctx.save();
      ctx.strokeStyle = `rgba(0, 148, 255, ${0.05 + fieldStrength * 0.1})`;
      ctx.lineWidth = 1;
      ctx.setLineDash([2, 6]);
      
      // Draw radial field lines from center
      for (let angle = 0; angle < Math.PI * 2; angle += Math.PI / 8) {
        const radius = 300 * (1 + fieldStrength * 0.5); // Scaled up from 200
        ctx.beginPath();
        ctx.moveTo(centerX, centerY);
        ctx.lineTo(
          centerX + Math.cos(angle) * radius,
          centerY + Math.sin(angle) * radius
        );
        ctx.stroke();
      }
      ctx.restore();
      
      // Draw connection lines between Lagrange points (equilibrium field)
      ctx.save();
      ctx.strokeStyle = `rgba(0, 212, 255, ${0.1 + (stabilityValue / 100) * 0.15})`;
      ctx.lineWidth = 1 + (stabilityValue / 100);
      ctx.setLineDash([4, 8]);
      points.forEach((point, i) => {
        points.forEach((otherPoint, j) => {
          if (i < j) {
            ctx.beginPath();
            ctx.moveTo(point.x, point.y);
            ctx.lineTo(otherPoint.x, otherPoint.y);
            ctx.stroke();
          }
        });
      });
      ctx.restore();
      
      // Update and draw particles
      const driftRange = { min: driftMin, max: driftMax };
      const chaos = { x: chaosX, y: chaosY };
      
      timingParticles.forEach(particle => {
        particle.update(deltaTime, stabilityValue, massValue, driftRange, chaos);
        particle.draw(ctx, stabilityValue, chaos);
      });
      
      // Draw 5 Lagrange points with pulsing
      points.forEach((point, i) => {
        const phase = time * 0.001 + i * 0.4;
        const stabilityEffect = stabilityValue / 100;
        const massEffect = massValue / 100;
        
        // Less pulsing with more stability, larger with more mass - scaled up
        const baseRadius = 15 + massEffect * 6; // Scaled up from 10+4
        const radius = baseRadius + Math.sin(phase * 2) * (6 - stabilityEffect * 4); // Scaled up from 4-3
        
        // Outer glow (affected by mass)
        const glowRadius = radius * 3 * (1 + massEffect * 0.5);
        const gradient = ctx.createRadialGradient(point.x, point.y, 0, point.x, point.y, glowRadius);
        gradient.addColorStop(0, `rgba(0, 212, 255, ${0.5 + massEffect * 0.3})`);
        gradient.addColorStop(0.5, `rgba(0, 212, 255, ${0.2 + massEffect * 0.1})`);
        gradient.addColorStop(1, 'rgba(0, 212, 255, 0)');
        
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(point.x, point.y, glowRadius, 0, Math.PI * 2);
        ctx.fill();
        
        // Core point (brighter with stability)
        ctx.save();
        ctx.shadowColor = '#00d4ff';
        ctx.shadowBlur = 15 + stabilityEffect * 10;
        ctx.fillStyle = `rgba(0, 212, 255, ${0.8 + stabilityEffect * 0.2})`;
        ctx.beginPath();
        ctx.arc(point.x, point.y, radius * 0.6, 0, Math.PI * 2);
        ctx.fill();
        ctx.restore();
        
        // Label with better contrast
        ctx.save();
        ctx.font = 'bold 12px Inter'; // Slightly larger font
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillStyle = 'rgba(0, 0, 0, 0.8)';
        ctx.fillRect(point.x - 18, point.y + radius + 12, 36, 18); // Larger label box
        ctx.fillStyle = '#00d4ff';
        ctx.shadowColor = '#00d4ff';
        ctx.shadowBlur = 8;
        ctx.fillText(point.label, point.x, point.y + radius + 21); // Adjusted position
        ctx.restore();
      });
      
      // Draw drift range indicator (visual feedback for drift slider)
      const driftPercent = (driftMax - driftMin) / 100;
      ctx.save();
      ctx.strokeStyle = `rgba(255, 148, 0, ${0.2 + driftPercent * 0.3})`;
      ctx.lineWidth = 2;
      ctx.setLineDash([8, 4]);
      const driftRadius = 90 + driftPercent * 60; // Scaled up from 60+40
      ctx.beginPath();
      ctx.arc(centerX, centerY, driftRadius, 0, Math.PI * 2);
      ctx.stroke();
      ctx.restore();
      
      // Draw chaos energy waves (visual feedback for micro chaos)
      const chaosIntensity = (chaosX + chaosY) / 200; // 0-1
      if (chaosIntensity > 0.1) {
        ctx.save();
        ctx.strokeStyle = `rgba(236, 72, 153, ${0.1 + chaosIntensity * 0.3})`; // Pink/magenta for chaos
        ctx.lineWidth = 1 + chaosIntensity * 2;
        ctx.setLineDash([4, 8]);
        
        // Draw chaotic energy rings
        for (let i = 0; i < 3; i++) {
          const wavePhase = (time * 0.0005 + i * 0.3) % 1;
          const waveRadius = 100 + wavePhase * 150 + chaosIntensity * 50;
          const waveOpacity = (1 - wavePhase) * (0.2 + chaosIntensity * 0.3);
          
          ctx.strokeStyle = `rgba(236, 72, 153, ${waveOpacity})`;
          ctx.beginPath();
          ctx.arc(centerX, centerY, waveRadius, 0, Math.PI * 2);
          ctx.stroke();
        }
        ctx.restore();
      }
    });
  };
  
  // Initialize when ready
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initCanvas);
  } else {
    requestAnimationFrame(() => setTimeout(initCanvas, 0));
  }
}

function setupControls() {
  const stabilitySlider = document.getElementById('stabilitySlider');
  new OrbitalsSlider(stabilitySlider, {
    min: 0, max: 100, value: 50,
    orientation: 'vertical',
    onChange: (v) => {
      document.getElementById('stabilityValue').textContent = Math.round(v) + '%';
      // Ensure CSS variable is set for fill
      const percent = (v - 0) / (100 - 0);
      stabilitySlider.style.setProperty('--slider-value', percent);
      if (window.updateStabilityValue) window.updateStabilityValue(v);
      sendToJUCE('stability', v);
    }
  });
  // Set initial CSS variable
  stabilitySlider.style.setProperty('--slider-value', 0.5);
  
  const massSlider = document.getElementById('massSlider');
  new OrbitalsSlider(massSlider, {
    min: 0, max: 100, value: 50,
    orientation: 'vertical',
    onChange: (v) => {
      document.getElementById('massValue').textContent = Math.round(v) + '%';
      // Ensure CSS variable is set for fill
      const percent = (v - 0) / (100 - 0);
      massSlider.style.setProperty('--slider-value', percent);
      if (window.updateMassValue) window.updateMassValue(v);
      sendToJUCE('mass', v);
    }
  });
  // Set initial CSS variable
  massSlider.style.setProperty('--slider-value', 0.5);
  
  const driftSlider = document.getElementById('driftSlider');
  new OrbitalsRangeSlider(driftSlider, {
    min: -50, max: 50, valueMin: -25, valueMax: 25,
    onChange: (min, max) => {
      document.getElementById('driftValue').textContent = `${Math.round(min)}ms to ${Math.round(max)}ms`;
      
      // Update CSS variables for fill visualization
      const percentMin = (min - (-50)) / (50 - (-50));
      const percentMax = (max - (-50)) / (50 - (-50));
      driftSlider.style.setProperty('--range-min', `${percentMin * 100}%`);
      driftSlider.style.setProperty('--range-max', `${percentMax * 100}%`);
      
      if (window.updateDriftRange) window.updateDriftRange(min, max);
      sendToJUCE('drift', { min, max });
    }
  });
  
  // Set initial CSS variables
  driftSlider.style.setProperty('--range-min', '25%');
  driftSlider.style.setProperty('--range-max', '75%');
  
  new OrbitalsXYPad(document.getElementById('chaosPad'), {
    minX: 0, maxX: 100, minY: 0, maxY: 100,
    valueX: 50, valueY: 50,
    onChange: (x, y) => {
      document.getElementById('chaosValue').textContent = `X:${Math.round(x)} Y:${Math.round(y)}`;
      if (window.updateMicroChaos) window.updateMicroChaos(x, y);
      sendToJUCE('microChaos', { x, y });
    }
  });
}

function sendToJUCE(param, value) {
  if (window.chrome?.webview) {
    window.chrome.webview.postMessage({ type: 'parameterChange', parameter: param, value });
  }
}
