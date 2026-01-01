document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('ionCanvas');
  if (canvas) {
    const ctx = CanvasUtils.setupHiDPI(canvas);
    const rect = canvas.getBoundingClientRect();
    animationManager.add('ion-field', (dt, t) => {
      ctx.fillStyle = 'rgba(10, 10, 15, 0.1)';
      ctx.fillRect(0, 0, rect.width, rect.height);
      const cx = rect.width / 2, cy = rect.height / 2;
      for (let r = 50; r < 200; r += 30) {
        ctx.strokeStyle = `rgba(139, 92, 246, ${0.3 - (r / 300) * 0.2})`;
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.arc(cx, cy, r, 0, Math.PI * 2);
        ctx.stroke();
      }
      for (let i = 0; i < 12; i++) {
        const angle = (i / 12) * Math.PI * 2;
        const x1 = cx + Math.cos(angle) * 60;
        const y1 = cy + Math.sin(angle) * 60;
        const x2 = cx + Math.cos(angle) * 150;
        const y2 = cy + Math.sin(angle) * 150;
        ctx.strokeStyle = 'rgba(139, 92, 246, 0.2)';
        ctx.beginPath();
        ctx.moveTo(x1, y1);
        ctx.lineTo(x2, y2);
        ctx.stroke();
      }
    });
  }
  new OrbitalsSlider(document.getElementById('tensionSlider'), { min: 0, max: 100, value: 50, orientation: 'vertical' });
  new OrbitalsKnob(document.getElementById('fieldKnob'), { min: 0, max: 100, value: 50 });
  document.querySelectorAll('.key-segment').forEach(seg => {
    seg.addEventListener('click', () => {
      document.querySelectorAll('.key-segment').forEach(s => s.classList.remove('active'));
      seg.classList.add('active');
    });
  });
});
