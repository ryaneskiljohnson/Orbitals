/**
 * @fileoverview Hadron - Particle Collision Saturation UI Logic
 * @module Hadron App
 */

const state = {
    drive: 50,
    tone: 50,
    bias: 0,
    mix: 100,
    output: 0,
    bypass: false,
    inputLevel: -100,
    outputLevel: -100
};

document.addEventListener('DOMContentLoaded', () => {
    new MandelbrotKnob(document.getElementById('driveKnob'), { min: 0, max: 100, value: 50, onChange: (v, p) => { state.drive = v; document.getElementById('driveValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('toneKnob'), { min: 0, max: 100, value: 50, onChange: (v, p) => { state.tone = v; document.getElementById('toneValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('biasKnob'), { min: -100, max: 100, value: 0, onChange: (v, p) => { state.bias = v; document.getElementById('biasValue').textContent = `${v >= 0 ? '+' : ''}${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('mixKnob'), { min: 0, max: 100, value: 100, onChange: (v, p) => { state.mix = v; document.getElementById('mixValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('outputKnob'), { min: -12, max: 12, value: 0, onChange: (v, p) => { state.output = v; document.getElementById('outputValue').textContent = `${v >= 0 ? '+' : ''}${v.toFixed(1)} dB`; sendToPlugin(p, v); } });
    document.getElementById('bypassToggle').addEventListener('click', (e) => { state.bypass = !state.bypass; e.currentTarget.classList.toggle('active'); e.currentTarget.querySelector('.bypass-text').textContent = e.currentTarget.classList.contains('active') ? 'ON' : 'OFF'; sendToPlugin('bypass', e.currentTarget.classList.contains('active') ? 0 : 1); });
    
    // Particle collision animation - responds to audio
    const canvas = document.getElementById('hadronCanvas');
    if (canvas) {
        const ctx = canvas.getContext('2d');
        canvas.width = 600;
        canvas.height = 600;
        let particles = [];
        let collisionTime = 0;
        
        function animate() {
            // Calculate audio reactivity
            const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
            const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60));
            
            ctx.fillStyle = 'rgba(13, 13, 21, 0.1)';
            ctx.fillRect(0, 0, 600, 600);
            
            collisionTime++;
            
            // Collision frequency responds to audio
            const collisionFreq = Math.floor(60 - normalizedLevel * 40);
            if (collisionTime % collisionFreq === 0) {
                const particleCount = 50 + Math.floor(normalizedLevel * 50);
                for (let i = 0; i < particleCount; i++) {
                    const angle = Math.random() * Math.PI * 2;
                    const speed = (Math.random() * 5 + 2) * (1 + normalizedLevel);
                    particles.push({ 
                        x: 300, 
                        y: 300, 
                        vx: Math.cos(angle) * speed, 
                        vy: Math.sin(angle) * speed, 
                        life: 60, 
                        color: Math.random() > 0.5 ? '#ff0080' : '#ffd700',
                        size: 2 + normalizedLevel * 3
                    });
                }
            }
            
            particles = particles.filter(p => p.life > 0);
            particles.forEach(p => {
                p.x += p.vx;
                p.y += p.vy;
                p.life--;
                const alpha = (p.life / 60) * (0.7 + normalizedLevel * 0.3);
                
                const grad = ctx.createRadialGradient(p.x, p.y, 0, p.x, p.y, p.size * 3);
                grad.addColorStop(0, p.color.replace(')', `, ${alpha})`).replace('#', 'rgba(').replace(/(.{2})/g, '$1,').slice(0, -1));
                grad.addColorStop(1, p.color.replace(')', ', 0)').replace('#', 'rgba('));
                ctx.fillStyle = grad;
                ctx.beginPath();
                ctx.arc(p.x, p.y, p.size * 3, 0, Math.PI * 2);
                ctx.fill();
            });
            
            requestAnimationFrame(animate);
        }
        animate();
    }
});

function sendToPlugin(parameter, value) {
    if (window.juce) {
        window.juce.postMessage({
            type: 'parameterChange',
            parameter: parameter,
            value: value
        });
    }
}

// Receive audio data from C++ for reactive animations
window.receiveAudioData = function(data) {
    if (!data) return;
    
    state.inputLevel = data.inputLevel || -100;
    state.outputLevel = data.outputLevel || -100;
};
