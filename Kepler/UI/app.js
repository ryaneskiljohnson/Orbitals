document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('keplerCanvas');
  if (canvas) {
    const ctx = CanvasUtils.setupHiDPI(canvas);
    const rect = canvas.getBoundingClientRect();
    let time = 0;
    animationManager.add('kepler-orbits', (dt) => {
      ctx.fillStyle = 'rgba(10, 10, 15, 0.1)';
      ctx.fillRect(0, 0, rect.width, rect.height);
      time += 0.01;
      const cx = rect.width / 2, cy = rect.height / 2;
      ctx.fillStyle = '#fbbf24';
      ctx.beginPath();
      ctx.arc(cx, cy, 10, 0, Math.PI * 2);
      ctx.fill();
      for (let i = 0; i < 4; i++) {
        const orbitRadius = 60 + i * 50;
        const eccent = 0.5;
        ctx.strokeStyle = `rgba(59, 130, 246, ${0.5 - i * 0.1})`;
        ctx.lineWidth = 2;
        ctx.beginPath();
        for (let angle = 0; angle < Math.PI * 2; angle += 0.1) {
          const r = (orbitRadius * (1 - eccent * eccent)) / (1 + eccent * Math.cos(angle));
          const x = cx + r * Math.cos(angle);
          const y = cy + r * Math.sin(angle);
          if (angle === 0) ctx.moveTo(x, y);
          else ctx.lineTo(x, y);
        }
        ctx.closePath();
        ctx.stroke();
        const planetAngle = time + i * Math.PI / 2;
        const planetR = (orbitRadius * (1 - eccent * eccent)) / (1 + eccent * Math.cos(planetAngle));
        const px = cx + planetR * Math.cos(planetAngle);
        const py = cy + planetR * Math.sin(planetAngle);
        ctx.fillStyle = '#60a5fa';
        ctx.beginPath();
        ctx.arc(px, py, 6, 0, Math.PI * 2);
        ctx.fill();
      }
    });
  }
  new OrbitalsKnob(document.getElementById('orbitCountKnob'), { min: 1, max: 8, value: 4, step: 1 });
  new OrbitalsKnob(document.getElementById('rotationKnob'), { min: 0, max: 7, value: 2, step: 1 });
  new OrbitalsSlider(document.getElementById('eccentricitySlider'), { min: 0, max: 1, value: 0.5, step: 0.01, orientation: 'horizontal' });
  new OrbitalsXYPad(document.getElementById('stabilityPad'), { minX: 0, maxX: 100, minY: 0, maxY: 100, valueX: 50, valueY: 50 });
});
