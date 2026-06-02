/**
 * @fileoverview ByteBeat - Quantum Descent Flanger UI Logic
 * @module ByteBeat App
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
    initializeSettingsButton();
    new MandelbrotKnob(document.getElementById('rateKnob'), { min: 0.1, max: 10, value: 0.5, onChange: (v, p) => { state.rate = v; document.getElementById('rateValue').textContent = `${v.toFixed(1)} Hz`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('depthKnob'), { min: 0, max: 100, value: 50, onChange: (v, p) => { state.depth = v; document.getElementById('depthValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('feedbackKnob'), { min: 0, max: 100, value: 30, onChange: (v, p) => { state.feedback = v; document.getElementById('feedbackValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('stereoKnob'), { min: 0, max: 100, value: 100, onChange: (v, p) => { state.stereo = v; document.getElementById('stereoValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('mixKnob'), { min: 0, max: 100, value: 50, onChange: (v, p) => { state.mix = v; document.getElementById('mixValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    initializeBypassToggle();
    
    // Rabbit hole spiral animation - responds to audio
    const canvas = document.getElementById('bytebeatCanvas');
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
    if (typeof window.postMessageToJUCE !== 'function') return;
    window.postMessageToJUCE({
        type: parameter === 'openSettings' ? 'openSettings' : 'parameterChange',
        parameter: parameter,
        value: value
    });
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
            inputMeterFill.style.background = 'linear-gradient(to right, rgba(255, 107, 0, 0.3) 0%, rgba(255, 107, 0, 0.6) 50%, #ff6b00 100%)';
            inputMeterFill.style.boxShadow = '0 0 8px rgba(255, 107, 0, 0.8)';
        }
        if (outputMeterFill) {
            outputMeterFill.style.background = 'linear-gradient(to right, rgba(255, 107, 0, 0.3) 0%, rgba(255, 107, 0, 0.6) 50%, #ff6b00 100%)';
            outputMeterFill.style.boxShadow = '0 0 8px rgba(255, 107, 0, 0.8)';
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
