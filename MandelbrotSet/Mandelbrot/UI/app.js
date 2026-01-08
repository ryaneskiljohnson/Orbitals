/**
 * @fileoverview Mandelbrot - Fractal Phaser UI Logic
 * @module Mandelbrot App
 */

const state = {
    rate: 0.5,
    depth: 50,
    feedback: 30,
    stages: 4,
    mix: 50,
    bypass: false,
    inputLevel: -100,
    outputLevel: -100
};

document.addEventListener('DOMContentLoaded', () => {
    initializeSettingsButton();
    new MandelbrotKnob(document.getElementById('rateKnob'), { min: 0.1, max: 10, value: 0.5, onChange: (v, p) => { state.rate = v; document.getElementById('rateValue').textContent = `${v.toFixed(1)} Hz`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('depthKnob'), { min: 0, max: 100, value: 50, onChange: (v, p) => { state.depth = v; document.getElementById('depthValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('feedbackKnob'), { min: 0, max: 100, value: 30, onChange: (v, p) => { state.feedback = v; document.getElementById('feedbackValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('stagesKnob'), { min: 2, max: 12, value: 4, step: 1, onChange: (v, p) => { state.stages = v; document.getElementById('stagesValue').textContent = v.toFixed(0); sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('mixKnob'), { min: 0, max: 100, value: 50, onChange: (v, p) => { state.mix = v; document.getElementById('mixValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    initializeBypassToggle();
    
    // Mandelbrot fractal animation - responds to audio
    const canvas = document.getElementById('mandelbrotCanvas');
    if (canvas) {
        const ctx = canvas.getContext('2d');
        canvas.width = 600;
        canvas.height = 600;
        let zoom = 1;
        let zoomTarget = 1;
        let centerX = -0.5;
        let centerY = 0;
        let time = 0;
        
        function drawMandelbrot() {
            // Calculate audio reactivity
            const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
            const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60));
            
            time += 0.01;
            
            // Zoom responds to audio
            zoomTarget = 1 + normalizedLevel * 2;
            zoom += (zoomTarget - zoom) * 0.05;
            
            const maxIter = 50 + Math.floor(normalizedLevel * 50);
            const pixelStep = 3 - Math.floor(normalizedLevel * 1); // Higher detail with audio
            
            for (let px = 0; px < 600; px += pixelStep) {
                for (let py = 0; py < 600; py += pixelStep) {
                    let x0 = (px / 600 - 0.5) * 4 / zoom + centerX;
                    let y0 = (py / 600 - 0.5) * 4 / zoom + centerY;
                    let x = 0, y = 0, iter = 0;
                    
                    while (x * x + y * y <= 4 && iter < maxIter) {
                        const xtemp = x * x - y * y + x0;
                        y = 2 * x * y + y0;
                        x = xtemp;
                        iter++;
                    }
                    
                    // Color intensity responds to audio
                    const hue = (iter / maxIter) * 360 + time * 50;
                    const saturation = 100;
                    const lightness = iter === maxIter ? 0 : 50 + normalizedLevel * 30;
                    ctx.fillStyle = iter === maxIter ? '#000' : `hsl(${hue}, ${saturation}%, ${lightness}%)`;
                    ctx.fillRect(px, py, pixelStep, pixelStep);
                }
            }
            
            requestAnimationFrame(drawMandelbrot);
        }
        drawMandelbrot();
    }
});

function initializeSettingsButton() {
    const settingsButton = document.getElementById('settingsButton');
    if (settingsButton) {
        // Only show settings button in standalone mode
        const isStandalone = window.isStandaloneMode === true;
        
        if (isStandalone) {
            settingsButton.addEventListener('click', () => {
                sendToPlugin('openSettings', 1);
            });
            settingsButton.style.display = 'flex';
        } else {
            // Hide the button in plugin mode (VST3/AU)
            settingsButton.style.display = 'none';
        }
    }
}

function sendToPlugin(parameter, value) {
    if (window.juce) {
        window.juce.postMessage({
            type: parameter === 'openSettings' ? 'openSettings' : 'parameterChange',
            parameter: parameter,
            value: value
        });
    }
}

// Receive audio data from C++ for reactive animations
window.receiveAudioData = function(data) {
    if (!data) return;
    
    state.inputLevel = data.inputLevel || -100;
    
    // Update VU meters
    updateVUMeter('input', state.inputLevel);
    updateVUMeter('output', state.outputLevel);
    state.outputLevel = data.outputLevel || -100;
};

// Update VU meter bar and peak
function updateVUMeter(type, levelDb) {
    const meterFill = document.getElementById(`${type}MeterFill`);
    const meterPeak = document.getElementById(`${type}MeterPeak`);
    const meterLabel = document.getElementById(`${type}Level`);
    
    if (!meterFill || !meterPeak || !meterLabel) return;
    
    // Convert dB to percentage (0 dB = 100%, -60 dB = 0%)
    const minDb = -60;
    const maxDb = 0;
    const normalized = Math.max(0, Math.min(1, (levelDb - minDb) / (maxDb - minDb)));
    const heightPercent = normalized * 100;
    
    meterFill.style.height = `${heightPercent}%`;
    meterLabel.textContent = levelDb > -100 ? `${levelDb.toFixed(1)} dB` : '-∞ dB';
    
    // Update peak hold
    if (levelDb > -60) {
        meterPeak.style.top = `${100 - heightPercent}%`;
        meterPeak.classList.add('visible');
        
        // Reset peak after 1 second
        clearTimeout(meterPeak._peakTimeout);
        meterPeak._peakTimeout = setTimeout(() => {
            meterPeak.classList.remove('visible');
        }, 1000);
    } else {
        meterPeak.classList.remove('visible');
    }
}


function initializeBypassToggle() {
    const bypassButton = document.getElementById('bypassToggle');
    if (!bypassButton) return;
    
    // Set initial state
    state.bypass = false;
    bypassButton.classList.add('active');
    bypassButton.querySelector('.bypass-text').textContent = 'ON';
    
    // Use onclick for direct event handling
    bypassButton.onclick = function() {
        state.bypass = !state.bypass;
        const isActive = !state.bypass;
        
        bypassButton.classList.toggle('active', isActive);
        bypassButton.querySelector('.bypass-text').textContent = isActive ? 'ON' : 'OFF';
        
        sendToPlugin('bypass', state.bypass ? 1.0 : 0.0);
        return false;
    };
}
