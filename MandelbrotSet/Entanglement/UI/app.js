/**
 * @fileoverview Entanglement - Quantum Delay UI Logic
 * @module Entanglement App
 */

// ===================================================================
// STATE
// ===================================================================

const state = {
    time: 250, // ms
    feedback: 30, // %
    mix: 50, // %
    damping: 50, // %
    bypass: false,
    inputLevel: -100,
    outputLevel: -100
};

// ===================================================================
// INITIALIZE CONTROLS
// ===================================================================

document.addEventListener('DOMContentLoaded', () => {
    initializeSettingsButton();
    initializeControls();
    initializeEntanglementAnimation();
    initializeBypassToggle();
});

function initializeControls() {
    // Time/Distance knob (10-2000 ms)
    new MandelbrotKnob(document.getElementById('timeKnob'), {
        min: 10,
        max: 2000,
        value: 250,
        step: 1,
        onChange: (value, param) => {
            state.time = value;
            document.getElementById('timeValue').textContent = `${value.toFixed(0)} ms`;
            sendToPlugin(param, value);
        }
    });

    // Feedback/Coupling knob (0-100%)
    new MandelbrotKnob(document.getElementById('feedbackKnob'), {
        min: 0,
        max: 100,
        value: 30,
        step: 0.1,
        onChange: (value, param) => {
            state.feedback = value;
            document.getElementById('feedbackValue').textContent = `${value.toFixed(0)}%`;
            sendToPlugin(param, value);
        }
    });

    // Mix/Coherence knob (0-100%)
    new MandelbrotKnob(document.getElementById('mixKnob'), {
        min: 0,
        max: 100,
        value: 50,
        step: 0.1,
        onChange: (value, param) => {
            state.mix = value;
            document.getElementById('mixValue').textContent = `${value.toFixed(0)}%`;
            sendToPlugin(param, value);
        }
    });

    // Damping/Decay knob (0-100%)
    new MandelbrotKnob(document.getElementById('dampingKnob'), {
        min: 0,
        max: 100,
        value: 50,
        step: 0.1,
        onChange: (value, param) => {
            state.damping = value;
            document.getElementById('dampingValue').textContent = `${value.toFixed(0)}%`;
            sendToPlugin(param, value);
        }
    });
}

function initializeBypassToggle() {
    const bypassButton = document.getElementById('bypassToggle');
    bypassButton.addEventListener('click', () => {
        state.bypass = !state.bypass;
        bypassButton.classList.toggle('active', !state.bypass);
        bypassButton.querySelector('.bypass-text').textContent = state.bypass ? 'OFF' : 'ON';
        sendToPlugin('bypass', state.bypass ? 1 : 0);
    });
}

// ===================================================================
// ENTANGLED PARTICLES ANIMATION
// ===================================================================

function initializeEntanglementAnimation() {
    const canvas = document.getElementById('entanglementCanvas');
    if (!canvas) return;
    
    const ctx = canvas.getContext('2d');
    canvas.width = 600;
    canvas.height = 600;
    
    let time = 0;
    
    function animate() {
        // Calculate audio reactivity
        const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
        const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60)); // -60 to 0 dB mapped to 0-1
        
        // Clear with fade effect
        ctx.fillStyle = 'rgba(13, 13, 21, 0.15)';
        ctx.fillRect(0, 0, 600, 600);
        
        time += 0.03 * (1 + normalizedLevel * 0.5);
        
        // Particle movement amplitude responds to audio
        const movementScale = 80 + normalizedLevel * 40;
        
        // Two entangled particles moving in synchronized patterns
        const x1 = 200 + Math.sin(time) * movementScale;
        const y1 = 300 + Math.cos(time * 1.5) * movementScale;
        const x2 = 400 - Math.sin(time) * movementScale;
        const y2 = 300 - Math.cos(time * 1.5) * movementScale;
        
        // Draw connection line (quantum entanglement) - responds to audio
        const lineOpacity = 0.4 + normalizedLevel * 0.4;
        const lineWidth = 2 + normalizedLevel * 2;
        ctx.strokeStyle = `rgba(255, 0, 128, ${lineOpacity})`;
        ctx.lineWidth = lineWidth;
        ctx.setLineDash([5, 5]);
        ctx.lineDashOffset = -time * 10;
        ctx.beginPath();
        ctx.moveTo(x1, y1);
        ctx.lineTo(x2, y2);
        ctx.stroke();
        ctx.setLineDash([]);
        
        // Particle size responds to audio
        const particleSize = 30 + normalizedLevel * 20;
        const particleIntensity = 0.8 + normalizedLevel * 0.2;
        
        // Draw particle 1 (pink) - responds to audio
        const grad1 = ctx.createRadialGradient(x1, y1, 0, x1, y1, particleSize);
        grad1.addColorStop(0, `rgba(255, 0, 128, ${particleIntensity})`);
        grad1.addColorStop(0.5, `rgba(255, 0, 128, ${particleIntensity * 0.5})`);
        grad1.addColorStop(1, 'rgba(255, 0, 128, 0)');
        ctx.fillStyle = grad1;
        ctx.beginPath();
        ctx.arc(x1, y1, particleSize, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw particle 2 (cyan) - responds to audio
        const grad2 = ctx.createRadialGradient(x2, y2, 0, x2, y2, particleSize);
        grad2.addColorStop(0, `rgba(0, 229, 255, ${particleIntensity})`);
        grad2.addColorStop(0.5, `rgba(0, 229, 255, ${particleIntensity * 0.5})`);
        grad2.addColorStop(1, 'rgba(0, 229, 255, 0)');
        ctx.fillStyle = grad2;
        ctx.beginPath();
        ctx.arc(x2, y2, particleSize, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw cores
        ctx.fillStyle = 'rgba(255, 255, 255, 0.9)';
        ctx.beginPath();
        ctx.arc(x1, y1, 3, 0, Math.PI * 2);
        ctx.arc(x2, y2, 3, 0, Math.PI * 2);
        ctx.fill();
        
        requestAnimationFrame(animate);
    }
    
    animate();
}

// ===================================================================
// COMMUNICATION WITH C++
// ===================================================================

function initializeSettingsButton() {
    const settingsButton = document.getElementById('settingsButton');
    if (settingsButton) {
        settingsButton.addEventListener('click', () => {
            sendToPlugin('openSettings', 1);
        });
        settingsButton.style.display = 'flex';
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
