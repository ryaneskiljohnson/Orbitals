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
    wetdry: 50,
    bypass: false,
    inputLevel: -100,
    outputLevel: -100
};

let juceReady = false;

// Wait for JUCE backend to be available (window.__JUCE__ or window.juce)
function waitForJuce() {
    return new Promise((resolve) => {
        const juceBackend = window.__JUCE__ || window.juce;
        if (juceBackend) {
            juceReady = true;
            console.log('✅ JUCE backend available immediately:', window.__JUCE__ ? 'window.__JUCE__' : 'window.juce');
            resolve();
        } else {
            console.log('⏳ Waiting for JUCE backend (window.__JUCE__ or window.juce)...');
            let attempts = 0;
            const check = setInterval(() => {
                const backend = window.__JUCE__ || window.juce;
                if (backend) {
                    clearInterval(check);
                    juceReady = true;
                    console.log('✅ JUCE backend became available after ' + (attempts * 100) + 'ms');
                    console.log('✅ Using:', window.__JUCE__ ? 'window.__JUCE__' : 'window.juce');
                    resolve();
                } else if (++attempts >= 50) {
                    clearInterval(check);
                    console.error('❌ JUCE backend never became available after 5 seconds');
                    console.error('This means the WebView bridge is not working!');
                    resolve(); // Resolve anyway to not block execution
                }
            }, 100);
        }
    });
}

document.addEventListener('DOMContentLoaded', async () => {
    console.log('🔵 DOMContentLoaded fired');
    
    // AUTOMATIC BRIDGE TEST - Runs immediately after page loads
    console.log('🧪 Starting automatic bridge test...');
    setTimeout(() => {
        testBridge();
    }, 1000); // Wait 1 second for everything to initialize
    await waitForJuce();
    console.log('🔵 Initializing controls...');
    initializeSettingsButton();
    initializeControls();
    initializeFabricAnimation();
    initializeBypassToggle();
    console.log('🔵 All controls initialized');
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

    // Wet/Dry slider
    new MandelbrotSlider(document.getElementById('wetdrySlider'), {
        min: 0,
        max: 100,
        value: 50,
        orientation: 'horizontal',
        onChange: (value, param) => {
            state.wetdry = value;
            // Format display: 0% = 100% Dry, 50% = 50/50, 100% = 100% Wet
            let displayText;
            if (value === 0) {
                displayText = '100% Dry';
            } else if (value === 100) {
                displayText = '100% Wet';
            } else if (value === 50) {
                displayText = '50/50';
            } else {
                displayText = `${value.toFixed(0)}% Wet`;
            }
            document.getElementById('wetdryValue').textContent = displayText;
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
        // Only show settings button in standalone mode
        const isStandalone = window.isStandaloneMode === true;
        
        if (isStandalone) {
            settingsButton.addEventListener('click', () => {
                console.log('Settings button clicked!');
                sendToPlugin('openSettings', 1);
            });
            settingsButton.style.display = 'flex';
            console.log('Settings button initialized and visible');
        } else {
            // Hide the button in plugin mode (VST3/AU)
            settingsButton.style.display = 'none';
        }
    }
}

function sendToPlugin(parameter, value) {
    console.log('📤 sendToPlugin called:', parameter, value);
    
    // JUCE 8 uses window.__JUCE__.backend.emitEvent() for event listeners
    const juceBackend = window.__JUCE__;
    
    if (juceBackend && juceBackend.backend) {
        console.log('✅ JUCE backend found: window.__JUCE__.backend');
        const message = {
            type: parameter === 'openSettings' ? 'openSettings' : 'parameterChange',
            parameter: parameter,
            value: value
        };
        console.log('📤 Sending message via emitEvent:', JSON.stringify(message));
        // Use emitEvent instead of postMessage for withEventListener
        juceBackend.backend.emitEvent('message', message);
    } else if (window.__JUCE__) {
        // Fallback: try postMessage (old API)
        console.log('⚠️ Using fallback postMessage API');
        const message = {
            type: parameter === 'openSettings' ? 'openSettings' : 'parameterChange',
            parameter: parameter,
            value: value
        };
        const messageString = JSON.stringify(message);
        window.__JUCE__.postMessage(messageString);
    } else {
        console.error('❌ window.__JUCE__ is not available!');
        console.error('Available window properties:', Object.keys(window).filter(k => k.includes('JUCE') || k.includes('juce') || k.includes('webkit')));
    }
}

// Test function to verify bridge is working
function testBridge() {
    console.log('🧪 ========================================');
    console.log('🧪 BRIDGE TEST STARTING');
    console.log('🧪 ========================================');
    
    // Test 1: Check if JUCE backend exists
    const juceBackend = window.__JUCE__;
    if (juceBackend && juceBackend.backend) {
        console.log('✅ TEST 1 PASSED: JUCE backend found');
        console.log('   window.__JUCE__:', juceBackend);
        console.log('   window.__JUCE__.backend:', juceBackend.backend);
        console.log('   Available methods:', Object.keys(juceBackend.backend));
    } else {
        console.error('❌ TEST 1 FAILED: No JUCE backend found!');
        console.error('   window.__JUCE__:', window.__JUCE__);
        console.error('   window.__JUCE__.backend:', window.__JUCE__?.backend);
        console.error('   Available properties:', Object.keys(window).filter(k => k.includes('JUCE') || k.includes('juce') || k.includes('webkit')));
        return;
    }
    
    // Test 2: Send a test message using emitEvent
    console.log('🧪 TEST 2: Sending test message to C++ via emitEvent...');
    const testMessage = {
        type: 'test',
        parameter: 'bridgeTest',
        value: 'Hello from JavaScript! Bridge test successful!'
    };
    console.log('   Sending:', JSON.stringify(testMessage));
    
    try {
        if (juceBackend.backend) {
            juceBackend.backend.emitEvent('message', testMessage);
            console.log('✅ TEST 2 PASSED: emitEvent() called without error');
        } else {
            console.error('❌ TEST 2 FAILED: juceBackend.backend is not available');
            return;
        }
    } catch (error) {
        console.error('❌ TEST 2 FAILED: emitEvent() threw error:', error);
        return;
    }
    
    // Test 3: Send a parameter change message
    console.log('🧪 TEST 3: Sending parameter change message...');
    const paramMessage = {
        type: 'parameterChange',
        parameter: 'size',
        value: 75
    };
    console.log('   Sending:', JSON.stringify(paramMessage));
    
    try {
        if (juceBackend.backend) {
            juceBackend.backend.emitEvent('message', paramMessage);
            console.log('✅ TEST 3 PASSED: Parameter change message sent');
        } else {
            console.error('❌ TEST 3 FAILED: juceBackend.backend is not available');
        }
    } catch (error) {
        console.error('❌ TEST 3 FAILED: Parameter change message error:', error);
    }
    
    console.log('🧪 ========================================');
    console.log('🧪 BRIDGE TEST COMPLETE');
    console.log('🧪 Check C++ console for "RECEIVED MESSAGE" logs');
    console.log('🧪 ========================================');
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
