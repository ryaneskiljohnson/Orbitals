/**
 * @fileoverview Hadron - Particle Collision Saturation UI Logic
 * @module Hadron App
 */

const state = {
    drive: 50,
    tone: 50,
    bias: 0,
    mix: 100,
    output: 0,
    bypass: false,
    inputLevel: -100,
    outputLevel: -100
};

document.addEventListener('DOMContentLoaded', () => {
    initializeSettingsButton();
    new MandelbrotKnob(document.getElementById('driveKnob'), { min: 0, max: 100, value: 50, onChange: (v, p) => { state.drive = v; document.getElementById('driveValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('toneKnob'), { min: 0, max: 100, value: 50, onChange: (v, p) => { state.tone = v; document.getElementById('toneValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('biasKnob'), { min: -100, max: 100, value: 0, onChange: (v, p) => { state.bias = v; document.getElementById('biasValue').textContent = `${v >= 0 ? '+' : ''}${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('mixKnob'), { min: 0, max: 100, value: 100, onChange: (v, p) => { state.mix = v; document.getElementById('mixValue').textContent = `${v.toFixed(0)}%`; sendToPlugin(p, v); } });
    new MandelbrotKnob(document.getElementById('outputKnob'), { min: -12, max: 12, value: 0, onChange: (v, p) => { state.output = v; document.getElementById('outputValue').textContent = `${v >= 0 ? '+' : ''}${v.toFixed(1)} dB`; sendToPlugin(p, v); } });
    initializeBypassToggle();
    initializeHadronAnimation();

    // Particle collision animation - responds to audio
    const canvas = document.getElementById('hadronCanvas');
    if (canvas) {
        const ctx = canvas.getContext('2d');
        canvas.width = 600;
        canvas.height = 600;
        let particles = [];
        let collisionTime = 0;
        
        function animate() {
            // Calculate audio reactivity
            const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
            const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60));
            
            ctx.fillStyle = 'rgba(13, 13, 21, 0.1)';
            ctx.fillRect(0, 0, 600, 600);
            
            collisionTime++;
            
            // Collision frequency responds to audio
            const collisionFreq = Math.floor(60 - normalizedLevel * 40);
            if (collisionTime % collisionFreq === 0) {
                const particleCount = 50 + Math.floor(normalizedLevel * 50);
                for (let i = 0; i < particleCount; i++) {
                    const angle = Math.random() * Math.PI * 2;
                    const speed = (Math.random() * 5 + 2) * (1 + normalizedLevel);
                    particles.push({ 
                        x: 300, 
                        y: 300, 
                        vx: Math.cos(angle) * speed, 
                        vy: Math.sin(angle) * speed, 
                        life: 60, 
                        color: Math.random() > 0.5 ? '#ff0080' : '#ffd700',
                        size: 2 + normalizedLevel * 3
                    });
                }
            }
            
            particles = particles.filter(p => p.life > 0);
            particles.forEach(p => {
                p.x += p.vx;
                p.y += p.vy;
                p.life--;
                const alpha = (p.life / 60) * (0.7 + normalizedLevel * 0.3);
                
                const grad = ctx.createRadialGradient(p.x, p.y, 0, p.x, p.y, p.size * 3);
                grad.addColorStop(0, p.color.replace(')', `, ${alpha})`).replace('#', 'rgba(').replace(/(.{2})/g, '$1,').slice(0, -1));
                grad.addColorStop(1, p.color.replace(')', ', 0)').replace('#', 'rgba('));
                ctx.fillStyle = grad;
                ctx.beginPath();
                ctx.arc(p.x, p.y, p.size * 3, 0, Math.PI * 2);
                ctx.fill();
            });
            
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
    if (window.juce) {
        window.juce.postMessage({
            type: parameter === 'openSettings' ? 'openSettings' : 'parameterChange',
            parameter: parameter,
            value: value
        });
    }
}

// Initialize Hadron particle collision animation
function initializeHadronAnimation() {
    const canvas = document.getElementById('hadronCanvas');
    if (!canvas) return;
    
    const ctx = canvas.getContext('2d');
    canvas.width = 600;
    canvas.height = 600;
    
    const centerX = 300;
    const centerY = 300;
    let particles = [];
    let collisionBursts = [];
    
    // Create orbiting particles
    for (let i = 0; i < 12; i++) {
        particles.push({
            angle: (Math.PI * 2 * i) / 12,
            speed: 0.02 + Math.random() * 0.01,
            radius: 150 + Math.random() * 50,
            size: 3 + Math.random() * 2
        });
    }
    
    let time = 0;
    
    function animate() {
        const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
        const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60));
        
        // Parameter influence
        const driveInfluence = state.drive / 100; // Controls speed
        const toneInfluence = state.tone / 100; // Controls color
        const biasInfluence = state.bias / 100; // Controls orbit shape
        const mixInfluence = state.mix / 100; // Controls visibility
        
        // Fade with trails
        ctx.fillStyle = 'rgba(13, 13, 21, 0.1)';
        ctx.fillRect(0, 0, 600, 600);
        
        time += 0.02;
        
        // Create collision bursts on audio peaks
        if (normalizedLevel > 0.6) {
            collisionBursts.push({
                x: centerX,
                y: centerY,
                radius: 0,
                maxRadius: 150 * normalizedLevel,
                opacity: 1
            });
        }
        
        // Update and draw collision bursts
        collisionBursts = collisionBursts.filter(burst => {
            burst.radius += 5;
            burst.opacity -= 0.02;
            
            if (burst.opacity > 0) {
                ctx.strokeStyle = `rgba(255, 128, 0, ${burst.opacity * mixInfluence})`;
                ctx.lineWidth = 3;
                ctx.beginPath();
                ctx.arc(burst.x, burst.y, burst.radius, 0, Math.PI * 2);
                ctx.stroke();
            }
            
            return burst.opacity > 0 && burst.radius < burst.maxRadius;
        });
        
        // Update and draw particles
        particles.forEach(particle => {
            // DRIVE affects particle speed
            particle.angle += particle.speed * (1 + driveInfluence * 2) * (1 + normalizedLevel * 0.5);
            
            // BIAS affects orbit shape (circular to elliptical)
            const radiusX = particle.radius * (1 + biasInfluence * 0.5);
            const radiusY = particle.radius * (1 - biasInfluence * 0.5);
            
            const x = centerX + Math.cos(particle.angle) * radiusX;
            const y = centerY + Math.sin(particle.angle) * radiusY;
            
            // TONE affects color (bright orange to dark red)
            const r = 255;
            const g = Math.floor(128 * (1 - toneInfluence));
            const b = 0;
            
            // Size responds to audio and drive
            const size = particle.size * (1 + driveInfluence * 0.5) * (1 + normalizedLevel * 0.8);
            const opacity = (0.7 + normalizedLevel * 0.3) * mixInfluence;
            
            // Draw particle with glow
            const gradient = ctx.createRadialGradient(x, y, 0, x, y, size * 3);
            gradient.addColorStop(0, `rgba(${r}, ${g}, ${b}, ${opacity})`);
            gradient.addColorStop(0.5, `rgba(${r}, ${g}, ${b}, ${opacity * 0.5})`);
            gradient.addColorStop(1, `rgba(${r}, ${g}, ${b}, 0)`);
            ctx.fillStyle = gradient;
            ctx.beginPath();
            ctx.arc(x, y, size * 3, 0, Math.PI * 2);
            ctx.fill();
            
            // Draw core
            ctx.fillStyle = `rgba(255, 255, 255, ${opacity})`;
            ctx.beginPath();
            ctx.arc(x, y, size, 0, Math.PI * 2);
            ctx.fill();
        });
        
        // Draw center collision zone
        const centerGlow = normalizedLevel * driveInfluence * mixInfluence;
        if (centerGlow > 0.1) {
            const gradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, 50 * centerGlow);
            gradient.addColorStop(0, `rgba(255, 200, 0, ${centerGlow * 0.6})`);
            gradient.addColorStop(1, 'rgba(255, 128, 0, 0)');
            ctx.fillStyle = gradient;
            ctx.beginPath();
            ctx.arc(centerX, centerY, 50 * centerGlow, 0, Math.PI * 2);
            ctx.fill();
        }
        
        requestAnimationFrame(animate);
    }
    
    animate();
}

// Initialize Hadron particle collision animation
function initializeHadronAnimation() {
    const canvas = document.getElementById('hadronCanvas');
    if (!canvas) return;
    
    const ctx = canvas.getContext('2d');
    canvas.width = 600;
    canvas.height = 600;
    
    const centerX = 300;
    const centerY = 300;
    let particles = [];
    let collisionBursts = [];
    
    // Create orbiting particles
    for (let i = 0; i < 12; i++) {
        particles.push({
            angle: (Math.PI * 2 * i) / 12,
            speed: 0.02 + Math.random() * 0.01,
            radius: 150 + Math.random() * 50,
            size: 3 + Math.random() * 2
        });
    }
    
    let time = 0;
    
    function animate() {
        const audioLevel = state.inputLevel > -100 ? state.inputLevel : -60;
        const normalizedLevel = Math.max(0, Math.min(1, (audioLevel + 60) / 60));
        
        // Parameter influence
        const driveInfluence = state.drive / 100;
        const toneInfluence = state.tone / 100;
        const biasInfluence = state.bias / 100;
        const mixInfluence = state.mix / 100;
        
        // Fade with trails
        ctx.fillStyle = 'rgba(13, 13, 21, 0.1)';
        ctx.fillRect(0, 0, 600, 600);
        
        time += 0.02;
        
        // Create collision bursts on audio peaks
        if (normalizedLevel > 0.6) {
            collisionBursts.push({
                x: centerX,
                y: centerY,
                radius: 0,
                maxRadius: 150 * normalizedLevel,
                opacity: 1
            });
        }
        
        // Update and draw collision bursts
        collisionBursts = collisionBursts.filter(burst => {
            burst.radius += 5;
            burst.opacity -= 0.02;
            
            if (burst.opacity > 0) {
                ctx.strokeStyle = `rgba(255, 128, 0, ${burst.opacity * mixInfluence})`;
                ctx.lineWidth = 3;
                ctx.beginPath();
                ctx.arc(burst.x, burst.y, burst.radius, 0, Math.PI * 2);
                ctx.stroke();
            }
            
            return burst.opacity > 0 && burst.radius < burst.maxRadius;
        });
        
        // Update and draw particles
        particles.forEach(particle => {
            // DRIVE affects particle speed
            particle.angle += particle.speed * (1 + driveInfluence * 2) * (1 + normalizedLevel * 0.5);
            
            // BIAS affects orbit shape
            const radiusX = particle.radius * (1 + biasInfluence * 0.5);
            const radiusY = particle.radius * (1 - biasInfluence * 0.5);
            
            const x = centerX + Math.cos(particle.angle) * radiusX;
            const y = centerY + Math.sin(particle.angle) * radiusY;
            
            // TONE affects color
            const r = 255;
            const g = Math.floor(128 * (1 - toneInfluence));
            const b = 0;
            
            // Size responds to audio and drive
            const size = particle.size * (1 + driveInfluence * 0.5) * (1 + normalizedLevel * 0.8);
            const opacity = (0.7 + normalizedLevel * 0.3) * mixInfluence;
            
            // Draw particle with glow
            const gradient = ctx.createRadialGradient(x, y, 0, x, y, size * 3);
            gradient.addColorStop(0, `rgba(${r}, ${g}, ${b}, ${opacity})`);
            gradient.addColorStop(0.5, `rgba(${r}, ${g}, ${b}, ${opacity * 0.5})`);
            gradient.addColorStop(1, `rgba(${r}, ${g}, ${b}, 0)`);
            ctx.fillStyle = gradient;
            ctx.beginPath();
            ctx.arc(x, y, size * 3, 0, Math.PI * 2);
            ctx.fill();
            
            // Draw core
            ctx.fillStyle = `rgba(255, 255, 255, ${opacity})`;
            ctx.beginPath();
            ctx.arc(x, y, size, 0, Math.PI * 2);
            ctx.fill();
        });
        
        // Draw center collision zone
        const centerGlow = normalizedLevel * driveInfluence * mixInfluence;
        if (centerGlow > 0.1) {
            const gradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, 50 * centerGlow);
            gradient.addColorStop(0, `rgba(255, 200, 0, ${centerGlow * 0.6})`);
            gradient.addColorStop(1, 'rgba(255, 128, 0, 0)');
            ctx.fillStyle = gradient;
            ctx.beginPath();
            ctx.arc(centerX, centerY, 50 * centerGlow, 0, Math.PI * 2);
            ctx.fill();
        }
        
        requestAnimationFrame(animate);
    }
    
    animate();
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
            inputMeterFill.style.background = 'linear-gradient(to right, rgba(255, 68, 68, 0.3) 0%, rgba(255, 68, 68, 0.6) 50%, #ff4444 100%)';
            inputMeterFill.style.boxShadow = '0 0 8px rgba(255, 68, 68, 0.8)';
        }
        if (outputMeterFill) {
            outputMeterFill.style.background = 'linear-gradient(to right, rgba(255, 68, 68, 0.3) 0%, rgba(255, 68, 68, 0.6) 50%, #ff4444 100%)';
            outputMeterFill.style.boxShadow = '0 0 8px rgba(255, 68, 68, 0.8)';
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
