/**
 * @fileoverview CymaSynth Application Logic
 * @module CymaSynth App
 */

/* ===================================================================
   INITIALIZATION
   =================================================================== */

document.addEventListener('DOMContentLoaded', () => {
  // Setup canvas
  const canvas = document.getElementById('cymasynthCanvas');
  if (canvas) {
    setupCymaSynthVisualization(canvas);
  }
});

/* ===================================================================
   CYMASYNTH VISUALIZATION
   =================================================================== */

function setupCymaSynthVisualization(canvas) {
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
    
    // Animation loop
    animationManager.add('cymasynth-visualization', (deltaTime) => {
      try {
        time += deltaTime * 0.001;
        
        canvasSize = getCanvasSize();
        centerX = canvasSize.width / 2;
        centerY = canvasSize.height / 2;
        
        // Clear canvas
        ctx.clearRect(0, 0, canvasSize.width, canvasSize.height);
      
        // Simple visualization - pulsing circle
        const radius = 50 + Math.sin(time * 2) * 20;
        const gradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, radius);
        gradient.addColorStop(0, 'rgba(56, 189, 248, 0.8)');
        gradient.addColorStop(0.5, 'rgba(56, 189, 248, 0.4)');
        gradient.addColorStop(1, 'rgba(56, 189, 248, 0)');
        
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
        ctx.fill();
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

function sendParameterToJUCE(param, value) {
  if (window.chrome?.webview) {
    window.chrome.webview.postMessage({ type: 'parameterChange', parameter: param, value });
  }
}

window.receiveMessageFromJUCE = function(message) {
  if (!message || typeof message !== 'object') return;
  // No MIDI handling - all removed
};
