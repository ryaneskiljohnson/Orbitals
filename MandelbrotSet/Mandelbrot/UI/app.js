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
    
    // Mandelbrot fractal animation - continuous zoom into fractal boundaries
    const canvas = document.getElementById('mandelbrotCanvas');
    if (canvas) {
        const ctx = canvas.getContext('2d');
        canvas.width = 600;
        canvas.height = 600;
        
        // Zoom state
        let zoom = 1;
        let centerX = -0.7;
        let centerY = 0.0;
        let targetX = -0.7;
        let targetY = 0.0;
        let time = 0;
        
        // Interesting zoom targets (areas with detail on the Mandelbrot boundary)
        const zoomTargets = [
            { x: -0.7, y: 0.0 },           // Main cardioid
            { x: -0.75, y: 0.1 },          // Upper tendril
            { x: -0.16, y: 1.04 },         // Elephant valley
            { x: -0.7269, y: 0.1889 },     // Seahorse valley
            { x: 0.285, y: 0.01 },         // Mini-brot
            { x: -0.8, y: 0.156 },         // Spiral area
        ];
        let currentTargetIndex = 0;
        
        function drawMandelbrot() {
            try {
                // Calculate audio reactivity
                const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
                const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60));
                
                // Parameter influences
                const rateInfluence = state.rate / 10; // 0.1 to 10 Hz -> 0.01 to 1
                const depthInfluence = state.depth / 100; // 0 to 100% -> 0 to 1
                const feedbackInfluence = state.feedback / 100;
                const stagesInfluence = state.stages / 12; // 2 to 12 -> ~0.17 to 1
                const mixInfluence = state.mix / 100;
                
                time += 0.01 * (0.5 + rateInfluence * 0.5);
                
                // Continuous zoom in - rate parameter controls zoom speed
                // Smoother zoom acceleration
                const baseZoomSpeed = 1.01 + rateInfluence * 0.02 + normalizedLevel * 0.005;
                zoom *= baseZoomSpeed;
                
                // Gradually move toward target - smoother interpolation
                centerX += (targetX - centerX) * 0.01;
                centerY += (targetY - centerY) * 0.01;
                
                // Switch to next target when zoom gets deep
                if (zoom > 1000 + depthInfluence * 9000) {
                    zoom = 1;
                    currentTargetIndex = (currentTargetIndex + 1) % zoomTargets.length;
                    targetX = zoomTargets[currentTargetIndex].x;
                    targetY = zoomTargets[currentTargetIndex].y;
                    centerX = targetX;
                    centerY = targetY;
                }
                
                // Iteration count affects detail - more stages = more iterations
                // Capped to maintain performance
                const maxIter = 60 + Math.floor(stagesInfluence * 80) + Math.floor(normalizedLevel * 20);
                
                // Pixel step for performance - feedback affects resolution
                // Fixed at 2px for smooth animation, only drop to 1px with very high feedback
                const pixelStep = feedbackInfluence > 0.9 ? 1 : 2;
                
                // Render Mandelbrot set
                for (let px = 0; px < canvas.width; px += pixelStep) {
                    for (let py = 0; py < canvas.height; py += pixelStep) {
                        let x0 = (px / canvas.width - 0.5) * 3.5 / zoom + centerX;
                        let y0 = (py / canvas.height - 0.5) * 3.5 / zoom + centerY;
                        let x = 0, y = 0, iter = 0;
                        
                        while (x * x + y * y <= 4 && iter < maxIter) {
                            const xtemp = x * x - y * y + x0;
                            y = 2 * x * y + y0;
                            x = xtemp;
                            iter++;
                        }
                        
                        // Color scheme - purple fractal colors with audio-reactive brightness
                        if (iter === maxIter) {
                            // Inside the set - black
                            ctx.fillStyle = '#000';
                        } else {
                            // Outside the set - colorful boundaries
                            // Mix parameter affects color saturation
                            const normalizedIter = iter / maxIter;
                            const hue = 270 + normalizedIter * 90 + time * 20; // Purple to magenta range
                            const saturation = 70 + mixInfluence * 30;
                            const lightness = 30 + normalizedIter * 40 + normalizedLevel * 20;
                            ctx.fillStyle = `hsl(${hue}, ${saturation}%, ${lightness}%)`;
                        }
                        
                        ctx.fillRect(px, py, pixelStep, pixelStep);
                    }
                }
                
                // Draw zoom indicator (shows current zoom depth)
                ctx.fillStyle = 'rgba(255, 255, 255, 0.8)';
                ctx.font = '12px monospace';
                const zoomText = `Zoom: ${zoom.toExponential(2)}x`;
                ctx.fillText(zoomText, 10, canvas.height - 10);
                
                requestAnimationFrame(drawMandelbrot);
            } catch (error) {
                console.error('❌ Mandelbrot animation error:', error);
                requestAnimationFrame(drawMandelbrot);
            }
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
            inputMeterFill.style.background = 'linear-gradient(to right, rgba(255, 0, 255, 0.3) 0%, rgba(255, 0, 255, 0.6) 50%, #ff00ff 100%)';
            inputMeterFill.style.boxShadow = '0 0 8px rgba(255, 0, 255, 0.8)';
        }
        if (outputMeterFill) {
            outputMeterFill.style.background = 'linear-gradient(to right, rgba(255, 0, 255, 0.3) 0%, rgba(255, 0, 255, 0.6) 50%, #ff00ff 100%)';
            outputMeterFill.style.boxShadow = '0 0 8px rgba(255, 0, 255, 0.8)';
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
