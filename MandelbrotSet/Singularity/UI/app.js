/**
 * @fileoverview Singularity - Black Hole Compressor UI Logic
 * @module Singularity App
 */

const state = {
    threshold: -20, // dB
    ratio: 4, // ratio
    attack: 10, // ms
    release: 100, // ms
    makeup: 0, // dB
    mix: 100, // %
    bypass: false,
    gainReduction: 0, // dB
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

document.addEventListener('DOMContentLoaded', async () => {
    console.log('🔵🔵🔵 DOMContentLoaded fired 🔵🔵🔵');
    
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
        initializeBlackHoleAnimation();
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

function initializeControls() {
    // Threshold knob (-60 to 0 dB)
    new MandelbrotKnob(document.getElementById('thresholdKnob'), {
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
    new MandelbrotKnob(document.getElementById('ratioKnob'), {
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
    new MandelbrotKnob(document.getElementById('attackKnob'), {
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
    new MandelbrotKnob(document.getElementById('releaseKnob'), {
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
    new MandelbrotKnob(document.getElementById('makeupKnob'), {
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

    // Mix knob (0 to 100%)
    new MandelbrotKnob(document.getElementById('mixKnob'), {
        min: 0,
        max: 100,
        value: 100,
        step: 0.1,
        onChange: (value, param) => {
            state.mix = value;
            document.getElementById('mixValue').textContent = `${value.toFixed(0)}%`;
            sendToPlugin(param, value);
        }
    });
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
            inputMeterFill.style.background = 'linear-gradient(to right, rgba(139, 61, 255, 0.3) 0%, rgba(139, 61, 255, 0.6) 50%, #8b3dff 100%)';
            inputMeterFill.style.boxShadow = '0 0 8px rgba(139, 61, 255, 0.8)';
        }
        if (outputMeterFill) {
            outputMeterFill.style.background = 'linear-gradient(to right, rgba(139, 61, 255, 0.3) 0%, rgba(139, 61, 255, 0.6) 50%, #8b3dff 100%)';
            outputMeterFill.style.boxShadow = '0 0 8px rgba(139, 61, 255, 0.8)';
        }
    }
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
    updateMeterColors();
    
    console.log('✅ Initial state set');
    
    // Add click listener
    bypassButton.onclick = function(e) {
        console.log('🖱️ ONCLICK FIRED');
        
        state.bypass = !state.bypass;
        updateMeterColors();
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

function initializeBlackHoleAnimation() {
    console.log('🎨 Initializing Singularity animation...');
    const canvas = document.getElementById('singularityCanvas');
    if (!canvas) {
        console.error('❌ singularityCanvas element not found!');
        return;
    }
    console.log('✅ Canvas found:', canvas);
    
    const ctx = canvas.getContext('2d');
    if (!ctx) {
        console.error('❌ Could not get 2D context!');
        return;
    }
    canvas.width = 550;
    canvas.height = 550;
    console.log('✅ Canvas configured: 550x550');
    
    const centerX = canvas.width / 2;
    const centerY = canvas.height / 2;
    const particles = [];
    const particleCount = 200;
    let lastAudioLevel = 0;
    let peakTriggerThreshold = 0.3;
    let audioBursts = []; // Store audio-triggered particle bursts
    
    // Create particles starting from outer edge
    for (let i = 0; i < particleCount; i++) {
        const angle = Math.random() * Math.PI * 2;
        const startDistance = 250 + Math.random() * 50; // Start from outer edge
        particles.push({
            angle: angle,
            distance: startDistance,
            baseAngle: angle, // Store original angle for spiral
            speed: 0.5 + Math.random() * 1.0, // Speed of being pulled in
            size: Math.random() * 2 + 1,
            opacity: Math.random() * 0.6 + 0.4,
            spiralSpeed: 0.02 + Math.random() * 0.03, // Spiral rotation speed
            trail: [] // Store trail positions
        });
    }
    
    function animate() {
        try {
            // If bypassed, show static state
            if (state.bypass) {
                ctx.fillStyle = 'rgba(5, 5, 10, 0.1)';
                ctx.fillRect(0, 0, canvas.width, canvas.height);
                
                // Draw static event horizon
                const gradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, 80);
                gradient.addColorStop(0, 'rgba(153, 69, 255, 0.3)');
                gradient.addColorStop(1, 'rgba(153, 69, 255, 0)');
                ctx.fillStyle = gradient;
                ctx.beginPath();
                ctx.arc(centerX, centerY, 80, 0, Math.PI * 2);
                ctx.fill();
                
                requestAnimationFrame(animate);
                return;
            }
            
            // Clear with fade effect
        ctx.fillStyle = 'rgba(5, 5, 10, 0.1)';
        ctx.fillRect(0, 0, canvas.width, canvas.height);
        
        // Calculate audio reactivity
        const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
        const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60)); // -60 to 0 dB mapped to 0-1
        const grLevel = Math.abs(state.gainReduction) / 18; // 0-18 dB mapped to 0-1
        
        // Detect audio peaks for particle bursts (like Fabric's ripple detection)
        const levelDiff = normalizedLevel - lastAudioLevel;
        if (levelDiff > 0.15 && normalizedLevel > peakTriggerThreshold) {
            // Audio peak detected - create particle burst from outer edge
            const burstAngle = Math.random() * Math.PI * 2;
            const burstParticleCount = Math.floor(5 + normalizedLevel * 10); // 5-15 particles per burst
            
            for (let i = 0; i < burstParticleCount; i++) {
                const angle = burstAngle + (Math.random() - 0.5) * 0.5; // Cluster around burst angle
                const startDistance = 260 + Math.random() * 30;
                particles.push({
                    angle: angle,
                    distance: startDistance,
                    baseAngle: angle,
                    speed: 0.8 + Math.random() * 1.2 + normalizedLevel * 0.5, // Faster with more audio
                    size: Math.random() * 2.5 + 1.5,
                    opacity: Math.random() * 0.5 + 0.5,
                    spiralSpeed: 0.02 + Math.random() * 0.03,
                    trail: []
                });
            }
            
            // Also create visual burst effect
            audioBursts.push({
                angle: burstAngle,
                radius: 0,
                maxRadius: 200 + normalizedLevel * 100,
                strength: normalizedLevel * 1.5,
                age: 0
            });
        }
        lastAudioLevel = normalizedLevel;
        
        // Update and filter audio bursts
        audioBursts = audioBursts.filter(burst => {
            burst.radius += 8;
            burst.age += 1;
            return burst.radius < burst.maxRadius;
        });
        
        // Parameter influences
        const thresholdInfluence = (state.threshold + 60) / 60; // -60 to 0 dB -> 0 to 1
        const ratioInfluence = (state.ratio - 1) / 19; // 1 to 20 -> 0 to 1
        const attackInfluence = state.attack / 100; // 0.1 to 100 ms -> 0 to 1
        const releaseInfluence = state.release / 1000; // 10 to 1000 ms -> 0 to 1
        const makeupInfluence = (state.makeup + 12) / 36; // -12 to +24 dB -> 0 to 1
        const mixInfluence = state.mix / 100; // 0 to 100% -> 0 to 1
        
        // Event horizon size responds to input level and threshold (more dramatic with audio)
        const horizonSize = 45 + normalizedLevel * 35 + thresholdInfluence * 20 + grLevel * 10;
        const horizonIntensity = 0.4 + normalizedLevel * 0.4 + ratioInfluence * 0.2 + grLevel * 0.2;
        
        // Draw audio burst effects (expanding rings from audio peaks)
        audioBursts.forEach(burst => {
            const burstOpacity = (1 - burst.radius / burst.maxRadius) * burst.strength * 0.3;
            const burstX = centerX + Math.cos(burst.angle) * (burst.radius * 0.3);
            const burstY = centerY + Math.sin(burst.angle) * (burst.radius * 0.3);
            
            ctx.strokeStyle = `rgba(255, 255, 255, ${burstOpacity})`;
            ctx.lineWidth = 2 + burst.strength;
            ctx.beginPath();
            ctx.arc(burstX, burstY, burst.radius, 0, Math.PI * 2);
            ctx.stroke();
            
            // Inner glow
            ctx.strokeStyle = `rgba(153, 69, 255, ${burstOpacity * 0.6})`;
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.arc(burstX, burstY, burst.radius - 5, 0, Math.PI * 2);
            ctx.stroke();
        });
        
        // Draw accretion disk (rotating ring of matter around black hole) - more active with audio
        const diskSize = horizonSize * 1.6 + normalizedLevel * 20;
        const diskGradient = ctx.createRadialGradient(centerX, centerY, horizonSize, centerX, centerY, diskSize);
        diskGradient.addColorStop(0, 'rgba(153, 69, 255, 0)');
        diskGradient.addColorStop(0.5, `rgba(153, 69, 255, ${horizonIntensity * 0.4 * (0.7 + normalizedLevel * 0.3)})`);
        diskGradient.addColorStop(1, 'rgba(153, 69, 255, 0)');
        ctx.fillStyle = diskGradient;
        ctx.beginPath();
        ctx.arc(centerX, centerY, diskSize, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw event horizon (black hole boundary) - dark center with bright edge
        const eventHorizonGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, horizonSize);
        eventHorizonGradient.addColorStop(0, 'rgba(0, 0, 0, 0.95)'); // Almost black center
        eventHorizonGradient.addColorStop(0.7, `rgba(153, 69, 255, ${horizonIntensity * 0.4})`); // Bright edge
        eventHorizonGradient.addColorStop(1, 'rgba(153, 69, 255, 0)');
        ctx.fillStyle = eventHorizonGradient;
        ctx.beginPath();
        ctx.arc(centerX, centerY, horizonSize, 0, Math.PI * 2);
        ctx.fill();
        
        // Singularity core (bright center) - pulses with compression
        const coreSize = 8 + grLevel * 12 + makeupInfluence * 8;
        const coreIntensity = 0.9 + grLevel * 0.1 + makeupInfluence * 0.1;
        const coreGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, coreSize);
        coreGradient.addColorStop(0, `rgba(255, 255, 255, ${coreIntensity})`);
        coreGradient.addColorStop(0.3, `rgba(255, 26, 255, ${coreIntensity * 0.8})`);
        coreGradient.addColorStop(1, 'rgba(153, 69, 255, 0)');
        ctx.fillStyle = coreGradient;
        ctx.beginPath();
        ctx.arc(centerX, centerY, coreSize, 0, Math.PI * 2);
        ctx.fill();
        
        // Gravitational pull strength - affects how fast particles are sucked in (stronger with audio)
        const pullStrength = 1.0 + normalizedLevel * 2.0 + ratioInfluence * 0.8 + grLevel * 0.5;
        const spiralTightness = 1.0 + (1 - attackInfluence) * 0.5 + normalizedLevel * 0.3; // Faster attack = tighter spiral, audio makes it tighter
        const trailLength = 5 + Math.floor(releaseInfluence * 10) + Math.floor(normalizedLevel * 5); // Longer release = longer trails, audio adds more
        
        // Update and draw particles being sucked into black hole
        particles.forEach(particle => {
            // Calculate gravitational acceleration (stronger as particle gets closer)
            const distanceFromCenter = particle.distance;
            const gravitationalForce = (300 - distanceFromCenter) / 300; // Stronger when closer
            const acceleration = gravitationalForce * pullStrength * (0.8 + ratioInfluence * 0.4);
            
            // Particle is pulled toward center with acceleration
            particle.distance -= particle.speed * acceleration;
            
            // Spiral rotation - tighter as it gets closer (like water going down drain)
            const spiralFactor = 1.0 + (300 - distanceFromCenter) / 300; // Faster rotation when closer
            particle.angle += particle.spiralSpeed * spiralFactor * spiralTightness;
            
            // Store position in trail
            const x = centerX + Math.cos(particle.angle) * particle.distance;
            const y = centerY + Math.sin(particle.angle) * particle.distance;
            particle.trail.push({ x, y, distance: particle.distance });
            
            // Limit trail length
            if (particle.trail.length > trailLength) {
                particle.trail.shift();
            }
            
            // Reset particle if it reaches the event horizon (gets sucked in)
            if (particle.distance < horizonSize + 5) {
                particle.distance = 280 + Math.random() * 40; // Respawn at outer edge
                particle.angle = Math.random() * Math.PI * 2;
                particle.baseAngle = particle.angle;
                particle.trail = []; // Clear trail
            }
            
            // Draw particle trail (shows spiral path into black hole)
            if (particle.trail.length > 1) {
                ctx.strokeStyle = `rgba(153, 69, 255, ${particle.opacity * 0.4 * mixInfluence})`;
                ctx.lineWidth = particle.size * 0.3;
                ctx.beginPath();
                ctx.moveTo(particle.trail[0].x, particle.trail[0].y);
                for (let i = 1; i < particle.trail.length; i++) {
                    ctx.lineTo(particle.trail[i].x, particle.trail[i].y);
                }
                ctx.stroke();
            }
            
            // Draw particle - brighter and larger as it gets closer (gravitational lensing)
            // Audio makes particles more visible and larger
            const distanceFactor = 1 - (particle.distance / 300);
            const lensingEffect = 1 + distanceFactor * 2; // Particles appear larger when closer
            const audioBoost = 1 + normalizedLevel * 0.5 + grLevel * 0.3; // Audio makes particles brighter/larger
            const glowSize = particle.size * lensingEffect * audioBoost * (0.5 + mixInfluence * 0.5);
            
            // Particle color shifts from white/cyan to purple as it approaches
            // Audio makes particles brighter (more white/cyan)
            const colorShift = distanceFactor * (1 - normalizedLevel * 0.3); // Less color shift with audio
            const r = Math.floor(255 - colorShift * 102); // 255 -> 153
            const g = Math.floor(255 - colorShift * 186); // 255 -> 69
            const b = 255;
            
            const particleGradient = ctx.createRadialGradient(x, y, 0, x, y, glowSize);
            const particleOpacity = particle.opacity * (0.6 + normalizedLevel * 0.4 + grLevel * 0.2) * mixInfluence;
            particleGradient.addColorStop(0, `rgba(${r}, ${g}, ${b}, ${particleOpacity})`);
            particleGradient.addColorStop(0.5, `rgba(153, 69, 255, ${particleOpacity * 0.6})`);
            particleGradient.addColorStop(1, 'rgba(153, 69, 255, 0)');
            
            ctx.fillStyle = particleGradient;
            ctx.beginPath();
            ctx.arc(x, y, glowSize, 0, Math.PI * 2);
            ctx.fill();
        });
        
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
    console.log('✅ Singularity animation fully initialized');
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
    if (!data) {
        console.log('⚠️ receiveAudioData called with no data');
        return;
    }
    
    // Debug logging (every 30 calls = ~1.5 seconds at 50ms timer)
    if (window.audioDataCounter === undefined) window.audioDataCounter = 0;
    if (++window.audioDataCounter % 30 === 0) {
        console.log('🎵 receiveAudioData: Input=' + data.inputLevel + 'dB, Output=' + data.outputLevel + 'dB, GR=' + data.gainReduction + 'dB');
    }
    
    state.inputLevel = data.inputLevel || -100;
    state.outputLevel = data.outputLevel || -100;
    state.gainReduction = data.gainReduction || 0;
    
    // Update VU meters
    updateVUMeter('input', state.inputLevel);
    updateVUMeter('output', state.outputLevel);
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
