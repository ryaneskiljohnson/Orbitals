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
    
    // Timing particles for visualization
    const timingParticles = [];
    
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
  
    animationManager.add('lagrange-points', (deltaTime, time) => {
      // Update center
      canvasSize = getCanvasSize();
      center = getCenter();
      centerX = center.x;
      centerY = center.y;
      
      ctx.fillStyle = 'rgba(10, 10, 15, 0.12)';
      ctx.fillRect(0, 0, canvasSize.width, canvasSize.height);
      
      // Draw connection lines between Lagrange points (equilibrium field)
      const spacing = 80 + (massValue / 100) * 40; // 80-120px based on mass
      const points = [
        { x: centerX - spacing, y: centerY, label: 'L1' },
        { x: centerX + spacing, y: centerY, label: 'L2' },
        { x: centerX, y: centerY - spacing, label: 'L3' },
        { x: centerX - spacing * 0.7, y: centerY + spacing * 0.7, label: 'L4' },
        { x: centerX + spacing * 0.7, y: centerY + spacing * 0.7, label: 'L5' }
      ];
      
      // Draw subtle connection lines
      ctx.save();
      ctx.strokeStyle = `rgba(0, 212, 255, ${0.1 + (stabilityValue / 100) * 0.15})`;
      ctx.lineWidth = 1;
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
      
      // Draw 5 Lagrange points with pulsing
      points.forEach((point, i) => {
        const phase = time * 0.001 + i * 0.4;
        const stabilityEffect = stabilityValue / 100;
        const radius = 12 + Math.sin(phase * 2) * (3 - stabilityEffect * 2); // Less pulsing with more stability
        
        // Glow
        const gradient = ctx.createRadialGradient(point.x, point.y, 0, point.x, point.y, radius * 2);
        gradient.addColorStop(0, 'rgba(0, 212, 255, 0.4)');
        gradient.addColorStop(0.5, 'rgba(0, 212, 255, 0.2)');
        gradient.addColorStop(1, 'rgba(0, 212, 255, 0)');
        
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(point.x, point.y, radius * 2, 0, Math.PI * 2);
        ctx.fill();
        
        // Core point
        ctx.save();
        ctx.shadowColor = '#00d4ff';
        ctx.shadowBlur = 20;
        ctx.fillStyle = '#00d4ff';
        ctx.beginPath();
        ctx.arc(point.x, point.y, radius * 0.5, 0, Math.PI * 2);
        ctx.fill();
        ctx.restore();
        
        // Label with better contrast
        ctx.save();
        ctx.font = 'bold 11px Inter';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillStyle = 'rgba(0, 0, 0, 0.7)'; // Dark background
        ctx.fillRect(point.x - 15, point.y + radius + 8, 30, 16);
        ctx.fillStyle = '#00d4ff';
        ctx.shadowColor = '#00d4ff';
        ctx.shadowBlur = 8;
        ctx.fillText(point.label, point.x, point.y + radius + 16);
        ctx.restore();
      });
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
      sendToJUCE('microChaos', { x, y });
    }
  });
}

function sendToJUCE(param, value) {
  if (window.chrome?.webview) {
    window.chrome.webview.postMessage({ type: 'parameterChange', parameter: param, value });
  }
}
