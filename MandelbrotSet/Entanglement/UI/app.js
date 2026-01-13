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
    wetdry: 50, // %
    highpass: 20, // Hz
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

    // Wet/Dry knob (0-100%)
    new MandelbrotKnob(document.getElementById('wetdryKnob'), {
        min: 0,
        max: 100,
        value: 50,
        step: 0.1,
        onChange: (value, param) => {
            state.wetdry = value;
            const displayValue = `${value.toFixed(0)}%`;
            document.getElementById('wetdryValue').textContent = displayValue;
            sendToPlugin(param, value);
        }
    });

    // Highpass knob (20-20000 Hz)
    new MandelbrotKnob(document.getElementById('highpassKnob'), {
        min: 20,
        max: 20000,
        value: 20,
        step: 1,
        onChange: (value, param) => {
            state.highpass = value;
            let displayValue;
            if (value >= 1000) {
                displayValue = `${(value / 1000).toFixed(1)} kHz`;
            } else {
                displayValue = `${value.toFixed(0)} Hz`;
            }
            document.getElementById('highpassValue').textContent = displayValue;
            sendToPlugin(param, value);
        }
    });
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
        const isActive = !state.bypass;
        
        bypassButton.classList.toggle('active', isActive);
        bypassButton.querySelector('.bypass-text').textContent = isActive ? 'ON' : 'OFF';
        
        updateMeterColors();
        sendToPlugin('bypass', state.bypass ? 1.0 : 0.0);
        return false;
    };
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
            inputMeterFill.style.background = 'linear-gradient(to right, rgba(255, 0, 128, 0.3) 0%, rgba(255, 0, 128, 0.6) 50%, #ff0080 100%)';
            inputMeterFill.style.boxShadow = '0 0 8px rgba(255, 0, 128, 0.8)';
        }
        if (outputMeterFill) {
            outputMeterFill.style.background = 'linear-gradient(to right, rgba(255, 0, 128, 0.3) 0%, rgba(255, 0, 128, 0.6) 50%, #ff0080 100%)';
            outputMeterFill.style.boxShadow = '0 0 8px rgba(255, 0, 128, 0.8)';
        }
    }
}

// ===================================================================
// ENTANGLED PARTICLES ANIMATION
// ===================================================================

function initializeEntanglementAnimation() {
    const canvas = document.getElementById('entanglementCanvas');
    if (!canvas) return;
    
    const ctx = canvas.getContext('2d');
    canvas.width = 1200;
    canvas.height = 1200;
    
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
        
        // Parameter influence - each uniquely affects animation
        const timeInfluence = state.time / 2000; // 0 to 1 (delay time) - affects rotation speed and orbit size
        const feedbackInfluence = state.feedback / 100; // 0 to 1 - affects line brightness and width
        const mixInfluence = state.mix / 100; // 0 to 1 (coherence) - affects particle size and intensity
        const dampingInfluence = state.damping / 100; // 0 to 1 (decay) - affects separation distance and trail fade
        const highpassInfluence = state.highpass / 20000; // 0 to 1 - affects line dash pattern
        const wetDryInfluence = state.wetdry / 100; // 0 to 1 - affects grayscale amount
        
        // WET/DRY controls grayscale (0% = full color, 100% = full grayscale)
        const grayscaleAmount = wetDryInfluence;
        
        // Apply grayscale function
        const applyGrayscale = (r, g, b, intensity) => {
            if (grayscaleAmount > 0) {
                const gray = r * 0.299 + g * 0.587 + b * 0.114;
                const r2 = r + (gray - r) * grayscaleAmount;
                const g2 = g + (gray - g) * grayscaleAmount;
                const b2 = b + (gray - b) * grayscaleAmount;
                return `rgba(${Math.round(r2)}, ${Math.round(g2)}, ${Math.round(b2)}, ${intensity})`;
            }
            return `rgba(${r}, ${g}, ${b}, ${intensity})`;
        };
        
        // Clear with fade effect - DAMPING controls trail fade (decay)
        const fadeAmount = 0.05 + dampingInfluence * 0.15;
        ctx.fillStyle = `rgba(13, 13, 21, ${fadeAmount})`;
        ctx.fillRect(0, 0, 1200, 1200);
        
        // TIME controls animation speed (more delay time = slower rotation)
        time += 0.02 * (1.0 - timeInfluence * 0.4) * (1 + normalizedLevel * 0.3);
        
        // TIME also affects orbit radius (distance = larger orbits)
        const baseOrbitRadius = 150 + normalizedLevel * 75;
        const movementScale = baseOrbitRadius * (1.0 + timeInfluence * 0.8);
        
        const centerX = canvas.width / 2;
        const centerY = canvas.height / 2;
        
        // Draw all entangled particle pairs
        particlePairs.forEach((pair, index) => {
            // TIME affects rotation speed
            pair.angle += pair.speed * (1.0 - timeInfluence * 0.3);
            
            // DAMPING controls separation distance (decay = particles spread apart)
            const separationDist = 150 + dampingInfluence * 250;
            
            // Calculate positions (mirror particles)
            const x1 = centerX + Math.cos(pair.angle + time) * movementScale;
            const y1 = centerY + Math.sin(pair.angle + time) * movementScale;
            const x2 = centerX - Math.cos(pair.angle + time) * movementScale;
            const y2 = centerY - Math.sin(pair.angle + time) * movementScale;
            
            // FEEDBACK controls line brightness and width (coupling strength)
            const lineOpacity = (0.2 + feedbackInfluence * 0.6 + normalizedLevel * 0.2);
            const lineWidth = 1 + feedbackInfluence * 3 + normalizedLevel * 1;
            
            // Apply grayscale to connection line based on WET/DRY
            const lineColor = applyGrayscale(255, 0, 128, lineOpacity);
            ctx.strokeStyle = lineColor;
            ctx.lineWidth = lineWidth;
            
            // HIGHPASS affects line dash pattern (higher cutoff = more solid, lower = more dashed)
            const dashLength = 10 - highpassInfluence * 8; // 10px to 2px dashes
            ctx.setLineDash([dashLength, dashLength]);
            ctx.lineDashOffset = -time * 10;
            ctx.beginPath();
            ctx.moveTo(x1, y1);
            ctx.lineTo(x2, y2);
            ctx.stroke();
            ctx.setLineDash([]);
            
            // COHERENCE (mix) controls particle size and intensity
            const particleSize = (30 + normalizedLevel * 20) * (0.5 + mixInfluence * 0.5);
            const particleIntensity = (0.5 + normalizedLevel * 0.3) * (0.5 + mixInfluence * 0.5);
            
            // Draw particle 1 (pink) - with grayscale applied
            const grad1 = ctx.createRadialGradient(x1, y1, 0, x1, y1, particleSize);
            const color1_0 = applyGrayscale(255, 0, 128, particleIntensity);
            const color1_05 = applyGrayscale(255, 0, 128, particleIntensity * 0.5);
            const color1_1 = applyGrayscale(255, 0, 128, 0);
            grad1.addColorStop(0, color1_0);
            grad1.addColorStop(0.5, color1_05);
            grad1.addColorStop(1, color1_1);
            ctx.fillStyle = grad1;
            ctx.beginPath();
            ctx.arc(x1, y1, particleSize, 0, Math.PI * 2);
            ctx.fill();
            
            // Draw particle 2 (cyan) - with grayscale applied
            const grad2 = ctx.createRadialGradient(x2, y2, 0, x2, y2, particleSize);
            const color2_0 = applyGrayscale(0, 229, 255, particleIntensity);
            const color2_05 = applyGrayscale(0, 229, 255, particleIntensity * 0.5);
            const color2_1 = applyGrayscale(0, 229, 255, 0);
            grad2.addColorStop(0, color2_0);
            grad2.addColorStop(0.5, color2_05);
            grad2.addColorStop(1, color2_1);
            ctx.fillStyle = grad2;
            ctx.beginPath();
            ctx.arc(x2, y2, particleSize, 0, Math.PI * 2);
            ctx.fill();
            
            // Draw cores (brighter with audio) - apply grayscale
            const coreOpacity = 0.8 + normalizedLevel * 0.2;
            const coreColor = applyGrayscale(255, 255, 255, coreOpacity);
            ctx.fillStyle = coreColor;
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
    
    // Update meter colors based on bypass state
    updateMeterColors();
    
    // Convert dB to percentage (0 dB = 100%, -60 dB = 0%)
    const minDb = -60;
    const maxDb = 0;
    const normalized = Math.max(0, Math.min(1, (levelDb - minDb) / (maxDb - minDb)));
    const widthPercent = normalized * 100;
    
    meterFill.style.width = `${widthPercent}%`;
    meterLabel.textContent = levelDb > -100 ? `${levelDb.toFixed(1)} dB` : '-∞ dB';
    
    // Update peak hold
    if (levelDb > -60) {
        meterPeak.style.left = `${widthPercent}%`;
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
