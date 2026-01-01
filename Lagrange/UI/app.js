document.addEventListener('DOMContentLoaded', () => {
  console.log('Lagrange initializing...');
  
  const canvas = document.getElementById('lagrangeCanvas');
  if (canvas) setupLagrangeVisualization(canvas);
  
  setupControls();
});

function setupLagrangeVisualization(canvas) {
  const ctx = CanvasUtils.setupHiDPI(canvas);
  const rect = canvas.getBoundingClientRect();
  
  animationManager.add('lagrange-points', (deltaTime, time) => {
    ctx.fillStyle = 'rgba(10, 10, 15, 0.1)';
    ctx.fillRect(0, 0, rect.width, rect.height);
    
    // Draw 5 Lagrange points
    const centerX = rect.width / 2;
    const centerY = rect.height / 2;
    const points = [
      { x: centerX - 100, y: centerY, label: 'L1' },
      { x: centerX + 100, y: centerY, label: 'L2' },
      { x: centerX, y: centerY - 100, label: 'L3' },
      { x: centerX - 70, y: centerY + 70, label: 'L4' },
      { x: centerX + 70, y: centerY + 70, label: 'L5' }
    ];
    
    points.forEach((point, i) => {
      const phase = time * 0.001 + i;
      const radius = 15 + Math.sin(phase) * 3;
      
      const gradient = ctx.createRadialGradient(point.x, point.y, 0, point.x, point.y, radius);
      gradient.addColorStop(0, '#00d4ff');
      gradient.addColorStop(1, 'rgba(0, 212, 255, 0)');
      
      ctx.fillStyle = gradient;
      ctx.beginPath();
      ctx.arc(point.x, point.y, radius, 0, Math.PI * 2);
      ctx.fill();
      
      CanvasUtils.glowText(ctx, point.label, point.x, point.y + radius + 15, '#00d4ff', '#00d4ff', 10);
    });
  });
}

function setupControls() {
  new OrbitalsSlider(document.getElementById('stabilitySlider'), {
    min: 0, max: 100, value: 50,
    orientation: 'vertical',
    onChange: (v) => {
      document.getElementById('stabilityValue').textContent = Math.round(v) + '%';
      sendToJUCE('stability', v);
    }
  });
  
  new OrbitalsKnob(document.getElementById('massKnob'), {
    min: 0, max: 100, value: 50,
    onChange: (v) => {
      document.getElementById('massValue').textContent = Math.round(v) + '%';
      sendToJUCE('mass', v);
    }
  });
  
  new OrbitalsRangeSlider(document.getElementById('driftSlider'), {
    min: -50, max: 50, valueMin: -25, valueMax: 25,
    onChange: (min, max) => {
      document.getElementById('driftValue').textContent = `${Math.round(min)}ms to ${Math.round(max)}ms`;
      sendToJUCE('drift', { min, max });
    }
  });
  
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
