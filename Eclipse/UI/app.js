document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('eclipseCanvas');
  if (canvas) setupEclipseVisualization(canvas);
  
  setupControls();
});

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
      return { x: size.width / 2, y: size.height / 2 };
    };
    
    let canvasSize = getCanvasSize();
    let center = getCenter();
    let centerX = center.x;
    let centerY = center.y;
    
    let eclipsePhase = 0.5; // Start at partial eclipse
    let thresholdValue = 64;
    let ghostValue = -12;
    let time = 0;
    
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
    window.updateGhostValue = (value) => { ghostValue = value; };
    
    animationManager.add('eclipse-animation', (deltaTime) => {
      time += deltaTime * 0.001;
      canvasSize = getCanvasSize();
      center = getCenter();
      centerX = center.x;
      centerY = center.y;
      
      ctx.fillStyle = 'rgba(10, 10, 15, 1)';
      ctx.fillRect(0, 0, canvasSize.width, canvasSize.height);
      
      const sunRadius = 80;
      const moonRadius = 75;
      
      // Draw corona rays (always visible, even during eclipse)
      const rayCount = 24;
      for (let i = 0; i < rayCount; i++) {
        const angle = (i / rayCount) * Math.PI * 2 + time * 0.5;
        const rayLength = 40 + Math.sin(time * 2 + i) * 15;
        const x1 = centerX + Math.cos(angle) * sunRadius;
        const y1 = centerY + Math.sin(angle) * sunRadius;
        const x2 = centerX + Math.cos(angle) * (sunRadius + rayLength);
        const y2 = centerY + Math.sin(angle) * (sunRadius + rayLength);
        
        const gradient = ctx.createLinearGradient(x1, y1, x2, y2);
        gradient.addColorStop(0, 'rgba(251, 191, 36, 0.6)');
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
      
      // Draw sun
      const sunGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, sunRadius);
      sunGradient.addColorStop(0, '#ffffff');
      sunGradient.addColorStop(0.3, '#fbbf24');
      sunGradient.addColorStop(0.7, '#f59e0b');
      sunGradient.addColorStop(1, 'rgba(245, 158, 11, 0)');
      
      ctx.save();
      ctx.shadowColor = '#fbbf24';
      ctx.shadowBlur = 40;
      ctx.fillStyle = sunGradient;
      ctx.beginPath();
      ctx.arc(centerX, centerY, sunRadius, 0, Math.PI * 2);
      ctx.fill();
      ctx.restore();
      
      // Draw moon (eclipsing from left to right based on threshold)
      const moonOffset = (eclipsePhase - 0.5) * sunRadius * 2.5; // -1.25 to +1.25 sun radius
      const moonX = centerX + moonOffset;
      
      // Moon shadow
      ctx.save();
      ctx.globalCompositeOperation = 'destination-out';
      ctx.fillStyle = '#000000';
      ctx.beginPath();
      ctx.arc(moonX, centerY, moonRadius, 0, Math.PI * 2);
      ctx.fill();
      ctx.restore();
      
      // Moon outline (corona edge)
      ctx.save();
      ctx.strokeStyle = 'rgba(251, 191, 36, 0.8)';
      ctx.lineWidth = 2;
      ctx.shadowColor = '#fbbf24';
      ctx.shadowBlur = 15;
      ctx.beginPath();
      ctx.arc(moonX, centerY, moonRadius, 0, Math.PI * 2);
      ctx.stroke();
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
  new OrbitalsSlider(document.getElementById('thresholdSlider'), {
    min: 0, max: 127, value: 64,
    orientation: 'vertical',
    onChange: (v) => {
      document.getElementById('thresholdValue').textContent = Math.round(v);
      if (window.updateThresholdValue) window.updateThresholdValue(v);
    }
  });
  
  new OrbitalsKnob(document.getElementById('ghostKnob'), {
    min: -60, max: 0, value: -12,
    onChange: (v) => {
      document.getElementById('ghostValue').textContent = Math.round(v) + 'dB';
      if (window.updateGhostValue) window.updateGhostValue(v);
    }
  });
  
  document.querySelectorAll('.toggle-switch-eclipse span').forEach(span => {
    span.addEventListener('click', () => {
      document.querySelectorAll('.toggle-switch-eclipse span').forEach(s => s.classList.remove('active'));
      span.classList.add('active');
    });
  });
}
