/**
 * @fileoverview Fabric - Spacetime Reverb UI Logic
 * @module Fabric App
 */

const state = {
    size: 50,
    diffusion: 50,
    damping: 50,
    predelay: 0,
    mix: 50,
    bypass: false,
    inputLevel: -100,
    outputLevel: -100
};

document.addEventListener('DOMContentLoaded', () => {
    initializeSettingsButton();
    initializeControls();
    initializeFabricAnimation();
    initializeBypassToggle();
});

function initializeControls() {
    // Size/Weave knob
    new MandelbrotKnob(document.getElementById('sizeKnob'), {
        min: 0,
        max: 100,
        value: 50,
        onChange: (value, param) => {
            state.size = value;
            document.getElementById('sizeValue').textContent = `${value.toFixed(0)}%`;
            updateSpaceInfo();
            sendToPlugin(param, value);
        }
    });

    // Diffusion knob
    new MandelbrotKnob(document.getElementById('diffusionKnob'), {
        min: 0,
        max: 100,
        value: 50,
        onChange: (value, param) => {
            state.diffusion = value;
            document.getElementById('diffusionValue').textContent = `${value.toFixed(0)}%`;
            sendToPlugin(param, value);
        }
    });

    // Damping knob
    new MandelbrotKnob(document.getElementById('dampingKnob'), {
        min: 0,
        max: 100,
        value: 50,
        onChange: (value, param) => {
            state.damping = value;
            document.getElementById('dampingValue').textContent = `${value.toFixed(0)}%`;
            sendToPlugin(param, value);
        }
    });

    // Predelay knob
    new MandelbrotKnob(document.getElementById('predelayKnob'), {
        min: 0,
        max: 200,
        value: 0,
        onChange: (value, param) => {
            state.predelay = value;
            document.getElementById('predelayValue').textContent = `${value.toFixed(0)} ms`;
            sendToPlugin(param, value);
        }
    });

    // Mix slider
    new MandelbrotSlider(document.getElementById('mixSlider'), {
        min: 0,
        max: 100,
        value: 50,
        orientation: 'horizontal',
        onChange: (value, param) => {
            state.mix = value;
            document.getElementById('mixValue').textContent = `${value.toFixed(0)}%`;
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

function updateSpaceInfo() {
    const size = state.size;
    let spaceType = 'Tiny';
    if (size > 75) spaceType = 'Cathedral';
    else if (size > 50) spaceType = 'Large';
    else if (size > 25) spaceType = 'Medium';
    else spaceType = 'Small';
    
    document.getElementById('spaceInfo').textContent = `Space: ${spaceType}`;
    
    const decayTime = 0.5 + (size / 100) * 5;
    document.getElementById('reverbTime').textContent = `Decay: ${decayTime.toFixed(1)}s`;
}

function initializeFabricAnimation() {
    const canvas = document.getElementById('fabricCanvas');
    if (!canvas) return;
    
    const ctx = canvas.getContext('2d');
    canvas.width = 700;
    canvas.height = 700;
    
    const centerX = canvas.width / 2;
    const centerY = canvas.height / 2;
    const gridSize = 40;
    const points = [];
    
    // Create grid points
    for (let x = 0; x < canvas.width; x += gridSize) {
        for (let y = 0; y < canvas.height; y += gridSize) {
            points.push({
                x: x,
                y: y,
                baseX: x,
                baseY: y,
                phase: Math.random() * Math.PI * 2
            });
        }
    }
    
    let time = 0;
    
    function animate() {
        time += 0.02;
        
        // Calculate audio reactivity
        const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
        const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60)); // -60 to 0 dB mapped to 0-1
        
        // Fade effect intensity responds to audio
        const fadeIntensity = 0.1 + normalizedLevel * 0.05;
        ctx.fillStyle = `rgba(13, 13, 21, ${fadeIntensity})`;
        ctx.fillRect(0, 0, canvas.width, canvas.height);
        
        // Warp amount responds to audio
        const warpMultiplier = 1 + normalizedLevel * 1.5;
        
        // Update and draw grid with warping
        points.forEach(point => {
            const dx = point.baseX - centerX;
            const dy = point.baseY - centerY;
            const dist = Math.sqrt(dx * dx + dy * dy);
            const waveOffset = Math.sin(dist * 0.02 + time + point.phase) * 15 * warpMultiplier;
            
            point.x = point.baseX + Math.cos(Math.atan2(dy, dx)) * waveOffset;
            point.y = point.baseY + Math.sin(Math.atan2(dy, dx)) * waveOffset;
        });
        
        // Draw grid lines - brightness responds to audio
        const lineOpacity = 0.3 + normalizedLevel * 0.4;
        ctx.strokeStyle = `rgba(0, 229, 255, ${lineOpacity})`;
        ctx.lineWidth = 1 + normalizedLevel * 0.5;
        
        // Horizontal lines
        for (let y = 0; y < canvas.height; y += gridSize) {
            ctx.beginPath();
            const rowPoints = points.filter(p => Math.abs(p.baseY - y) < 1);
            rowPoints.sort((a, b) => a.baseX - b.baseX);
            rowPoints.forEach((p, i) => {
                if (i === 0) ctx.moveTo(p.x, p.y);
                else ctx.lineTo(p.x, p.y);
            });
            ctx.stroke();
        }
        
        // Vertical lines
        for (let x = 0; x < canvas.width; x += gridSize) {
            ctx.beginPath();
            const colPoints = points.filter(p => Math.abs(p.baseX - x) < 1);
            colPoints.sort((a, b) => a.baseY - b.baseY);
            colPoints.forEach((p, i) => {
                if (i === 0) ctx.moveTo(p.x, p.y);
                else ctx.lineTo(p.x, p.y);
            });
            ctx.stroke();
        }
        
        // Draw intersection points - size and brightness respond to audio
        const pointOpacity = 0.6 + normalizedLevel * 0.3;
        const pointSize = 2 + normalizedLevel * 2;
        ctx.fillStyle = `rgba(102, 255, 255, ${pointOpacity})`;
        points.forEach(point => {
            ctx.beginPath();
            ctx.arc(point.x, point.y, pointSize, 0, Math.PI * 2);
            ctx.fill();
        });
        
        requestAnimationFrame(animate);
    }
    
    animate();
    updateSpaceInfo();
}

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
