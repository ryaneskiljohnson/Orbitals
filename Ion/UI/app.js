document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('ionCanvas');
  if (canvas) setupIonVisualization(canvas);
  
  setupControls();
});

function setupIonVisualization(canvas) {
  const initCanvas = () => {
    let ctx = CanvasUtils.setupHiDPI(canvas);
    
    const getCanvasSize = () => {
      const rect = canvas.getBoundingClientRect();
      return { width: rect.width, height: rect.height };
    };
    
    // Get center based on Field knob position
    const getCenter = () => {
      const size = getCanvasSize();
      const canvasRect = canvas.getBoundingClientRect();
      
      const fieldKnob = document.getElementById('fieldKnob');
      if (fieldKnob) {
        const knobRect = fieldKnob.getBoundingClientRect();
        const knobCenterX = knobRect.left + knobRect.width / 2 - canvasRect.left;
        const knobCenterY = knobRect.top + knobRect.height / 2 - canvasRect.top;
        return { x: knobCenterX, y: knobCenterY };
      }
      
      return { x: size.width / 2, y: size.height / 2 };
    };
    
    let canvasSize = getCanvasSize();
    let center = getCenter();
    let centerX = center.x;
    let centerY = center.y;
    
    let fieldValue = 50;
    let tensionValue = 50;
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
    window.updateFieldValue = (value) => { fieldValue = value; };
    window.updateTensionValue = (value) => { tensionValue = value; };
    
    animationManager.add('ion-field', (deltaTime) => {
      time += deltaTime * 0.001;
      canvasSize = getCanvasSize();
      center = getCenter();
      centerX = center.x;
      centerY = center.y;
      
      ctx.fillStyle = 'rgba(10, 10, 15, 0.12)';
      ctx.fillRect(0, 0, canvasSize.width, canvasSize.height);
      
      // Draw magnetic field rings
      const fieldWidth = 50 + (fieldValue / 100) * 100; // 50-150px
      const ringCount = 5;
      for (let i = 0; i < ringCount; i++) {
        const r = 60 + i * 30;
        const alpha = 0.25 - (i / ringCount) * 0.15;
        const pulse = Math.sin(time * 2 + i * 0.5) * 0.05;
        
        ctx.save();
        ctx.strokeStyle = `rgba(139, 92, 246, ${alpha + pulse})`;
        ctx.lineWidth = 1.5;
        ctx.setLineDash([3, 6]);
        ctx.beginPath();
        ctx.arc(centerX, centerY, r, 0, Math.PI * 2);
        ctx.stroke();
        ctx.restore();
      }
      
      // Draw 12 chromatic field lines (like magnetic field lines)
      const noteCount = 12;
      for (let i = 0; i < noteCount; i++) {
        const angle = (i / noteCount) * Math.PI * 2 - Math.PI / 2;
        const innerRadius = 50;
        const outerRadius = 50 + fieldWidth;
        
        const x1 = centerX + Math.cos(angle) * innerRadius;
        const y1 = centerY + Math.sin(angle) * innerRadius;
        const x2 = centerX + Math.cos(angle) * outerRadius;
        const y2 = centerY + Math.sin(angle) * outerRadius;
        
        // Pulsing field lines
        const pulse = Math.sin(time * 3 + i * 0.3) * 0.1;
        const alpha = 0.15 + pulse + (tensionValue / 100) * 0.15;
        
        const gradient = ctx.createLinearGradient(x1, y1, x2, y2);
        gradient.addColorStop(0, `rgba(139, 92, 246, ${alpha})`);
        gradient.addColorStop(1, `rgba(6, 182, 212, ${alpha * 0.5})`);
        
        ctx.save();
        ctx.strokeStyle = gradient;
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        ctx.moveTo(x1, y1);
        ctx.lineTo(x2, y2);
        ctx.stroke();
        ctx.restore();
        
        // Note markers at field line ends
        const markerSize = 3 + Math.sin(time * 2 + i * 0.5) * 1;
        ctx.save();
        ctx.shadowColor = '#8b5cf6';
        ctx.shadowBlur = 10;
        ctx.fillStyle = '#8b5cf6';
        ctx.beginPath();
        ctx.arc(x2, y2, markerSize, 0, Math.PI * 2);
        ctx.fill();
        ctx.restore();
      }
      
      // Draw center core (root note)
      const coreGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, 25);
      coreGradient.addColorStop(0, 'rgba(139, 92, 246, 0.6)');
      coreGradient.addColorStop(0.5, 'rgba(139, 92, 246, 0.3)');
      coreGradient.addColorStop(1, 'rgba(139, 92, 246, 0)');
      ctx.fillStyle = coreGradient;
      ctx.beginPath();
      ctx.arc(centerX, centerY, 25, 0, Math.PI * 2);
      ctx.fill();
      
      ctx.save();
      ctx.shadowColor = '#8b5cf6';
      ctx.shadowBlur = 20;
      ctx.fillStyle = '#8b5cf6';
      ctx.beginPath();
      ctx.arc(centerX, centerY, 8, 0, Math.PI * 2);
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
  new OrbitalsSlider(document.getElementById('tensionSlider'), {
    min: 0, max: 100, value: 50,
    orientation: 'vertical',
    onChange: (v) => {
      document.getElementById('tensionValue').textContent = Math.round(v) + '%';
      if (window.updateTensionValue) window.updateTensionValue(v);
    }
  });
  
  new OrbitalsKnob(document.getElementById('fieldKnob'), {
    min: 0, max: 100, value: 50,
    onChange: (v) => {
      document.getElementById('fieldValue').textContent = Math.round(v) + '%';
      if (window.updateFieldValue) window.updateFieldValue(v);
    }
  });
  
  document.querySelectorAll('.key-segment').forEach(seg => {
    seg.addEventListener('click', () => {
      document.querySelectorAll('.key-segment').forEach(s => s.classList.remove('active'));
      seg.classList.add('active');
    });
  });
}
