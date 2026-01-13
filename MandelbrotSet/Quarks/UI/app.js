/**
 * @fileoverview Quarks - Color Charge Gate UI Logic
 * @module Quarks App
 */

const state = {
    threshold: -40,
    ratio: 2,
    attack: 5,
    release: 50,
    range: 60,
    bypass: false,
    inputLevel: -100,
    outputLevel: -100
};

document.addEventListener('DOMContentLoaded', () => {
    initializeSettingsButton();
    new MandelbrotKnob(document.getElementById('thresholdKnob'), { min: -80, max: 0, value: -40, onChange: (v, p) => { state.threshold = v; document.getElementById('thresholdValue').textContent = `${v.toFixed(0)} dB`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('ratioKnob'), { min: 1, max: 20, value: 2, onChange: (v, p) => { state.ratio = v; document.getElementById('ratioValue').textContent = `${v.toFixed(1)}:1`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('attackKnob'), { min: 0.1, max: 50, value: 5, onChange: (v, p) => { state.attack = v; document.getElementById('attackValue').textContent = `${v.toFixed(1)} ms`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('releaseKnob'), { min: 10, max: 500, value: 50, onChange: (v, p) => { state.release = v; document.getElementById('releaseValue').textContent = `${v.toFixed(0)} ms`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('rangeKnob'), { min: 0, max: 80, value: 60, onChange: (v, p) => { state.range = v; document.getElementById('rangeValue').textContent = `${v.toFixed(0)} dB`; sendToPlugin(p, v); } });
    initializeBypassToggle();
    
    // Quark confinement animation - responds to audio
    const canvas = document.getElementById('quarksCanvas');
    if (canvas) {
        const ctx = canvas.getContext('2d');
        canvas.width = 600;
        canvas.height = 600;
        let time = 0;
        const quarks = [
            { x: 300, y: 200, color: '#ff0000', angle: 0 }, 
            { x: 400, y: 350, color: '#00ff00', angle: 2.09 }, 
            { x: 200, y: 350, color: '#0000ff', angle: 4.19 }
        ];
        
        function animate() {
            // Calculate audio reactivity
            const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
            const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60));
            
            ctx.fillStyle = 'rgba(13, 13, 21, 0.1)';
            ctx.fillRect(0, 0, 600, 600);
            
            time += 0.02 * (1 + normalizedLevel * 0.5);
            
            // Orbit radius responds to audio
            const orbitRadius = 100 + normalizedLevel * 50;
            
            quarks.forEach((q, i) => {
                q.angle += 0.01 * (1 + normalizedLevel * 0.5);
                q.x = 300 + Math.cos(q.angle) * orbitRadius;
                q.y = 300 + Math.sin(q.angle) * orbitRadius;
            });
            
            // Draw gluon connections - stronger with audio
            const gluonOpacity = 0.2 + normalizedLevel * 0.4;
            const gluonWidth = 2 + normalizedLevel * 2;
            ctx.strokeStyle = `rgba(255, 215, 0, ${gluonOpacity})`;
            ctx.lineWidth = gluonWidth;
            for (let i = 0; i < 3; i++) {
                for (let j = i + 1; j < 3; j++) {
                    ctx.beginPath();
                    ctx.moveTo(quarks[i].x, quarks[i].y);
                    ctx.lineTo(quarks[j].x, quarks[j].y);
                    ctx.stroke();
                }
            }
            
            // Draw quarks - size responds to audio
            quarks.forEach(q => {
                const quarkSize = 15 + normalizedLevel * 10;
                const quarkIntensity = 0.8 + normalizedLevel * 0.2;
                const grad = ctx.createRadialGradient(q.x, q.y, 0, q.x, q.y, quarkSize);
                grad.addColorStop(0, q.color.replace(')', `, ${quarkIntensity})`).replace('#', 'rgba('));
                grad.addColorStop(1, q.color.replace(')', ', 0)').replace('#', 'rgba('));
                ctx.fillStyle = grad;
                ctx.beginPath();
                ctx.arc(q.x, q.y, quarkSize, 0, Math.PI * 2);
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
    
    // Update VU meters
    updateVUMeter('input', state.inputLevel);
    updateVUMeter('output', state.outputLevel);
    state.outputLevel = data.outputLevel || -100;
};

// Update VU meter bar and peak
function updateVUMeter(type, levelDb) {
        // Update meter colors based on bypass state
    updateMeterColors();

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



function updateMeterColors() {
    const inputMeterFill = document.getElementById('inputMeterFill');
    const outputMeterFill = document.getElementById('outputMeterFill');
    
    if (state.bypass) {
        // Grey when bypassed
        if (inputMeterFill) {
            inputMeterFill.style.background = 'linear-gradient(to right, rgba(128, 128, 128, 0.3) 0%, rgba(128, 128, 128, 0.6) 50%, #808080 100%)';
            inputMeterFill.style.boxShadow = '0 0 8px rgba(128, 128, 128, 0.8)';
        }
        if (outputMeterFill) {
            outputMeterFill.style.background = 'linear-gradient(to right, rgba(128, 128, 128, 0.3) 0%, rgba(128, 128, 128, 0.6) 50%, #808080 100%)';
            outputMeterFill.style.boxShadow = '0 0 8px rgba(128, 128, 128, 0.8)';
        }
    } else {
        // Theme color when active
        if (inputMeterFill) {
            inputMeterFill.style.background = 'linear-gradient(to right, rgba(0, 255, 136, 0.3) 0%, rgba(0, 255, 136, 0.6) 50%, #00ff88 100%)';
            inputMeterFill.style.boxShadow = '0 0 8px rgba(0, 255, 136, 0.8)';
        }
        if (outputMeterFill) {
            outputMeterFill.style.background = 'linear-gradient(to right, rgba(0, 255, 136, 0.3) 0%, rgba(0, 255, 136, 0.6) 50%, #00ff88 100%)';
            outputMeterFill.style.boxShadow = '0 0 8px rgba(0, 255, 136, 0.8)';
        }
    }
}

function initializeBypassToggle() {
    const bypassButton = document.getElementById('bypassToggle');
    if (!bypassButton) return;
    
    // Set initial state
    state.bypass = false;
    bypassButton.classList.add('active');
    bypassButton.querySelector('.bypass-text').textContent = 'ON';
    updateMeterColors();
    
    // Use onclick for direct event handling
    bypassButton.onclick = function() {
        state.bypass = !state.bypass;
        updateMeterColors();
        const isActive = !state.bypass;
        
        bypassButton.classList.toggle('active', isActive);
        bypassButton.querySelector('.bypass-text').textContent = isActive ? 'ON' : 'OFF';
        
        sendToPlugin('bypass', state.bypass ? 1.0 : 0.0);
        return false;
    };
}
