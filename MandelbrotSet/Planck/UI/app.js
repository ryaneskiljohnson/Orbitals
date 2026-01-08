/**
 * @fileoverview Planck - Quantum Scale EQ UI Logic
 * @module Planck App
 */

const state = {
    lowfreq: 100,
    lowgain: 0,
    midfreq: 1000,
    midgain: 0,
    highfreq: 10000,
    highgain: 0,
    q: 1.0,
    bypass: false,
    inputLevel: -100,
    outputLevel: -100
};

document.addEventListener('DOMContentLoaded', () => {
    initializeSettingsButton();
    new MandelbrotKnob(document.getElementById('lowFreqKnob'), { min: 20, max: 500, value: 100, onChange: (v, p) => { state.lowfreq = v; document.getElementById('lowFreqValue').textContent = `${v.toFixed(0)} Hz`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('lowGainKnob'), { min: -12, max: 12, value: 0, onChange: (v, p) => { state.lowgain = v; document.getElementById('lowGainValue').textContent = `${v >= 0 ? '+' : ''}${v.toFixed(1)} dB`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('midFreqKnob'), { min: 200, max: 5000, value: 1000, onChange: (v, p) => { state.midfreq = v; document.getElementById('midFreqValue').textContent = `${(v/1000).toFixed(1)}k Hz`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('midGainKnob'), { min: -12, max: 12, value: 0, onChange: (v, p) => { state.midgain = v; document.getElementById('midGainValue').textContent = `${v >= 0 ? '+' : ''}${v.toFixed(1)} dB`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('highFreqKnob'), { min: 2000, max: 20000, value: 10000, onChange: (v, p) => { state.highfreq = v; document.getElementById('highFreqValue').textContent = `${(v/1000).toFixed(1)}k Hz`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('highGainKnob'), { min: -12, max: 12, value: 0, onChange: (v, p) => { state.highgain = v; document.getElementById('highGainValue').textContent = `${v >= 0 ? '+' : ''}${v.toFixed(1)} dB`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('qKnob'), { min: 0.1, max: 10, value: 1, onChange: (v, p) => { state.q = v; document.getElementById('qValue').textContent = v.toFixed(1); sendToPlugin(p, v); } });
    
    initializeBypassToggle();
    
    // Quantum foam animation - responds to audio
    const canvas = document.getElementById('planckCanvas');
    if (canvas) {
        const ctx = canvas.getContext('2d');
        canvas.width = 800;
        canvas.height = 400;
        const bubbles = Array.from({length: 80}, () => ({ 
            x: Math.random() * 800, 
            y: Math.random() * 400, 
            r: Math.random() * 3 + 1, 
            phase: Math.random() * Math.PI * 2,
            speed: Math.random() * 0.02 + 0.03
        }));
        
        function animate() {
            // Calculate audio reactivity
            const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
            const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60));
            
            ctx.fillStyle = 'rgba(13, 13, 21, 0.1)';
            ctx.fillRect(0, 0, 800, 400);
            
            bubbles.forEach(b => {
                b.phase += b.speed * (1 + normalizedLevel);
                b.r = 2 + Math.sin(b.phase) * (1.5 + normalizedLevel * 2);
                
                const intensity = 0.6 + normalizedLevel * 0.3;
                const grad = ctx.createRadialGradient(b.x, b.y, 0, b.x, b.y, b.r * 3);
                grad.addColorStop(0, `rgba(255, 215, 0, ${intensity})`);
                grad.addColorStop(1, 'rgba(255, 215, 0, 0)');
                ctx.fillStyle = grad;
                ctx.beginPath();
                ctx.arc(b.x, b.y, b.r * 3, 0, Math.PI * 2);
                ctx.fill();
            });
            
            requestAnimationFrame(animate);
        }
        animate();
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
    state.outputLevel = data.outputLevel || -100;
    
    // Update VU meters
    updateVUMeter('input', state.inputLevel);
    updateVUMeter('output', state.outputLevel);
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
