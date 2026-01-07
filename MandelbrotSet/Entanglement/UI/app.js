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
    let particlePairs = [];
    
    // Create initial particle pairs
    for (let i = 0; i < 5; i++) {
        particlePairs.push({
            angle: (Math.PI * 2 * i) / 5,
            speed: 0.02 + Math.random() * 0.01,
            phase: Math.random() * Math.PI * 2
        });
    }
    
    function animate() {
        // Calculate audio reactivity
        const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
        const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60)); // -60 to 0 dB mapped to 0-1
        
        // Parameter influence
        const timeInfluence = state.time / 2000; // 0 to 1 (delay time)
        const feedbackInfluence = state.feedback / 100; // 0 to 1
        const mixInfluence = state.mix / 100; // 0 to 1
        const dampingInfluence = state.damping / 100; // 0 to 1
        
        // Clear with fade effect (damping controls trails)
        const fadeAmount = 0.1 + dampingInfluence * 0.1;
        ctx.fillStyle = `rgba(13, 13, 21, ${fadeAmount})`;
        ctx.fillRect(0, 0, 600, 600);
        
        // Time controls animation speed
        time += 0.02 * (1 + timeInfluence * 0.5) * (1 + normalizedLevel * 0.5);
        
        // Particle movement amplitude responds to audio and mix
        const movementScale = (60 + normalizedLevel * 30) * (0.5 + mixInfluence * 0.5);
        
        const centerX = 300;
        const centerY = 300;
        
        // Draw all entangled particle pairs
        particlePairs.forEach((pair, index) => {
            pair.angle += pair.speed;
            
            // Distance between pairs controlled by TIME parameter
            const separationDist = 100 + timeInfluence * 150;
            
            // Calculate positions (mirror particles)
            const x1 = centerX + Math.cos(pair.angle + time) * movementScale;
            const y1 = centerY + Math.sin(pair.angle + time) * movementScale;
            const x2 = centerX - Math.cos(pair.angle + time) * movementScale;
            const y2 = centerY - Math.sin(pair.angle + time) * movementScale;
            
            // Draw connection line (quantum entanglement) - FEEDBACK controls brightness
            const lineOpacity = (0.3 + feedbackInfluence * 0.4 + normalizedLevel * 0.3) * mixInfluence;
            const lineWidth = 1 + feedbackInfluence * 2 + normalizedLevel * 1;
            ctx.strokeStyle = `rgba(255, 0, 128, ${lineOpacity})`;
            ctx.lineWidth = lineWidth;
            ctx.setLineDash([5, 5]);
            ctx.lineDashOffset = -time * 10;
            ctx.beginPath();
            ctx.moveTo(x1, y1);
            ctx.lineTo(x2, y2);
            ctx.stroke();
            ctx.setLineDash([]);
            
            // Particle size responds to audio and mix
            const particleSize = (20 + normalizedLevel * 15) * (0.7 + mixInfluence * 0.3);
            const particleIntensity = (0.6 + normalizedLevel * 0.3) * mixInfluence;
            
            // Draw particle 1 (pink)
            const grad1 = ctx.createRadialGradient(x1, y1, 0, x1, y1, particleSize);
            grad1.addColorStop(0, `rgba(255, 0, 128, ${particleIntensity})`);
            grad1.addColorStop(0.5, `rgba(255, 0, 128, ${particleIntensity * 0.5})`);
            grad1.addColorStop(1, 'rgba(255, 0, 128, 0)');
            ctx.fillStyle = grad1;
            ctx.beginPath();
            ctx.arc(x1, y1, particleSize, 0, Math.PI * 2);
            ctx.fill();
            
            // Draw particle 2 (cyan)
            const grad2 = ctx.createRadialGradient(x2, y2, 0, x2, y2, particleSize);
            grad2.addColorStop(0, `rgba(0, 229, 255, ${particleIntensity})`);
            grad2.addColorStop(0.5, `rgba(0, 229, 255, ${particleIntensity * 0.5})`);
            grad2.addColorStop(1, 'rgba(0, 229, 255, 0)');
            ctx.fillStyle = grad2;
            ctx.beginPath();
            ctx.arc(x2, y2, particleSize, 0, Math.PI * 2);
            ctx.fill();
            
            // Draw cores (brighter with audio)
            const coreOpacity = 0.8 + normalizedLevel * 0.2;
            ctx.fillStyle = `rgba(255, 255, 255, ${coreOpacity})`;
            ctx.beginPath();
            ctx.arc(x1, y1, 3, 0, Math.PI * 2);
            ctx.arc(x2, y2, 3, 0, Math.PI * 2);
            ctx.fill();
        });
        
        requestAnimationFrame(animate);
    }
    
    animate();
}

// Receive audio data from C++
window.receiveAudioData = function(data) {
    if (!data) return;
    state.inputLevel = data.inputLevel || -100;
    state.outputLevel = data.outputLevel || -100;
};

// ===================================================================
// COMMUNICATION WITH C++
// ===================================================================

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
