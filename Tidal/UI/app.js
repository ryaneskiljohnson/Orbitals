document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('tidalCanvas');
  if (canvas) {
    const ctx = CanvasUtils.setupHiDPI(canvas);
    const rect = canvas.getBoundingClientRect();
    let phase = 0;
    animationManager.add('tidal-wave', (dt) => {
      ctx.fillStyle = 'rgba(10, 10, 15, 0.1)';
      ctx.fillRect(0, 0, rect.width, rect.height);
      phase += 0.02;
      ctx.strokeStyle = '#3b82f6';
      ctx.lineWidth = 3;
      ctx.beginPath();
      const amplitude = 50;
      const frequency = 0.02;
      for (let x = 0; x < rect.width; x++) {
        const y = rect.height / 2 + Math.sin(x * frequency + phase) * amplitude;
        if (x === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
      ctx.stroke();
      const moonX = rect.width - 80;
      const moonY = 60;
      const gradient = ctx.createRadialGradient(moonX, moonY, 0, moonX, moonY, 30);
      gradient.addColorStop(0, '#cbd5e1');
      gradient.addColorStop(1, 'rgba(203, 213, 225, 0)');
      ctx.fillStyle = gradient;
      ctx.beginPath();
      ctx.arc(moonX, moonY, 30, 0, Math.PI * 2);
      ctx.fill();
    });
  }
  new OrbitalsKnob(document.getElementById('phaseKnob'), { min: 0, max: 360, value: 0 });
  new OrbitalsKnob(document.getElementById('strengthKnob'), { min: 0, max: 100, value: 50 });
});
