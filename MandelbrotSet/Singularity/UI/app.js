/**
 * @fileoverview Singularity - Black Hole Compressor UI Logic
 * @module Singularity App
 */

// ===================================================================
// STATE
// ===================================================================

const state = {
    threshold: -20, // dB
    ratio: 4, // ratio
    attack: 10, // ms
    release: 100, // ms
    makeup: 0, // dB
    bypass: false,
    gainReduction: 0, // dB
    inputLevel: -100,
    outputLevel: -100
};

// ===================================================================
// INITIALIZE CONTROLS
// ===================================================================

document.addEventListener('DOMContentLoaded', () => {
    initializeControls();
    initializeBlackHoleAnimation();
    initializeBypassToggle();
    startMetering();
});

function initializeControls() {
    // Threshold knob (-60 to 0 dB)
    const thresholdKnob = new MandelbrotKnob(document.getElementById('thresholdKnob'), {
        min: -60,
        max: 0,
        value: -20,
        step: 0.1,
        onChange: (value, param) => {
            state.threshold = value;
            document.getElementById('thresholdValue').textContent = `${value.toFixed(1)} dB`;
            sendToPlugin(param, value);
        }
    });

    // Ratio knob (1:1 to 20:1)
    const ratioKnob = new MandelbrotKnob(document.getElementById('ratioKnob'), {
        min: 1,
        max: 20,
        value: 4,
        step: 0.1,
        onChange: (value, param) => {
            state.ratio = value;
            document.getElementById('ratioValue').textContent = `${value.toFixed(1)}:1`;
            sendToPlugin(param, value);
        }
    });

    // Attack knob (0.1 to 100 ms)
    const attackKnob = new MandelbrotKnob(document.getElementById('attackKnob'), {
        min: 0.1,
        max: 100,
        value: 10,
        step: 0.1,
        onChange: (value, param) => {
            state.attack = value;
            document.getElementById('attackValue').textContent = `${value.toFixed(1)} ms`;
            sendToPlugin(param, value);
        }
    });

    // Release knob (10 to 1000 ms)
    const releaseKnob = new MandelbrotKnob(document.getElementById('releaseKnob'), {
        min: 10,
        max: 1000,
        value: 100,
        step: 1,
        onChange: (value, param) => {
            state.release = value;
            document.getElementById('releaseValue').textContent = `${value.toFixed(0)} ms`;
            sendToPlugin(param, value);
        }
    });

    // Makeup gain knob (-12 to +24 dB)
    const makeupKnob = new MandelbrotKnob(document.getElementById('makeupKnob'), {
        min: -12,
        max: 24,
        value: 0,
        step: 0.1,
        onChange: (value, param) => {
            state.makeup = value;
            const sign = value >= 0 ? '+' : '';
            document.getElementById('makeupValue').textContent = `${sign}${value.toFixed(1)} dB`;
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
// BLACK HOLE ANIMATION
// ===================================================================

function initializeBlackHoleAnimation() {
    const canvas = document.getElementById('singularityCanvas');
    if (!canvas) return;
    
    const ctx = canvas.getContext('2d');
    canvas.width = 600;
    canvas.height = 600;
    
    const centerX = canvas.width / 2;
    const centerY = canvas.height / 2;
    const particles = [];
    const particleCount = 150;
    
    // Create particles
    for (let i = 0; i < particleCount; i++) {
        particles.push({
            angle: Math.random() * Math.PI * 2,
            distance: Math.random() * 250 + 50,
            speed: Math.random() * 0.02 + 0.01,
            size: Math.random() * 2 + 1,
            opacity: Math.random() * 0.5 + 0.5,
            spiralSpeed: Math.random() * 0.02 + 0.01
        });
    }
    
    function animate() {
        // Clear with fade effect
        ctx.fillStyle = 'rgba(5, 5, 10, 0.1)';
        ctx.fillRect(0, 0, canvas.width, canvas.height);
        
        // Calculate audio reactivity
        const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
        const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60)); // -60 to 0 dB mapped to 0-1
        const grLevel = Math.abs(state.gainReduction) / 18; // 0-18 dB mapped to 0-1
        
        // Event horizon size responds to input level
        const horizonSize = 80 + normalizedLevel * 40;
        const horizonIntensity = 0.6 + normalizedLevel * 0.4;
        
        // Draw event horizon (purple glow) - responds to audio
        const gradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, horizonSize);
        gradient.addColorStop(0, `rgba(153, 69, 255, ${horizonIntensity})`);
        gradient.addColorStop(0.5, `rgba(153, 69, 255, ${horizonIntensity * 0.5})`);
        gradient.addColorStop(1, 'rgba(153, 69, 255, 0)');
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(centerX, centerY, horizonSize, 0, Math.PI * 2);
        ctx.fill();
        
        // Singularity core pulses with compression
        const coreSize = 20 + grLevel * 15;
        const coreIntensity = 0.8 + grLevel * 0.2;
        const coreGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, coreSize);
        coreGradient.addColorStop(0, `rgba(255, 26, 255, ${coreIntensity})`);
        coreGradient.addColorStop(1, 'rgba(153, 69, 255, 0)');
        ctx.fillStyle = coreGradient;
        ctx.beginPath();
        ctx.arc(centerX, centerY, coreSize, 0, Math.PI * 2);
        ctx.fill();
        
        // Particle speed increases with audio level
        const speedMultiplier = 1 + normalizedLevel * 2;
        
        // Update and draw particles spiraling inward
        particles.forEach(particle => {
            // Spiral toward center - faster with more audio
            particle.distance -= particle.speed * 2 * speedMultiplier;
            particle.angle += particle.spiralSpeed * (1 + normalizedLevel);
            
            // Reset if reached center
            if (particle.distance < 10) {
                particle.distance = 300;
                particle.angle = Math.random() * Math.PI * 2;
            }
            
            // Calculate position
            const x = centerX + Math.cos(particle.angle) * particle.distance;
            const y = centerY + Math.sin(particle.angle) * particle.distance;
            
            // Draw particle with gravitational lensing effect
            const distanceFactor = 1 - (particle.distance / 300);
            const glowSize = particle.size * (1 + distanceFactor * 3) * (1 + normalizedLevel * 0.5);
            
            // Particle glow - brighter with audio
            const particleGradient = ctx.createRadialGradient(x, y, 0, x, y, glowSize);
            const particleOpacity = particle.opacity * distanceFactor * (0.8 + normalizedLevel * 0.2);
            particleGradient.addColorStop(0, `rgba(255, 255, 255, ${particleOpacity})`);
            particleGradient.addColorStop(0.5, `rgba(153, 69, 255, ${particleOpacity * 0.5})`);
            particleGradient.addColorStop(1, 'rgba(153, 69, 255, 0)');
            
            ctx.fillStyle = particleGradient;
            ctx.beginPath();
            ctx.arc(x, y, glowSize, 0, Math.PI * 2);
            ctx.fill();
            
            // Draw trail - more visible with audio
            if (particle.distance > 50) {
                const trailX = centerX + Math.cos(particle.angle - particle.spiralSpeed * 5) * (particle.distance + 5);
                const trailY = centerY + Math.sin(particle.angle - particle.spiralSpeed * 5) * (particle.distance + 5);
                
                const trailOpacity = particle.opacity * 0.3 * distanceFactor * (0.5 + normalizedLevel * 0.5);
                ctx.strokeStyle = `rgba(153, 69, 255, ${trailOpacity})`;
                ctx.lineWidth = particle.size * 0.5;
                ctx.beginPath();
                ctx.moveTo(trailX, trailY);
                ctx.lineTo(x, y);
                ctx.stroke();
            }
        });
        
        requestAnimationFrame(animate);
    }
    
    animate();
}

// ===================================================================
// METERING
// ===================================================================

function startMetering() {
    // Metering is now handled by receiveAudioData() called from C++
    // This function kept for compatibility but no longer needed
}

// ===================================================================
// COMMUNICATION WITH C++
// ===================================================================

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
    state.gainReduction = data.gainReduction || 0;
    
    // Update level displays
    document.getElementById('inputLevel').textContent = 
        state.inputLevel > -100 ? `Input: ${state.inputLevel.toFixed(1)} dB` : 'Input: -∞ dB';
    document.getElementById('outputLevel').textContent = 
        state.outputLevel > -100 ? `Output: ${state.outputLevel.toFixed(1)} dB` : 'Output: -∞ dB';
    
    // Update gain reduction meter
    const grValue = Math.abs(state.gainReduction);
    const grPercent = Math.min((grValue / 18) * 100, 100);
    document.getElementById('gainReductionBar').style.width = `${grPercent}%`;
    document.getElementById('grValue').textContent = `${grValue.toFixed(1)} dB`;
};
