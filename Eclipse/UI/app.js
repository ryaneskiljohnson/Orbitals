document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('eclipseCanvas');
  if (canvas) {
    const ctx = CanvasUtils.setupHiDPI(canvas);
    const rect = canvas.getBoundingClientRect();
    let eclipsePhase = 0;
    animationManager.add('eclipse-animation', (dt) => {
      ctx.fillStyle = 'rgba(10, 10, 15, 1)';
      ctx.fillRect(0, 0, rect.width, rect.height);
      const cx = rect.width / 2, cy = rect.height / 2;
      const sunGradient = ctx.createRadialGradient(cx, cy, 0, cx, cy, 100);
      sunGradient.addColorStop(0, '#ffffff');
      sunGradient.addColorStop(0.3, '#fbbf24');
      sunGradient.addColorStop(1, 'rgba(251, 191, 36, 0)');
      ctx.fillStyle = sunGradient;
      ctx.beginPath();
      ctx.arc(cx, cy, 100, 0, Math.PI * 2);
      ctx.fill();
      eclipsePhase = (eclipsePhase + 0.005) % 1;
      const moonX = cx - 200 + eclipsePhase * 400;
      ctx.fillStyle = '#000000';
      ctx.beginPath();
      ctx.arc(moonX, cy, 90, 0, Math.PI * 2);
      ctx.fill();
      ctx.strokeStyle = 'rgba(251, 191, 36, 0.8)';
      ctx.lineWidth = 3;
      ctx.stroke();
    });
  }
  new OrbitalsSlider(document.getElementById('thresholdSlider'), { min: 0, max: 127, value: 64, orientation: 'vertical' });
  new OrbitalsKnob(document.getElementById('ghostKnob'), { min: -60, max: 0, value: -12 });
  document.querySelectorAll('.toggle-switch-eclipse span').forEach(span => {
    span.addEventListener('click', () => {
      document.querySelectorAll('.toggle-switch-eclipse span').forEach(s => s.classList.remove('active'));
      span.classList.add('active');
    });
  });
});
