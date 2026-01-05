/**
 * @fileoverview Rabbithole - Quantum Descent Flanger UI Logic
 * @module Rabbithole App
 */

const state = {
    rate: 0.5,
    depth: 50,
    feedback: 30,
    stereo: 100,
    mix: 50,
    bypass: false,
    inputLevel: -100,
    outputLevel: -100
};

document.addEventListener('DOMContentLoaded', () => {
    new MandelbrotKnob(document.getElementById('rateKnob'), { min: 0.1, max: 10, value: 0.5, onChange: (v, p) => { state.rate = v; document.getElementById('rateValue').textContent = `${v.toFixed(1)} Hz`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('depthKnob'), { min: 0, max: 100, value: 50, onChange: (v, p) => { state.depth = v; document.getElementById('depthValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('feedbackKnob'), { min: 0, max: 100, value: 30, onChange: (v, p) => { state.feedback = v; document.getElementById('feedbackValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('stereoKnob'), { min: 0, max: 100, value: 100, onChange: (v, p) => { state.stereo = v; document.getElementById('stereoValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('mixKnob'), { min: 0, max: 100, value: 50, onChange: (v, p) => { state.mix = v; document.getElementById('mixValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    document.getElementById('bypassToggle').addEventListener('click', (e) => { state.bypass = !state.bypass; e.currentTarget.classList.toggle('active'); e.currentTarget.querySelector('.bypass-text').textContent = e.currentTarget.classList.contains('active') ? 'ON' : 'OFF'; sendToPlugin('bypass', e.currentTarget.classList.contains('active') ? 0 : 1); });
    
    // Rabbit hole spiral animation - responds to audio
    const canvas = document.getElementById('rabbitholeCanvas');
    if (canvas) {
        const ctx = canvas.getContext('2d');
        canvas.width = 600;
        canvas.height = 600;
        let time = 0;
        
        function animate() {
            // Calculate audio reactivity
            const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
            const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60));
            
            ctx.fillStyle = 'rgba(13, 13, 21, 0.05)';
            ctx.fillRect(0, 0, 600, 600);
            
            time += 0.05 * (1 + normalizedLevel * 0.5);
            
            // Number of rings responds to audio
            const ringCount = 20 + Math.floor(normalizedLevel * 10);
            
            for (let i = 0; i < ringCount; i++) {
                // Ring size and movement respond to audio
                const baseRadius = 50 + i * 25;
                const wobble = Math.sin(time + i * 0.5) * (10 + normalizedLevel * 10);
                const radius = baseRadius + wobble;
                
                const hue = 270 - i * 10 + time * 20;
                const opacity = (0.6 - i * 0.03) * (0.7 + normalizedLevel * 0.3);
                const lineWidth = 3 + normalizedLevel * 2;
                
                ctx.strokeStyle = `hsla(${hue}, 100%, 50%, ${opacity})`;
                ctx.lineWidth = lineWidth;
                ctx.beginPath();
                ctx.arc(300, 300, radius, 0, Math.PI * 2);
                ctx.stroke();
            }
            
            // Central vortex - pulses with audio
            const vortexSize = 20 + normalizedLevel * 20;
            const vortexIntensity = 0.8 + normalizedLevel * 0.2;
            const vortexGrad = ctx.createRadialGradient(300, 300, 0, 300, 300, vortexSize);
            vortexGrad.addColorStop(0, `rgba(139, 61, 255, ${vortexIntensity})`);
            vortexGrad.addColorStop(1, 'rgba(139, 61, 255, 0)');
            ctx.fillStyle = vortexGrad;
            ctx.beginPath();
            ctx.arc(300, 300, vortexSize, 0, Math.PI * 2);
            ctx.fill();
            
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
