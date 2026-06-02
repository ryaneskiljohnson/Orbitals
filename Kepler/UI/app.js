// Shared state for both canvases
let globalOrbitCount = 4;
let globalEccentricity = 0.5;
let globalRotationSpeed = 50;
let globalDirection = 'cw';

document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('keplerCanvas');
  if (canvas) setupKeplerVisualization(canvas);
  
  setupControls();
});

function setupKeplerVisualization(canvas) {
  const initCanvas = () => {
    let ctx = CanvasUtils.setupHiDPI(canvas);
    
    const getCanvasSize = () => {
      const rect = canvas.getBoundingClientRect();
      return { width: rect.width, height: rect.height };
    };
    
    // Get center
    const getCenter = () => {
      const size = getCanvasSize();
      return { x: size.width / 2, y: size.height / 2 };
    };
    
    let canvasSize = getCanvasSize();
    let center = getCenter();
    let centerX = center.x;
    let centerY = center.y;
    
    let time = 0;
    let orbitCount = 4;
    let eccentricity = 0.5;
    let rotationSpeed = 2;
    
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
    window.updateOrbitCount = (value) => { 
      orbitCount = Math.max(1, Math.min(8, value)); 
      globalOrbitCount = orbitCount;
    };
    window.updateEccentricity = (value) => { 
      eccentricity = Math.max(0, Math.min(0.95, value)); 
      globalEccentricity = eccentricity;
    };
    window.updateRotationSpeed = (value) => { 
      rotationSpeed = value; 
      globalRotationSpeed = value;
    };
    window.updateDirection = (dir) => {
      globalDirection = dir;
    };
    
    animationManager.add('kepler-orbits', (deltaTime) => {
      // Apply direction
      let speedMultiplier = (rotationSpeed / 100) * 0.02;
      if (globalDirection === 'ccw') speedMultiplier *= -1;
      else if (globalDirection === 'alt') speedMultiplier *= Math.sin(time * 0.5);
      
      time += speedMultiplier;
      canvasSize = getCanvasSize();
      center = getCenter();
      centerX = center.x;
      centerY = center.y;
      
      // Clear canvas with fully transparent background to preserve the dark background
      ctx.clearRect(0, 0, canvasSize.width, canvasSize.height);
      
      // Draw central sun
      const sunGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, 20);
      sunGradient.addColorStop(0, '#fef3c7');
      sunGradient.addColorStop(0.3, '#fbbf24');
      sunGradient.addColorStop(0.6, '#f59e0b');
      sunGradient.addColorStop(1, 'rgba(245, 158, 11, 0)');
      
      ctx.save();
      ctx.shadowColor = '#fbbf24';
      ctx.shadowBlur = 30;
      ctx.fillStyle = sunGradient;
      ctx.beginPath();
      ctx.arc(centerX, centerY, 20, 0, Math.PI * 2);
      ctx.fill();
      ctx.restore();
      
      // Sun core
      ctx.fillStyle = '#ffffff';
      ctx.beginPath();
      ctx.arc(centerX, centerY, 8, 0, Math.PI * 2);
      ctx.fill();
      
      // Draw orbits and planets
      for (let i = 0; i < orbitCount; i++) {
        const baseRadius = Math.min(canvasSize.width, canvasSize.height) * 0.15;
        const orbitRadius = baseRadius + i * (baseRadius * 0.7);
        const eccent = eccentricity;
        const alpha = 0.5 - (i / orbitCount) * 0.2;
        
        // Draw elliptical orbit path
        ctx.save();
        ctx.strokeStyle = `rgba(59, 130, 246, ${alpha})`;
        ctx.lineWidth = 2;
        ctx.setLineDash([5, 10]);
        ctx.beginPath();
        
        for (let angle = 0; angle <= Math.PI * 2; angle += 0.03) {
          const r = (orbitRadius * (1 - eccent * eccent)) / (1 + eccent * Math.cos(angle));
          const x = centerX + r * Math.cos(angle);
          const y = centerY + r * Math.sin(angle);
          if (angle === 0) ctx.moveTo(x, y);
          else ctx.lineTo(x, y);
        }
        ctx.closePath();
        ctx.stroke();
        ctx.restore();
        
        // Calculate planet position using Kepler's laws
        const planetAngle = time + i * (Math.PI * 2 / orbitCount);
        const planetR = (orbitRadius * (1 - eccent * eccent)) / (1 + eccent * Math.cos(planetAngle));
        const px = centerX + planetR * Math.cos(planetAngle);
        const py = centerY + planetR * Math.sin(planetAngle);
        
        // Draw planet with glow
        const planetGradient = ctx.createRadialGradient(px, py, 0, px, py, 18);
        planetGradient.addColorStop(0, '#ffffff');
        planetGradient.addColorStop(0.3, '#60a5fa');
        planetGradient.addColorStop(0.6, 'rgba(96, 165, 250, 0.5)');
        planetGradient.addColorStop(1, 'rgba(96, 165, 250, 0)');
        ctx.fillStyle = planetGradient;
        ctx.beginPath();
        ctx.arc(px, py, 18, 0, Math.PI * 2);
        ctx.fill();
        
        // Planet core
        ctx.save();
        ctx.shadowColor = '#60a5fa';
        ctx.shadowBlur = 20;
        ctx.fillStyle = '#60a5fa';
        ctx.beginPath();
        ctx.arc(px, py, 7, 0, Math.PI * 2);
        ctx.fill();
        ctx.restore();
        
        // Planet inner core
        ctx.fillStyle = '#ffffff';
        ctx.beginPath();
        ctx.arc(px, py, 3, 0, Math.PI * 2);
        ctx.fill();
      }
    });
  };
  
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initCanvas);
  } else {
    requestAnimationFrame(() => setTimeout(initCanvas, 0));
  }
}

function setupOrbitPreview(canvas) {
  let ctx = CanvasUtils.setupHiDPI(canvas);
  
  const drawPreview = () => {
    const width = canvas.width;
    const height = canvas.height;
    const centerX = width / 2;
    const centerY = height / 2;
    
    // Clear
    ctx.fillStyle = 'rgba(10, 10, 15, 0.95)';
    ctx.fillRect(0, 0, width, height);
    
    // Draw sun
    ctx.fillStyle = '#fbbf24';
    ctx.shadowColor = '#fbbf24';
    ctx.shadowBlur = 15;
    ctx.beginPath();
    ctx.arc(centerX, centerY, 10, 0, Math.PI * 2);
    ctx.fill();
    ctx.shadowBlur = 0;
    
    // Draw orbits
    for (let i = 0; i < globalOrbitCount; i++) {
      const baseRadius = Math.min(width, height) * 0.15;
      const orbitRadius = baseRadius + i * (baseRadius * 0.5);
      const eccent = globalEccentricity;
      const alpha = 0.6 - (i / globalOrbitCount) * 0.2;
      
      ctx.save();
      ctx.strokeStyle = `rgba(59, 130, 246, ${alpha})`;
      ctx.lineWidth = 1.5;
      ctx.beginPath();
      
      for (let angle = 0; angle <= Math.PI * 2; angle += 0.05) {
        const r = (orbitRadius * (1 - eccent * eccent)) / (1 + eccent * Math.cos(angle));
        const x = centerX + r * Math.cos(angle);
        const y = centerY + r * Math.sin(angle);
        if (angle === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
      ctx.closePath();
      ctx.stroke();
      ctx.restore();
    }
  };
  
  window.updateOrbitPreview = drawPreview;
  
  // Handle resize
  const resizeObserver = new ResizeObserver(() => {
    ctx = CanvasUtils.setupHiDPI(canvas);
    drawPreview();
  });
  resizeObserver.observe(canvas);
  
  // Initial draw
  drawPreview();
}

function setupControls() {
  // Semi-Major Axis Slider (actually controls orbit count)
  new OrbitalsSlider(document.getElementById('semiMajorSlider'), {
    min: 1, max: 8, value: 4, step: 1,
    orientation: 'vertical',
    onChange: (v) => {
      document.getElementById('semiMajorValue').textContent = Math.round(v);
      if (window.updateOrbitCount) window.updateOrbitCount(Math.round(v));
      if (window.updateOrbitPreview) window.updateOrbitPreview();
      sendToJUCE('orbitCount', Math.round(v));
    }
  });
  
  // Eccentricity Knob
  new OrbitalsKnob(document.getElementById('eccentricityKnob'), {
    min: 0, max: 95, value: 50,
    onChange: (v) => {
      const eccValue = v / 100;
      document.getElementById('eccentricityValue').textContent = eccValue.toFixed(2);
      if (window.updateEccentricity) window.updateEccentricity(eccValue);
      if (window.updateOrbitPreview) window.updateOrbitPreview();
      sendToJUCE('eccentricity', eccValue);
    }
  });
  
  // Velocity Spread Slider (actually controls rotation speed)
  new OrbitalsSlider(document.getElementById('spreadSlider'), {
    min: 0, max: 100, value: 50,
    orientation: 'vertical',
    onChange: (v) => {
      document.getElementById('spreadValue').textContent = Math.round(v);
      if (window.updateRotationSpeed) window.updateRotationSpeed(v);
      sendToJUCE('rotationSpeed', v);
    }
  });

  const stabilityPad = document.getElementById('stabilityPad');
  if (stabilityPad) {
    new OrbitalsXYPad(stabilityPad, {
      minX: 0, maxX: 100, minY: 0, maxY: 100,
      valueX: 50, valueY: 50,
      onChange: (x, y) => {
        document.getElementById('stabilityValue').textContent =
          `X:${Math.round(x)} Y:${Math.round(y)}`;
        sendToJUCE('stabilityX', x);
        sendToJUCE('stabilityY', y);
      }
    });
  }
  
  // Period selector buttons
  document.querySelectorAll('.period-selector button').forEach(btn => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.period-selector button').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      const period = btn.dataset.period;
      const periodMap = { bar: 0, beat: 1, half: 2, double: 3 };
      sendToJUCE('period', periodMap[period] ?? 0);
    });
  });
  
  // Direction selector buttons
  document.querySelectorAll('.direction-selector button').forEach(btn => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.direction-selector button').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      const direction = btn.dataset.dir;
      if (window.updateDirection) window.updateDirection(direction);
      const directionMap = { cw: 0, ccw: 1, pendulum: 2 };
      sendToJUCE('direction', directionMap[direction] ?? 0);
    });
  });
  
  // Orbit preview removed - using main canvas only
  
  // Setup bypass toggle
  if (window.setupBypassToggle) {
    window.setupBypassToggle(sendToJUCE);
  }
}

/**
 * @brief Sends parameter changes to C++ via JUCE native bridge (macOS + Windows).
 * @param {string} param Parameter ID.
 * @param {*} value Raw parameter value.
 */
function sendToJUCE(param, value) {
    if (typeof window.postMessageToJUCE !== 'function') return;
    window.postMessageToJUCE({ type: 'parameterChange', parameter: param, value: value });
}
