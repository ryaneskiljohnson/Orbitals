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
    window.updateOrbitCount = (value) => { orbitCount = Math.max(1, Math.min(8, value)); };
    window.updateEccentricity = (value) => { eccentricity = Math.max(0, Math.min(0.95, value)); };
    window.updateRotationSpeed = (value) => { rotationSpeed = value; };
    
    animationManager.add('kepler-orbits', (deltaTime) => {
      time += (rotationSpeed / 100) * 0.015;
      canvasSize = getCanvasSize();
      center = getCenter();
      centerX = center.x;
      centerY = center.y;
      
      ctx.fillStyle = 'rgba(10, 10, 15, 0.12)';
      ctx.fillRect(0, 0, canvasSize.width, canvasSize.height);
      
      // Draw central sun
      const sunGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, 15);
      sunGradient.addColorStop(0, '#fbbf24');
      sunGradient.addColorStop(0.5, '#f59e0b');
      sunGradient.addColorStop(1, 'rgba(245, 158, 11, 0)');
      
      ctx.save();
      ctx.shadowColor = '#fbbf24';
      ctx.shadowBlur = 25;
      ctx.fillStyle = sunGradient;
      ctx.beginPath();
      ctx.arc(centerX, centerY, 15, 0, Math.PI * 2);
      ctx.fill();
      ctx.restore();
      
      // Draw orbits and planets
      for (let i = 0; i < orbitCount; i++) {
        const orbitRadius = 60 + i * 45;
        const eccent = eccentricity;
        const alpha = 0.4 - (i / orbitCount) * 0.15;
        
        // Draw elliptical orbit path
        ctx.save();
        ctx.strokeStyle = `rgba(59, 130, 246, ${alpha})`;
        ctx.lineWidth = 1.5;
        ctx.setLineDash([4, 8]);
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
        
        // Calculate planet position using Kepler's laws
        const planetAngle = time + i * (Math.PI * 2 / orbitCount);
        const planetR = (orbitRadius * (1 - eccent * eccent)) / (1 + eccent * Math.cos(planetAngle));
        const px = centerX + planetR * Math.cos(planetAngle);
        const py = centerY + planetR * Math.sin(planetAngle);
        
        // Draw planet with glow
        const planetGradient = ctx.createRadialGradient(px, py, 0, px, py, 12);
        planetGradient.addColorStop(0, '#60a5fa');
        planetGradient.addColorStop(0.5, 'rgba(96, 165, 250, 0.5)');
        planetGradient.addColorStop(1, 'rgba(96, 165, 250, 0)');
        ctx.fillStyle = planetGradient;
        ctx.beginPath();
        ctx.arc(px, py, 12, 0, Math.PI * 2);
        ctx.fill();
        
        // Planet core
        ctx.save();
        ctx.shadowColor = '#60a5fa';
        ctx.shadowBlur = 15;
        ctx.fillStyle = '#60a5fa';
        ctx.beginPath();
        ctx.arc(px, py, 5, 0, Math.PI * 2);
        ctx.fill();
        ctx.restore();
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
  new OrbitalsKnob(document.getElementById('orbitCountKnob'), {
    min: 1, max: 8, value: 4, step: 1,
    onChange: (v) => {
      document.getElementById('orbitCountValue').textContent = Math.round(v);
      if (window.updateOrbitCount) window.updateOrbitCount(v);
    }
  });
  
  new OrbitalsKnob(document.getElementById('rotationKnob'), {
    min: 0, max: 100, value: 50,
    onChange: (v) => {
      document.getElementById('rotationValue').textContent = Math.round(v) + '%';
      if (window.updateRotationSpeed) window.updateRotationSpeed(v);
    }
  });
  
  new OrbitalsSlider(document.getElementById('eccentricitySlider'), {
    min: 0, max: 0.95, value: 0.5, step: 0.01,
    orientation: 'horizontal',
    onChange: (v) => {
      document.getElementById('eccentricityValue').textContent = v.toFixed(2);
      if (window.updateEccentricity) window.updateEccentricity(v);
    }
  });
  
  new OrbitalsXYPad(document.getElementById('stabilityPad'), {
    minX: 0, maxX: 100, minY: 0, maxY: 100,
    valueX: 50, valueY: 50,
    onChange: (x, y) => {
      document.getElementById('stabilityValue').textContent = `X:${Math.round(x)} Y:${Math.round(y)}`;
    }
  });
}
