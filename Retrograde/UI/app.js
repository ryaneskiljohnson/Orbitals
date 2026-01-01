document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('retrogradeCanvas');
  if (canvas) setupRetrogradeVisualization(canvas);
  
  setupControls();
});

function setupRetrogradeVisualization(canvas) {
  const initCanvas = () => {
    let ctx = CanvasUtils.setupHiDPI(canvas);
    
    const getCanvasSize = () => {
      const rect = canvas.getBoundingClientRect();
      return { width: rect.width, height: rect.height };
    };
    
    // Get center - centered on canvas
    const getCenter = () => {
      const size = getCanvasSize();
      return { x: size.width / 2, y: size.height / 2 };
    };
    
    let canvasSize = getCanvasSize();
    let center = getCenter();
    let centerX = center.x;
    let centerY = center.y;
    
    let rotation = 0;
    let scopeValue = 1;
    let symmetryValue = 50;
    let echoValue = 0;
    
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
    window.updateScopeValue = (value) => { scopeValue = value; };
    window.updateSymmetryValue = (value) => { symmetryValue = value; };
    window.updateEchoValue = (value) => { echoValue = value; };
    
    animationManager.add('retrograde-orbit', (deltaTime, time) => {
      canvasSize = getCanvasSize();
      center = getCenter();
      centerX = center.x;
      centerY = center.y;
      
      ctx.fillStyle = 'rgba(10, 10, 15, 0.12)';
      ctx.fillRect(0, 0, canvasSize.width, canvasSize.height);
      
      // Counter-clockwise rotation (retrograde motion)
      rotation -= 0.015;
      
      const baseRadius = 100;
      
      // Draw mirror line (symmetry axis)
      const symmetryAngle = (symmetryValue / 100) * Math.PI;
      ctx.save();
      ctx.strokeStyle = 'rgba(236, 72, 153, 0.3)';
      ctx.lineWidth = 2;
      ctx.setLineDash([8, 12]);
      ctx.beginPath();
      const lineLength = Math.max(canvasSize.width, canvasSize.height) * 0.5;
      ctx.moveTo(centerX - Math.cos(symmetryAngle) * lineLength, centerY - Math.sin(symmetryAngle) * lineLength);
      ctx.lineTo(centerX + Math.cos(symmetryAngle) * lineLength, centerY + Math.sin(symmetryAngle) * lineLength);
      ctx.stroke();
      ctx.restore();
      
      // Draw multiple orbits based on scope
      const orbitCount = Math.max(1, scopeValue);
      for (let orbit = 0; orbit < orbitCount; orbit++) {
        const r = baseRadius + orbit * 35;
        const orbitAlpha = 0.3 - (orbit / orbitCount) * 0.2;
        
        // Orbit ring
        ctx.save();
        ctx.strokeStyle = `rgba(107, 47, 181, ${orbitAlpha})`;
        ctx.lineWidth = 1.5;
        ctx.setLineDash([4, 8]);
        ctx.beginPath();
        ctx.arc(centerX, centerY, r, 0, Math.PI * 2);
        ctx.stroke();
        ctx.restore();
        
        // Orbiting particle (counter-clockwise)
        const particleAngle = rotation + orbit * (Math.PI / orbitCount);
        const x = centerX + Math.cos(particleAngle) * r;
        const y = centerY + Math.sin(particleAngle) * r;
        
        // Draw particle with glow
        const gradient = ctx.createRadialGradient(x, y, 0, x, y, 12);
        gradient.addColorStop(0, '#a855f7');
        gradient.addColorStop(0.5, 'rgba(168, 85, 247, 0.5)');
        gradient.addColorStop(1, 'rgba(168, 85, 247, 0)');
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(x, y, 12, 0, Math.PI * 2);
        ctx.fill();
        
        // Core
        ctx.save();
        ctx.shadowColor = '#a855f7';
        ctx.shadowBlur = 15;
        ctx.fillStyle = '#a855f7';
        ctx.beginPath();
        ctx.arc(x, y, 4, 0, Math.PI * 2);
        ctx.fill();
        ctx.restore();
        
        // Mirror particle (reflected across symmetry line) if echo > 0
        if (echoValue > 0 && orbit < echoValue) {
          const mirrorAngle = 2 * symmetryAngle - particleAngle;
          const mx = centerX + Math.cos(mirrorAngle) * r;
          const my = centerY + Math.sin(mirrorAngle) * r;
          
          const mirrorGradient = ctx.createRadialGradient(mx, my, 0, mx, my, 10);
          mirrorGradient.addColorStop(0, 'rgba(236, 72, 153, 0.6)');
          mirrorGradient.addColorStop(1, 'rgba(236, 72, 153, 0)');
          ctx.fillStyle = mirrorGradient;
          ctx.beginPath();
          ctx.arc(mx, my, 10, 0, Math.PI * 2);
          ctx.fill();
          
          ctx.save();
          ctx.shadowColor = '#ec4899';
          ctx.shadowBlur = 12;
          ctx.fillStyle = '#ec4899';
          ctx.beginPath();
          ctx.arc(mx, my, 3, 0, Math.PI * 2);
          ctx.fill();
          ctx.restore();
        }
      }
      
      // Draw center point
      ctx.save();
      ctx.shadowColor = '#6b2fb5';
      ctx.shadowBlur = 25;
      ctx.fillStyle = '#6b2fb5';
      ctx.beginPath();
      ctx.arc(centerX, centerY, 6, 0, Math.PI * 2);
      ctx.fill();
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
  // Scope Knob (1-4 notes/bars)
  new OrbitalsKnob(document.getElementById('scopeKnob'), {
    min: 1, max: 4, value: 1, step: 1,
    onChange: (v) => {
      document.getElementById('scopeValue').textContent = Math.round(v);
      if (window.updateScopeValue) window.updateScopeValue(v);
    }
  });
  
  // Symmetry Slider (0-360 degrees, displayed as 0-100 for simplicity)
  new OrbitalsSlider(document.getElementById('symmetrySlider'), {
    min: 0, max: 360, value: 180,
    orientation: 'horizontal',
    onChange: (v) => {
      document.getElementById('symmetryValue').textContent = Math.round(v) + '°';
      if (window.updateSymmetryValue) window.updateSymmetryValue(v / 360 * 100); // Normalize to 0-100 for internal use
    }
  });
  
  // Echo Knob (0-8 echoes)
  new OrbitalsKnob(document.getElementById('echoKnob'), {
    min: 0, max: 8, value: 0, step: 1,
    onChange: (v) => {
      document.getElementById('echoValue').textContent = Math.round(v);
      if (window.updateEchoValue) window.updateEchoValue(v);
    }
  });
  
  // Mode selector buttons
  document.querySelectorAll('.mode-selector button').forEach(btn => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.mode-selector button').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      const mode = btn.dataset.mode;
      console.log('Reverse mode:', mode);
    });
  });
}
