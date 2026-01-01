document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('retrogradeCanvas');
  if (canvas) {
    const ctx = CanvasUtils.setupHiDPI(canvas);
    const rect = canvas.getBoundingClientRect();
    let rotation = 0;
    animationManager.add('retrograde-orbit', (dt) => {
      ctx.fillStyle = 'rgba(10, 10, 15, 0.1)';
      ctx.fillRect(0, 0, rect.width, rect.height);
      rotation += 0.02;
      const cx = rect.width / 2, cy = rect.height / 2, r = 120;
      ctx.strokeStyle = '#6b2fb5';
      ctx.lineWidth = 2;
      ctx.beginPath();
      ctx.arc(cx, cy, r, 0, Math.PI * 2);
      ctx.stroke();
      const x = cx + Math.cos(rotation) * r;
      const y = cy + Math.sin(rotation) * r;
      CanvasUtils.circle(ctx, x, y, 8, '#a855f7');
      ctx.strokeStyle = '#ec4899';
      ctx.setLineDash([5, 5]);
      ctx.beginPath();
      ctx.moveTo(cx, cy);
      ctx.lineTo(cx, cy - r);
      ctx.stroke();
      ctx.setLineDash([]);
    });
  }
  new OrbitalsKnob(document.getElementById('scopeKnob'), { min: 0, max: 4, value: 1, step: 1 });
  new OrbitalsSlider(document.getElementById('symmetrySlider'), { min: 0, max: 100, value: 50, orientation: 'horizontal' });
  new OrbitalsKnob(document.getElementById('echoKnob'), { min: 0, max: 8, value: 0, step: 1 });
  document.querySelectorAll('.mode-selector button').forEach(btn => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.mode-selector button').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
    });
  });
});
