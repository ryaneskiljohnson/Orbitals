document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('trajectoryCanvas');
  if (canvas) {
    const ctx = CanvasUtils.setupHiDPI(canvas);
    const rect = canvas.getBoundingClientRect();
    animationManager.add('apogee-trajectory', (dt, t) => {
      ctx.fillStyle = 'rgba(10, 10, 15, 0.1)';
      ctx.fillRect(0, 0, rect.width, rect.height);
      const gradient = ctx.createLinearGradient(0, rect.height, 0, 0);
      gradient.addColorStop(0, '#0094ff');
      gradient.addColorStop(0.5, '#00d4ff');
      gradient.addColorStop(1, '#ffffff');
      ctx.strokeStyle = gradient;
      ctx.lineWidth = 3;
      ctx.beginPath();
      for (let i = 0; i <= 100; i++) {
        const x = (i / 100) * rect.width;
        const y = rect.height - Math.pow(i / 100, 2) * rect.height * 0.8;
        if (i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
      }
      ctx.stroke();
    });
  }
  new OrbitalsSlider(document.getElementById('liftSlider'), { min: 0, max: 100, value: 50, orientation: 'vertical' });
  new OrbitalsKnob(document.getElementById('momentumKnob'), { min: 0, max: 100, value: 50 });
});
