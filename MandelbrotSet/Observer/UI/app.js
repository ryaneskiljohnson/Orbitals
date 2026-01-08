/**
 * @fileoverview Observer - Wavefunction Collapse Dynamic EQ UI Logic
 * @module Observer App
 */

const state = {
    threshold: -20,
    frequency: 1000,
    ratio: 2,
    attack: 10,
    release: 100,
    bypass: false,
    inputLevel: -100,
    outputLevel: -100
};

document.addEventListener('DOMContentLoaded', () => {
    initializeSettingsButton();
    new MandelbrotKnob(document.getElementById('thresholdKnob'), { min: -60, max: 0, value: -20, onChange: (v, p) => { state.threshold = v; document.getElementById('thresholdValue').textContent = `${v.toFixed(1)} dB`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('freqKnob'), { min: 20, max: 20000, value: 1000, onChange: (v, p) => { state.frequency = v; document.getElementById('freqValue').textContent = v < 1000 ? `${v.toFixed(0)} Hz` : `${(v/1000).toFixed(1)}k Hz`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('ratioKnob'), { min: 1, max: 10, value: 2, onChange: (v, p) => { state.ratio = v; document.getElementById('ratioValue').textContent = `${v.toFixed(1)}:1`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('attackKnob'), { min: 0.1, max: 100, value: 10, onChange: (v, p) => { state.attack = v; document.getElementById('attackValue').textContent = `${v.toFixed(1)} ms`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('releaseKnob'), { min: 10, max: 1000, value: 100, onChange: (v, p) => { state.release = v; document.getElementById('releaseValue').textContent = `${v.toFixed(0)} ms`; sendToPlugin(p, v); } });
    initializeBypassToggle();
    
    // Wavefunction animation - responds to audio
    const canvas = document.getElementById('observerCanvas');
    if (canvas) {
        const ctx = canvas.getContext('2d');
        canvas.width = 600;
        canvas.height = 600;
        let time = 0;
        let collapsed = false;
        let collapsePhase = 0;
        
        function animate() {
            // Calculate audio reactivity
            const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
            const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60));
            
            ctx.fillStyle = 'rgba(13, 13, 21, 0.08)';
            ctx.fillRect(0, 0, 600, 600);
            
            time += 0.02 * (1 + normalizedLevel * 0.5);
            
            // Collapse triggered by audio peaks
            if (normalizedLevel > 0.7 && Math.sin(time) > 0.9) {
                collapsed = !collapsed;
                collapsePhase = 0;
            }
            
            if (collapsed) {
                collapsePhase += 0.1;
                const collapseSize = 50 + normalizedLevel * 30;
                const collapseIntensity = 0.8 + normalizedLevel * 0.2;
                
                // Collapsed state - single particle
                ctx.fillStyle = `rgba(0, 229, 255, ${collapseIntensity})`;
                ctx.beginPath();
                ctx.arc(300, 300, collapseSize, 0, Math.PI * 2);
                ctx.fill();
                
                // Pulsing rings
                for (let i = 0; i < 3; i++) {
                    const ringSize = collapseSize + (collapsePhase * 20 + i * 30) % 150;
                    const ringOpacity = (1 - (ringSize - collapseSize) / 150) * normalizedLevel;
                    ctx.strokeStyle = `rgba(0, 229, 255, ${ringOpacity})`;
                    ctx.lineWidth = 2;
                    ctx.beginPath();
                    ctx.arc(300, 300, ringSize, 0, Math.PI * 2);
                    ctx.stroke();
                }
            } else {
                // Superposition state - multiple possibilities
                const particleCount = 30 + Math.floor(normalizedLevel * 20);
                for (let i = 0; i < particleCount; i++) {
                    const angle = (i / particleCount) * Math.PI * 2 + time;
                    const r = 100 + Math.sin(time * 2 + i) * (30 + normalizedLevel * 30);
                    const x = 300 + Math.cos(angle) * r;
                    const y = 300 + Math.sin(angle) * r;
                    
                    const size = 15 + normalizedLevel * 10;
                    const intensity = 0.5 + normalizedLevel * 0.4;
                    const grad = ctx.createRadialGradient(x, y, 0, x, y, size);
                    grad.addColorStop(0, `rgba(255, 140, 0, ${intensity})`);
                    grad.addColorStop(1, 'rgba(255, 140, 0, 0)');
                    ctx.fillStyle = grad;
                    ctx.beginPath();
                    ctx.arc(x, y, size, 0, Math.PI * 2);
                    ctx.fill();
                }
            }
            
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
