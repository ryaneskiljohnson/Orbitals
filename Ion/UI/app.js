/**
 * @fileoverview Ion - Harmonic Constraint Engine
 * @module Ion Application
 * 
 * @brief Main application logic for Ion - magnetic harmonic attraction
 * @note Visualizes magnetic field with chromatic note attraction
 */

document.addEventListener('DOMContentLoaded', () => {
  const canvas = document.getElementById('ionCanvas');
  if (canvas) setupIonVisualization(canvas);
  
  setupControls();
});

/**
 * @brief Sets up the magnetic field visualization
 * @param {HTMLCanvasElement} canvas - The canvas element
 */
function setupIonVisualization(canvas) {
  const initCanvas = () => {
    let ctx = CanvasUtils.setupHiDPI(canvas);
    
    const getCanvasSize = () => {
      const rect = canvas.getBoundingClientRect();
      return { width: rect.width, height: rect.height };
    };
    
    const getCenter = () => {
      const size = getCanvasSize();
      // Center in the animation panel (right side)
      return { x: size.width / 2, y: size.height / 2 };
    };
    
    let canvasSize = getCanvasSize();
    let center = getCenter();
    let centerX = center.x;
    let centerY = center.y;
    
    // Control values
    let fieldStrength = 50; // 0-100
    let attractionStrength = 50; // 0-100
    let root = 0; // 0-11 (C-B) - Root note of the scale
    let scale = 'major'; // Current scale
    let attractionType = 'snap'; // snap, pull, guide
    let time = 0;
    
    // Scale definitions (intervals from root)
    const scales = {
      major: [0, 2, 4, 5, 7, 9, 11],
      minor: [0, 2, 3, 5, 7, 8, 10],
      dorian: [0, 2, 3, 5, 7, 9, 10],
      phrygian: [0, 1, 3, 5, 7, 8, 10],
      lydian: [0, 2, 4, 6, 7, 9, 11],
      mixolydian: [0, 2, 4, 5, 7, 9, 10],
      aeolian: [0, 2, 3, 5, 7, 8, 10],
      locrian: [0, 1, 3, 5, 6, 8, 10],
      chromatic: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
    };
    
    // Note particles attracted to field
    class NoteParticle {
      constructor() {
        this.reset();
      }
      
      reset() {
        this.angle = Math.random() * Math.PI * 2;
        this.distance = (200 + Math.random() * 150) * 1.2; // Scaled 20% larger
        this.note = Math.floor(Math.random() * 12);
        this.velocity = 0.5 + Math.random() * 1.5;
        this.size = (3 + Math.random() * 3) * 1.2; // Scaled 20% larger
        this.alpha = 0.8;
      }
      
      update(deltaTime) {
        // Move particle
        this.angle += this.velocity * deltaTime * 0.0001;
        
        // Check if note is in scale
        const currentScale = scales[scale] || scales.major;
        const relativeNote = (this.note - root + 12) % 12;
        const inScale = currentScale.includes(relativeNote);
        
        // Apply magnetic attraction based on attraction type
        if (inScale) {
          const baseAttraction = attractionStrength / 100;
          let attractionMultiplier = 1;
          let attractionSpeed = 0.05;
          
          // Different behaviors for each attraction type
          if (attractionType === 'snap') {
            // SNAP: Strong, immediate attraction - fast and aggressive
            attractionMultiplier = 3.5;
            attractionSpeed = 0.12;
            this.alpha = 1.0; // Full brightness when snapping
          } else if (attractionType === 'pull') {
            // PULL: Gradual, smooth attraction - moderate speed
            attractionMultiplier = 1.8;
            attractionSpeed = 0.08;
            this.alpha = 0.85;
          } else if (attractionType === 'guide') {
            // GUIDE: Subtle, gentle attraction - slow and gentle
            attractionMultiplier = 0.6;
            attractionSpeed = 0.03;
            this.alpha = 0.7;
          }
          
          const attraction = baseAttraction * attractionMultiplier;
          this.distance -= attraction * deltaTime * attractionSpeed;
        } else {
          // Out-of-scale particles drift away
          const driftSpeed = attractionType === 'snap' ? 0.5 : 
                            attractionType === 'pull' ? 0.3 : 0.15;
          this.distance += driftSpeed * deltaTime * 0.05;
          this.alpha = 0.4;
        }
        
        // Reset if too close or too far (scaled 20% larger)
        if (this.distance < 30 * 1.2 || this.distance > 400 * 1.2) {
          this.reset();
        }
      }
      
      draw(ctx) {
        const x = centerX + Math.cos(this.angle) * this.distance;
        const y = centerY + Math.sin(this.angle) * this.distance;
        
        // Color based on note
        const hue = (this.note / 12) * 300 + 240; // 240-540 (purple to cyan)
        
        ctx.save();
        ctx.globalAlpha = this.alpha;
        
        // Glow
        const gradient = ctx.createRadialGradient(x, y, 0, x, y, this.size * 2);
        gradient.addColorStop(0, `hsl(${hue}, 80%, 70%)`);
        gradient.addColorStop(0.5, `hsl(${hue}, 80%, 50%, 0.5)`);
        gradient.addColorStop(1, `hsl(${hue}, 80%, 30%, 0)`);
        
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(x, y, this.size * 2, 0, Math.PI * 2);
        ctx.fill();
        
        // Core
        ctx.shadowColor = `hsl(${hue}, 80%, 70%)`;
        ctx.shadowBlur = 8;
        ctx.fillStyle = `hsl(${hue}, 80%, 80%)`;
        ctx.beginPath();
        ctx.arc(x, y, this.size, 0, Math.PI * 2);
        ctx.fill();
        
        ctx.restore();
      }
    }
    
    // Initialize particles
    const particles = [];
    for (let i = 0; i < 30; i++) {
      particles.push(new NoteParticle());
    }
    
    // Handle resize
    const resizeObserver = new ResizeObserver(() => {
      ctx = CanvasUtils.setupHiDPI(canvas);
      requestAnimationFrame(() => {
        canvasSize = getCanvasSize();
        center = getCenter();
        centerX = center.x;
        centerY = center.y;
      });
    });
    resizeObserver.observe(canvas);
    
    // Expose control updaters
    window.updateFieldStrength = (value) => { fieldStrength = value; };
    window.updateAttractionStrength = (value) => { attractionStrength = value; };
    window.updateRoot = (value) => { root = value; };
    window.updateScale = (newScale) => { scale = newScale; };
    window.updateAttractionType = (type) => { attractionType = type; };
    
    animationManager.add('ion-magnetic-field', (deltaTime) => {
      time += deltaTime * 0.001;
      canvasSize = getCanvasSize();
      center = getCenter();
      centerX = center.x;
      centerY = center.y;
      
      // Clear with smooth fade trail effect
      ctx.fillStyle = 'rgba(10, 10, 20, 0.25)';
      ctx.fillRect(0, 0, canvasSize.width, canvasSize.height);
      
      // Draw magnetic field rings (scaled 20% larger) - pulse intensity varies by attraction type
      const fieldRadius = (50 + (fieldStrength / 100) * 150) * 1.2; // 60-240px (20% larger)
      const ringCount = 8;
      
      let ringPulseIntensity = 0.05;
      let ringBaseAlpha = 0.15;
      if (attractionType === 'snap') {
        ringPulseIntensity = 0.12; // Stronger pulse
        ringBaseAlpha = 0.2;
      } else if (attractionType === 'pull') {
        ringPulseIntensity = 0.07;
        ringBaseAlpha = 0.17;
      } else if (attractionType === 'guide') {
        ringPulseIntensity = 0.03; // Subtle pulse
        ringBaseAlpha = 0.12;
      }
      
      for (let i = 0; i < ringCount; i++) {
        const r = (fieldRadius / ringCount) * (i + 1);
        const alpha = ringBaseAlpha - (i / ringCount) * 0.1;
        const pulse = Math.sin(time * 2 + i * 0.5) * ringPulseIntensity;
        
        ctx.save();
        ctx.strokeStyle = `rgba(139, 92, 246, ${alpha + pulse})`;
        ctx.lineWidth = 1.5;
        ctx.setLineDash([4, 8]);
        ctx.beginPath();
        ctx.arc(centerX, centerY, r, 0, Math.PI * 2);
        ctx.stroke();
        ctx.restore();
      }
      
      // Draw chromatic field lines (12 notes) - scaled 20% larger
      const currentScale = scales[scale] || scales.major;
      for (let i = 0; i < 12; i++) {
        const angle = (i / 12) * Math.PI * 2 - Math.PI / 2;
        const innerRadius = 40 * 1.2; // 48px (20% larger)
        const outerRadius = fieldRadius;
        
        const x1 = centerX + Math.cos(angle) * innerRadius;
        const y1 = centerY + Math.sin(angle) * innerRadius;
        const x2 = centerX + Math.cos(angle) * outerRadius;
        const y2 = centerY + Math.sin(angle) * outerRadius;
        
        // Check if this note is in the current scale
        const relativeNote = (i - root + 12) % 12;
        const inScale = currentScale.includes(relativeNote);
        
        // Pulsing field lines - intensity varies by attraction type
        let pulseIntensity = 0.1;
        let baseAlpha = inScale ? 0.25 : 0.08;
        
        if (attractionType === 'snap') {
          pulseIntensity = 0.2; // Stronger pulse for snap
          baseAlpha = inScale ? 0.35 : 0.08;
        } else if (attractionType === 'pull') {
          pulseIntensity = 0.12; // Moderate pulse
          baseAlpha = inScale ? 0.28 : 0.08;
        } else if (attractionType === 'guide') {
          pulseIntensity = 0.05; // Subtle pulse
          baseAlpha = inScale ? 0.18 : 0.08;
        }
        
        const pulse = Math.sin(time * 3 + i * 0.3) * pulseIntensity;
        const alpha = baseAlpha + pulse + (attractionStrength / 100) * 0.15;
        
        const hue = (i / 12) * 300 + 240;
        const gradient = ctx.createLinearGradient(x1, y1, x2, y2);
        gradient.addColorStop(0, `hsla(${hue}, 80%, 60%, ${alpha})`);
        gradient.addColorStop(1, `hsla(${hue}, 80%, 50%, ${alpha * 0.3})`);
        
        ctx.save();
        ctx.strokeStyle = gradient;
        ctx.lineWidth = inScale ? 2 : 1;
        ctx.beginPath();
        ctx.moveTo(x1, y1);
        ctx.lineTo(x2, y2);
        ctx.stroke();
        ctx.restore();
        
        // Note markers at field line ends (scaled 20% larger)
        if (inScale) {
          const markerSize = (4 + Math.sin(time * 2 + i * 0.5) * 1.5) * 1.2;
          ctx.save();
          ctx.shadowColor = `hsl(${hue}, 80%, 70%)`;
          ctx.shadowBlur = 12;
          ctx.fillStyle = `hsl(${hue}, 80%, 70%)`;
          ctx.beginPath();
          ctx.arc(x2, y2, markerSize, 0, Math.PI * 2);
          ctx.fill();
          ctx.restore();
        }
      }
      
      // Update and draw particles
      particles.forEach(particle => {
        particle.update(deltaTime);
        particle.draw(ctx);
      });
      
      // Draw center core (root note) - scaled 20% larger, pulse varies by attraction type
      let corePulseAmplitude = 3;
      let corePulseSpeed = 2;
      let coreIntensity = 0.8;
      
      if (attractionType === 'snap') {
        corePulseAmplitude = 6; // Stronger pulse
        corePulseSpeed = 3; // Faster pulse
        coreIntensity = 1.0; // Brighter
      } else if (attractionType === 'pull') {
        corePulseAmplitude = 4;
        corePulseSpeed = 2.5;
        coreIntensity = 0.85;
      } else if (attractionType === 'guide') {
        corePulseAmplitude = 2; // Subtle pulse
        corePulseSpeed = 1.5; // Slower pulse
        coreIntensity = 0.7; // Dimmer
      }
      
      const coreRadius = (25 + Math.sin(time * corePulseSpeed) * corePulseAmplitude) * 1.2;
      const coreGradient = ctx.createRadialGradient(centerX, centerY, 0, centerX, centerY, coreRadius);
      coreGradient.addColorStop(0, `rgba(192, 132, 252, ${coreIntensity})`);
      coreGradient.addColorStop(0.5, `rgba(139, 92, 246, ${coreIntensity * 0.6})`);
      coreGradient.addColorStop(1, `rgba(139, 92, 246, 0)`);
      ctx.fillStyle = coreGradient;
      ctx.beginPath();
      ctx.arc(centerX, centerY, coreRadius, 0, Math.PI * 2);
      ctx.fill();
      
      ctx.save();
      ctx.shadowColor = '#c084fc';
      ctx.shadowBlur = (25 + Math.sin(time * corePulseSpeed) * 5) * 1.2; // Pulse shadow too
      ctx.fillStyle = '#c084fc';
      ctx.beginPath();
      ctx.arc(centerX, centerY, 12 * 1.2, 0, Math.PI * 2); // 14.4px (20% larger)
      ctx.fill();
      ctx.restore();
      
      // Draw root note label - scaled 20% larger
      const noteNames = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'];
      ctx.save();
      ctx.font = 'bold ' + (14 * 1.2) + 'px Inter'; // 16.8px (20% larger)
      ctx.fillStyle = '#ffffff';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.shadowColor = '#8b5cf6';
      ctx.shadowBlur = 10;
      ctx.fillText(noteNames[root], centerX, centerY);
      ctx.restore();
    });
  };
  
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initCanvas);
  } else {
    requestAnimationFrame(() => setTimeout(initCanvas, 0));
  }
}

/**
 * @brief Sets up all UI controls
 */
function setupControls() {
  // Field Strength Slider
  const fieldStrengthSlider = document.getElementById('fieldStrengthSlider');
  if (fieldStrengthSlider) {
    const slider = new OrbitalsSlider(fieldStrengthSlider, {
      min: 0, max: 100, value: 50,
      orientation: 'horizontal',
      onChange: (v) => {
        document.getElementById('fieldStrengthValue').textContent = Math.round(v) + '%';
        if (window.updateFieldStrength) window.updateFieldStrength(v);
        sendToJUCE('fieldStrength', v);
      }
    });
    // Ensure initial value is set for fill (updatePosition already sets --slider-value)
    slider.updatePosition();
  }
  
  // Attraction Strength Slider
  const attractionSlider = document.getElementById('attractionSlider');
  if (attractionSlider) {
    const slider = new OrbitalsSlider(attractionSlider, {
      min: 0, max: 100, value: 50,
      orientation: 'horizontal',
      onChange: (v) => {
        document.getElementById('attractionStrengthValue').textContent = Math.round(v) + '%';
        if (window.updateAttractionStrength) window.updateAttractionStrength(v);
        sendToJUCE('attractionStrength', v);
      }
    });
    // Ensure initial value is set for fill (updatePosition already sets --slider-value)
    slider.updatePosition();
  }
  
  // Root Selector Buttons
  const rootButtons = document.querySelectorAll('.root-selector button');
  rootButtons.forEach(button => {
    button.addEventListener('click', () => {
      rootButtons.forEach(b => b.classList.remove('active'));
      button.classList.add('active');
      const note = parseInt(button.dataset.note);
      if (window.updateRoot) window.updateRoot(note);
      sendToJUCE('root', note);
    });
  });
  
  // Scale Selector Buttons
  const scaleButtons = document.querySelectorAll('.scale-selector button');
  scaleButtons.forEach(button => {
    button.addEventListener('click', () => {
      scaleButtons.forEach(b => b.classList.remove('active'));
      button.classList.add('active');
      const scale = button.dataset.scale;
      if (window.updateScale) window.updateScale(scale);
      sendToJUCE('scale', scale);
    });
  });
  
  // Attraction Type Buttons
  const attractionButtons = document.querySelectorAll('.attraction-type-selector button');
  attractionButtons.forEach(button => {
    button.addEventListener('click', () => {
      attractionButtons.forEach(b => b.classList.remove('active'));
      button.classList.add('active');
      const type = button.dataset.type;
      // Update value display if it exists (not in footer)
      const valueDisplay = document.getElementById('attractionValue');
      if (valueDisplay) {
        valueDisplay.textContent = type.toUpperCase();
      }
      if (window.updateAttractionType) window.updateAttractionType(type);
      sendToJUCE('attractionType', type);
    });
  });
}


/**
 * @brief Sends parameter changes to JUCE
 * @param {string} param - Parameter name
 * @param {*} value - Parameter value
 */
function sendToJUCE(param, value) {
  if (window.chrome?.webview) {
    window.chrome.webview.postMessage({ type: 'parameterChange', parameter: param, value });
  }
}
