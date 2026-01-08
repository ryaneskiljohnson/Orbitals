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

console.log('🚀🚀🚀 APP.JS LOADING 🚀🚀🚀');

// Test button click globally
window.testBypassButton = function() {
    console.log('🧪 Manual bypass button test...');
    const btn = document.getElementById('bypassToggle');
    console.log('Button element:', btn);
    if (btn) {
        console.log('Triggering click programmatically...');
        btn.click();
    }
};

document.addEventListener('DOMContentLoaded', async () => {
    console.log('🔵🔵🔵 DOMContentLoaded fired 🔵🔵🔵');
    
    // AUTOMATIC BRIDGE TEST - Runs immediately after page loads
    console.log('🧪 Starting automatic bridge test...');
    setTimeout(() => {
        testBridge();
    }, 1000); // Wait 1 second for everything to initialize
    await waitForJuce();
    console.log('🔵 Initializing controls...');
    
    try {
        initializeSettingsButton();
        console.log('✅ Settings button initialized');
    } catch (e) {
        console.error('❌ Settings button error:', e);
    }
    
    try {
        initializeControls();
        console.log('✅ Controls initialized');
    } catch (e) {
        console.error('❌ Controls error:', e);
    }
    
    try {
        initializeFabricAnimation();
        console.log('✅ Animation initialized');
    } catch (e) {
        console.error('❌ Animation error:', e);
    }
    
    try {
        console.log('🔵 ABOUT TO INITIALIZE BYPASS TOGGLE...');
        initializeBypassToggle();
        console.log('✅✅✅ BYPASS TOGGLE INITIALIZED ✅✅✅');
    } catch (e) {
        console.error('❌❌❌ BYPASS TOGGLE ERROR:', e);
        console.error('Stack:', e.stack);
    }
    
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
    console.log('🔵 initializeBypassToggle() STARTED');
    
    const bypassButton = document.getElementById('bypassToggle');
    console.log('🔍 bypassButton element:', bypassButton);
    
    if (!bypassButton) {
        console.error('❌ Button not found!');
        return;
    }
    
    console.log('✅ Button found, setting up...');
    
    // Set initial state
    state.bypass = false;
    bypassButton.classList.add('active');
    bypassButton.querySelector('.bypass-text').textContent = 'ON';
    
    console.log('✅ Initial state set');
    
    // Add click listener
    bypassButton.onclick = function(e) {
        console.log('🖱️ ONCLICK FIRED');
        
        state.bypass = !state.bypass;
        const isActive = !state.bypass;
        
        bypassButton.classList.toggle('active', isActive);
        bypassButton.querySelector('.bypass-text').textContent = isActive ? 'ON' : 'OFF';
        
        console.log('📤 About to call sendToPlugin with bypass =', state.bypass ? 1.0 : 0.0);
        sendToPlugin('bypass', state.bypass ? 1.0 : 0.0);
        console.log('✅ sendToPlugin called');
        
        return false;
    };
    
    console.log('✅ Click listener attached via onclick');
    console.log('🔵 initializeBypassToggle() COMPLETE');
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
    console.log('🎨 Initializing Fabric animation...');
    const canvas = document.getElementById('fabricCanvas');
    if (!canvas) {
        console.error('❌ fabricCanvas element not found!');
        return;
    }
    console.log('✅ Canvas found:', canvas);
    
    const ctx = canvas.getContext('2d');
    if (!ctx) {
        console.error('❌ Could not get 2D context!');
        return;
    }
    canvas.width = 600;
    canvas.height = 600;
    console.log('✅ Canvas configured: 600x600');
    
    const centerX = canvas.width / 2;
    const centerY = canvas.height / 2;
    const gridSize = 40;
    const points = [];
    
    console.log('📐 Creating grid points...');
    // Create grid points
    for (let x = 0; x < canvas.width; x += gridSize) {
        for (let y = 0; y < canvas.height; y += gridSize) {
            points.push({
                x: x,
                y: y,
                baseX: x,
                baseY: y,
                phase: Math.random() * Math.PI * 2,
                velocity: 0
            });
        }
    }
    console.log(`✅ Created ${points.length} grid points`);
    
    let time = 0;
    let ripples = [];
    let lastAudioLevel = 0;
    let peakTriggerThreshold = 0.3;
    
    console.log('🎬 Setting up animation variables...');
    
    function animate() {
        try {
            time += 0.02;
            
            // Debug: Log first few frames
            if (window.frameCount === undefined) window.frameCount = 0;
            if (window.frameCount < 5) {
                console.log(`🎬 Frame ${window.frameCount}: time=${time.toFixed(2)}, inputLevel=${state.inputLevel}`);
            }
            window.frameCount++;
        
        // Calculate audio reactivity
        const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
        const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60)); // -60 to 0 dB mapped to 0-1
        
        // Parameter influence on animation (calculate early so available everywhere)
        const sizeInfluence = state.size / 100; // 0 to 1
        const diffusionInfluence = state.diffusion / 100; // 0 to 1
        const dampingInfluence = state.damping / 100; // 0 to 1
        const mixInfluence = state.mix / 100; // 0 to 1
        const wetdryInfluence = state.wetdry / 100; // 0 to 1
        const timeDilation = 1.0 + (state.predelay / 200.0) * 0.5; // Slower animation with more predelay
        
        // Debug: Log animation state periodically
        if (window.animDebugCounter === undefined) window.animDebugCounter = 0;
        if (++window.animDebugCounter % 120 === 0) {
            console.log(`🎨 Animation: audioLevel=${audioLevel.toFixed(1)}dB, normalized=${normalizedLevel.toFixed(3)}, ripples=${ripples.length}`);
        }
        
        // Detect audio peaks for ripple triggers
        const levelDiff = normalizedLevel - lastAudioLevel;
        if (levelDiff > 0.15 && normalizedLevel > peakTriggerThreshold) {
            // Audio peak detected - create ripple
            console.log(`🌊 RIPPLE TRIGGERED! Level=${normalizedLevel.toFixed(2)}, Diff=${levelDiff.toFixed(2)}`);
            ripples.push({
                x: centerX + (Math.random() - 0.5) * 100,
                y: centerY + (Math.random() - 0.5) * 100,
                radius: 0,
                maxRadius: 300 + normalizedLevel * 200,
                strength: normalizedLevel * 2,
                age: 0
            });
        }
        lastAudioLevel = normalizedLevel;
        
        // Fade effect intensity responds to audio and damping
        // Higher damping = more fade (more trails)
        const fadeIntensity = 0.05 + dampingInfluence * 0.05 + normalizedLevel * 0.03;
        try {
            ctx.fillStyle = `rgba(13, 13, 21, ${fadeIntensity})`;
            ctx.fillRect(0, 0, canvas.width, canvas.height);
        } catch (e) {
            console.error('❌ Error in fillRect:', e);
            return;
        }
        
        // Base warp amount combines audio + size parameter
        const baseWarp = (normalizedLevel * 25 + sizeInfluence * 15) * mixInfluence;
        
        // Wave frequency affected by diffusion
        const waveFrequency = 0.02 + diffusionInfluence * 0.03; // More diffusion = higher frequency
        
        // Update ripples
        try {
            ripples = ripples.filter(ripple => {
                ripple.radius += 4;
                ripple.age += 1;
                return ripple.radius < ripple.maxRadius;
            });
        } catch (e) {
            console.error('❌ Error updating ripples:', e);
        }
        
        // Update grid points with spacetime warping
        try {
            points.forEach(point => {
                const dx = point.baseX - centerX;
                const dy = point.baseY - centerY;
                const dist = Math.sqrt(dx * dx + dy * dy);
                
                // Base wave motion (affected by diffusion parameter)
                let waveOffset = Math.sin(dist * waveFrequency + time / timeDilation + point.phase) * (8 + baseWarp);
                
                // Add ripple effects (spacetime bending)
                let rippleX = 0;
                let rippleY = 0;
                ripples.forEach(ripple => {
                    const rippleDx = point.baseX - ripple.x;
                    const rippleDy = point.baseY - ripple.y;
                    const rippleDist = Math.sqrt(rippleDx * rippleDx + rippleDy * rippleDy);
                    
                    const rippleWave = Math.sin((rippleDist - ripple.radius) * 0.1);
                    const rippleFalloff = Math.exp(-ripple.age * 0.02);
                    const rippleStrength = rippleWave * ripple.strength * 20 * rippleFalloff;
                    
                    if (rippleDist > 0) {
                        rippleX += (rippleDx / rippleDist) * rippleStrength;
                        rippleY += (rippleDy / rippleDist) * rippleStrength;
                    }
                });
                
                // Combine base wave with ripples
                const angle = Math.atan2(dy, dx);
                point.x = point.baseX + Math.cos(angle) * waveOffset + rippleX;
                point.y = point.baseY + Math.sin(angle) * waveOffset + rippleY;
            });
        } catch (e) {
            console.error('❌ Error updating grid points:', e);
            return;
        }
        
        // Draw ripple waves (visibility affected by wet/dry and predelay)
        ripples.forEach(ripple => {
            const rippleOpacity = (1 - ripple.radius / ripple.maxRadius) * ripple.strength * 0.3 * wetdryInfluence;
            const rippleWidth = 2 + sizeInfluence * 1;
            ctx.strokeStyle = `rgba(0, 229, 255, ${rippleOpacity})`;
            ctx.lineWidth = rippleWidth;
            ctx.beginPath();
            ctx.arc(ripple.x, ripple.y, ripple.radius, 0, Math.PI * 2);
            ctx.stroke();
            
            // Inner glow (more visible with higher diffusion)
            const glowOpacity = rippleOpacity * 0.5 * (0.5 + diffusionInfluence * 0.5);
            ctx.strokeStyle = `rgba(102, 255, 255, ${glowOpacity})`;
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.arc(ripple.x, ripple.y, ripple.radius - 5, 0, Math.PI * 2);
            ctx.stroke();
        });
        
        // Draw grid lines - brightness responds to audio and wet/dry
        const lineOpacity = 0.2 + normalizedLevel * 0.4 + wetdryInfluence * 0.2;
        ctx.strokeStyle = `rgba(0, 229, 255, ${lineOpacity})`;
        ctx.lineWidth = 1 + normalizedLevel * 1 + mixInfluence * 0.5;
        
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
        
        // Draw intersection points - size and brightness respond to audio and diffusion
        const pointOpacity = 0.5 + normalizedLevel * 0.3 + diffusionInfluence * 0.2;
        const pointSize = 1.5 + normalizedLevel * 3 + diffusionInfluence * 1.5;
        ctx.fillStyle = `rgba(102, 255, 255, ${pointOpacity})`;
        points.forEach(point => {
            ctx.beginPath();
            ctx.arc(point.x, point.y, pointSize, 0, Math.PI * 2);
            ctx.fill();
        });
        
        // Draw center energy burst during high audio (scaled by wet/dry)
        if (normalizedLevel > 0.4 && wetdryInfluence > 0.3) {
            const burstSize = (normalizedLevel - 0.4) * 80 * wetdryInfluence;
            const burstOpacity = normalizedLevel * wetdryInfluence * 0.5;
            const gradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, burstSize);
            gradient.addColorStop(0, `rgba(0, 229, 255, ${burstOpacity})`);
            gradient.addColorStop(0.5, `rgba(102, 255, 255, ${burstOpacity * 0.5})`);
            gradient.addColorStop(1, 'rgba(0, 229, 255, 0)');
            ctx.fillStyle = gradient;
            ctx.beginPath();
            ctx.arc(centerX, centerY, burstSize, 0, Math.PI * 2);
            ctx.fill();
        }
        
        requestAnimationFrame(animate);
        } catch (error) {
            console.error('❌ FATAL ERROR in animation loop:', error);
            console.error('Stack:', error.stack);
            // Try to continue anyway
            requestAnimationFrame(animate);
        }
    }
    
    console.log('✅ Starting animation loop...');
    animate();
    console.log('✅ Animation started, calling updateSpaceInfo...');
    updateSpaceInfo();
    console.log('✅ Fabric animation fully initialized');
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
    // Special alert for bypass to confirm it's being sent
    if (parameter === 'bypass') {
        alert('sendToPlugin: bypass=' + value);
    }
    
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
        
        if (parameter === 'bypass') {
            alert('Sent to C++: ' + JSON.stringify(message));
        }
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
    
    const oldInputLevel = state.inputLevel;
    state.inputLevel = data.inputLevel || -100;
    state.outputLevel = data.outputLevel || -100;
    
    // Debug: Log audio levels periodically
    if (window.audioDebugCounter === undefined) window.audioDebugCounter = 0;
    if (++window.audioDebugCounter % 30 === 0) {
        console.log(`🎵 Audio levels: Input=${state.inputLevel.toFixed(1)}dB, Output=${state.outputLevel.toFixed(1)}dB`);
    }
    
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
