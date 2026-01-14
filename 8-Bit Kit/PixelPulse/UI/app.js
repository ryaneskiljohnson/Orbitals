/**
 * @fileoverview PixelPulse - Retro Rhythm Game
 * @module PixelPulse App
 * 
 * A rhythm game where hitting notes generates chords and melodies.
 * Inspired by classic arcade games like Guitar Hero and DDR.
 */

// Musical scales (note indices relative to root)
const SCALES = {
    'Major': [0, 2, 4, 5, 7, 9, 11],
    'Minor': [0, 2, 3, 5, 7, 8, 10],
    'Pentatonic': [0, 2, 4, 7, 9],
    'Blues': [0, 3, 5, 6, 7, 10],
    'Dorian': [0, 2, 3, 5, 7, 9, 10],
    'Mixolydian': [0, 2, 4, 5, 7, 9, 10]
};

const scaleNames = Object.keys(SCALES);

// Game state
const state = {
    tempo: 120,
    scale: 0,
    difficulty: 1,
    volume: 0,
    gameRunning: false,
    score: 0,
    combo: 0,
    bestCombo: 0,
    
    // Game timing
    beatTime: 0,
    lastBeatTime: 0,
    
    // Notes
    notes: [],
    nextNoteId: 0,
    
    // Hit window timing (in milliseconds)
    perfectWindow: 50,
    goodWindow: 100,
    okWindow: 150,
    
    // Lanes based on difficulty
    lanes: 4,
    
    // Key mappings
    keyMap: {
        'KeyA': 0,
        'KeyS': 1,
        'KeyD': 2,
        'KeyF': 3,
        'KeyJ': 4,
        'KeyK': 5,
        'KeyL': 6,
        'Semicolon': 7
    }
};

// Canvas and context
let canvas, ctx;
let animationId = null;

// Lane colors
const LANE_COLORS = [
    '#ff0000', // Red
    '#00ff00', // Green
    '#0000ff', // Blue
    '#ffff00', // Yellow
    '#ff00ff', // Magenta
    '#00ffff', // Cyan
    '#ff8800', // Orange
    '#8800ff'  // Purple
];

/**
 * Note object
 */
class Note {
    constructor(lane, time, noteValue) {
        this.id = state.nextNoteId++;
        this.lane = lane;
        this.spawnTime = time;
        this.y = 0;
        this.noteValue = noteValue; // MIDI note value
        this.hit = false;
        this.missed = false;
        this.active = true;
    }
    
    update(currentTime, noteSpeed) {
        // Calculate how far down the screen the note should be
        const elapsed = currentTime - this.spawnTime;
        const targetTime = 3000; // Time it takes for note to reach bottom (3 seconds)
        this.y = (elapsed / targetTime) * (canvas.height - 100);
        
        // Check if note is missed (past the hit line)
        if (this.y > canvas.height - 80 && !this.hit && !this.missed) {
            this.missed = true;
            state.combo = 0;
            showFeedback('MISS');
            return false;
        }
        
        return this.y < canvas.height;
    }
    
    draw() {
        if (!this.active) return;
        
        const laneWidth = canvas.width / state.lanes;
        const x = this.lane * laneWidth + laneWidth / 2;
        const size = 40;
        
        // Note glow
        ctx.shadowBlur = 20;
        ctx.shadowColor = LANE_COLORS[this.lane];
        
        // Draw note
        if (this.hit) {
            // Shrinking animation
            ctx.globalAlpha = 0.5;
        }
        
        ctx.fillStyle = LANE_COLORS[this.lane];
        ctx.fillRect(x - size/2, this.y - size/2, size, size);
        
        // Note border
        ctx.strokeStyle = '#ffffff';
        ctx.lineWidth = 2;
        ctx.strokeRect(x - size/2, this.y - size/2, size, size);
        
        ctx.shadowBlur = 0;
        ctx.globalAlpha = 1;
    }
    
    checkHit(currentTime) {
        const hitLineY = canvas.height - 80;
        const distance = Math.abs(this.y - hitLineY);
        
        // Convert pixel distance to time difference
        const targetTime = 3000;
        const timePerPixel = targetTime / (canvas.height - 100);
        const timeDiff = distance * timePerPixel;
        
        if (timeDiff <= state.perfectWindow) {
            return 'perfect';
        } else if (timeDiff <= state.goodWindow) {
            return 'good';
        } else if (timeDiff <= state.okWindow) {
            return 'ok';
        }
        return null;
    }
}

/**
 * Initialize the game
 */
function initializeGame() {
    canvas = document.getElementById('gameCanvas');
    if (!canvas) {
        console.error('Canvas not found!');
        return;
    }
    
    ctx = canvas.getContext('2d');
    canvas.width = 600;
    canvas.height = 600;
    
    console.log('Game initialized. Canvas size:', canvas.width, 'x', canvas.height);
    console.log('==========================================');
    console.log('KEYBOARD INPUT ENABLED');
    console.log('Keys will be captured from JUCE C++');
    console.log('If that fails, browser events will be used');
    console.log('If both fail, use MOUSE CLICKS!');
    console.log('==========================================');
    
    // Set up keyboard controls at BODY level to ensure we catch everything
    document.body.addEventListener('keydown', handleKeyPress, true);
    window.addEventListener('keydown', handleKeyPress, true);
    
    // Make sure canvas is focusable
    canvas.tabIndex = 1000;
    canvas.style.outline = 'none';
    
    // Click to focus
    canvas.addEventListener('click', () => {
        canvas.focus();
        console.log('Canvas clicked and focused');
    });
    
    // Focus canvas immediately
    setTimeout(() => {
        canvas.focus();
        console.log('Canvas auto-focused');
        console.log('==========================================');
        console.log('KEYBOARD TEST:');
        console.log('1. Press ANY key now to test');
        console.log('2. If you see console logs, keyboard works!');
        console.log('3. Then click PLAY and use A S D F keys');
        console.log('==========================================');
    }, 100);
    
    // Add mouse click backup control
    canvas.addEventListener('click', (e) => {
        if (!state.gameRunning) {
            console.log('Click detected but game not running - press PLAY first');
            return;
        }
        
        const rect = canvas.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const laneWidth = canvas.width / state.lanes;
        const lane = Math.floor((x / rect.width) * state.lanes);
        
        console.log('Canvas clicked at lane:', lane);
        
        // Find and hit note in clicked lane
        let closestNote = null;
        let closestDistance = Infinity;
        const hitLineY = canvas.height - 80;
        
        for (const note of state.notes) {
            if (note.lane === lane && !note.hit && !note.missed && note.active) {
                const distance = Math.abs(note.y - hitLineY);
                if (distance < closestDistance) {
                    closestDistance = distance;
                    closestNote = note;
                }
            }
        }
        
        if (closestNote) {
            const hitQuality = closestNote.checkHit(performance.now());
            if (hitQuality) {
                hitNote(closestNote, hitQuality);
            }
        }
        
        flashLane(lane);
    });
    
    // Also add visual indicator
    console.log('Controls: A S D F keys (or click on lanes)');
    console.log('Key mappings:', state.keyMap);
    
    // Start game loop
    gameLoop();
}

/**
 * Handle keyboard input from JUCE (called from C++)
 */
window.handleKeyFromJUCE = function(keyCode) {
    console.log('=== KEY FROM JUCE ===');
    console.log('Key code:', keyCode);
    console.log('Game running:', state.gameRunning);
    
    processKeyPress(keyCode);
};

/**
 * Handle keyboard input from browser
 */
function handleKeyPress(e) {
    console.log('=== KEY FROM BROWSER ===');
    console.log('Key code:', e.code);
    console.log('Key:', e.key);
    
    if (e.code) {
        processKeyPress(e.code);
    }
    
    // Prevent default to avoid scrolling
    e.preventDefault();
}

/**
 * Process key press (works for both JUCE and browser)
 */
function processKeyPress(keyCode) {
    console.log('Processing key:', keyCode);
    
    // Flash the key indicator regardless of game state
    updateKeyIndicator(keyCode, true);
    
    if (!state.gameRunning) {
        console.log('Game not running - press PLAY button first!');
        return;
    }
    
    const lane = state.keyMap[keyCode];
    console.log('Lane for key:', lane, 'Max lanes:', state.lanes);
    
    if (lane === undefined || lane >= state.lanes) {
        console.log('Invalid lane or lane out of range');
        return;
    }
    
    // Find the closest note in this lane
    let closestNote = null;
    let closestDistance = Infinity;
    const hitLineY = canvas.height - 80;
    
    for (const note of state.notes) {
        if (note.lane === lane && !note.hit && !note.missed && note.active) {
            const distance = Math.abs(note.y - hitLineY);
            if (distance < closestDistance) {
                closestDistance = distance;
                closestNote = note;
            }
        }
    }
    
    if (closestNote) {
        console.log('Found note to hit:', closestNote.id, 'at y:', closestNote.y);
        const hitQuality = closestNote.checkHit(performance.now());
        console.log('Hit quality:', hitQuality);
        if (hitQuality) {
            hitNote(closestNote, hitQuality);
        }
    } else {
        console.log('No note found in lane', lane);
    }
    
    // Visual feedback for key press even if no note hit
    flashLane(lane);
}

/**
 * Flash lane on key press
 */
function flashLane(lane) {
    // Store the flash state for rendering
    if (!state.laneFlashes) {
        state.laneFlashes = {};
    }
    state.laneFlashes[lane] = performance.now();
}

/**
 * Update key indicator (for debugging)
 */
function updateKeyIndicator(keyCode, pressed) {
    if (!state.keyStates) {
        state.keyStates = {};
    }
    state.keyStates[keyCode] = pressed ? performance.now() : 0;
}

/**
 * Handle note hit
 */
function hitNote(note, quality) {
    note.hit = true;
    note.active = false;
    
    // Play the note
    playNote(note.noteValue);
    
    // Calculate score
    let points = 0;
    switch (quality) {
        case 'perfect':
            points = 300;
            break;
        case 'good':
            points = 200;
            break;
        case 'ok':
            points = 100;
            break;
    }
    
    // Apply combo multiplier
    state.combo++;
    if (state.combo > state.bestCombo) {
        state.bestCombo = state.combo;
    }
    
    const multiplier = Math.min(Math.floor(state.combo / 10) + 1, 8);
    state.score += points * multiplier;
    
    // Show feedback
    showFeedback(quality.toUpperCase());
    
    // Update UI
    updateScoreDisplay();
}

/**
 * Show hit feedback
 */
function showFeedback(text) {
    const feedback = document.getElementById('hitFeedback');
    if (!feedback) return;
    
    feedback.textContent = text;
    feedback.className = 'hit-feedback ' + text.toLowerCase();
    
    // Trigger animation
    setTimeout(() => feedback.classList.add('show'), 10);
    setTimeout(() => feedback.classList.remove('show'), 300);
    
    // Update combo display
    const comboValue = document.getElementById('comboValue');
    if (comboValue) {
        comboValue.classList.add('active');
        setTimeout(() => comboValue.classList.remove('active'), 300);
    }
}

/**
 * Update score display
 */
function updateScoreDisplay() {
    const scoreValue = document.getElementById('scoreValue');
    const comboValue = document.getElementById('comboValue');
    
    if (scoreValue) {
        scoreValue.textContent = state.score.toString().padStart(6, '0');
    }
    
    if (comboValue) {
        comboValue.textContent = 'x' + state.combo;
    }
}

/**
 * Spawn a new note
 */
function spawnNote(currentTime) {
    if (!state.gameRunning) return;
    
    // Choose random lane
    const lane = Math.floor(Math.random() * state.lanes);
    
    // Get scale
    const scaleName = scaleNames[state.scale];
    const scale = SCALES[scaleName];
    
    // Choose random note from scale (in a comfortable MIDI range)
    const octave = 4 + Math.floor(Math.random() * 2); // C4 to C5
    const scaleIndex = Math.floor(Math.random() * scale.length);
    const noteValue = 60 + (octave - 4) * 12 + scale[scaleIndex]; // MIDI note number
    
    const note = new Note(lane, currentTime, noteValue);
    state.notes.push(note);
    console.log('Spawned note in lane', lane, 'with MIDI value', noteValue, 'Total notes:', state.notes.length);
}

/**
 * Play a note (send to C++)
 */
function playNote(midiNote) {
    if (window.juce) {
        window.juce.postMessage({
            type: 'playNote',
            note: midiNote,
            velocity: 100,
            duration: 200
        });
    }
}

/**
 * Game loop
 */
let lastTime = performance.now();
let noteSpawnTimer = 0;

function gameLoop() {
    const currentTime = performance.now();
    const deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    // Clear canvas
    ctx.fillStyle = '#000000';
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    
    // Draw lanes
    drawLanes();
    
    // Draw hit line
    drawHitLine();
    
    if (state.gameRunning) {
        // Spawn notes based on tempo
        const beatInterval = (60 / state.tempo) * 1000; // milliseconds per beat
        noteSpawnTimer += deltaTime;
        
        // Spawn notes on each beat (with some variation)
        if (noteSpawnTimer >= beatInterval) {
            noteSpawnTimer = 0;
            
            // Spawn 1-2 notes depending on difficulty
            const noteCount = state.difficulty >= 2 ? Math.floor(Math.random() * 2) + 1 : 1;
            for (let i = 0; i < noteCount; i++) {
                spawnNote(currentTime);
            }
        }
        
        // Update notes
        state.notes = state.notes.filter(note => {
            const stillActive = note.update(currentTime, state.tempo);
            return stillActive;
        });
    }
    
    // Draw notes
    for (const note of state.notes) {
        note.draw();
    }
    
    // Draw status message if game not running
    if (!state.gameRunning) {
        ctx.fillStyle = '#00ffff';
        ctx.font = 'bold 28px monospace';
        ctx.textAlign = 'center';
        ctx.shadowBlur = 10;
        ctx.shadowColor = '#00ffff';
        ctx.fillText('PRESS PLAY TO START', canvas.width / 2, canvas.height / 2 - 60);
        ctx.font = 'bold 20px monospace';
        ctx.fillText('USE A S D F KEYS', canvas.width / 2, canvas.height / 2 - 10);
        ctx.fillStyle = '#ffff00';
        ctx.fillText('OR CLICK ON LANES', canvas.width / 2, canvas.height / 2 + 30);
        ctx.font = '14px monospace';
        ctx.fillStyle = '#888888';
        ctx.fillText('(Open console [F12] to debug keyboard)', canvas.width / 2, canvas.height / 2 + 70);
        ctx.shadowBlur = 0;
    }
    
    // Continue loop
    animationId = requestAnimationFrame(gameLoop);
}

/**
 * Draw lanes
 */
function drawLanes() {
    const laneWidth = canvas.width / state.lanes;
    const currentTime = performance.now();
    
    for (let i = 0; i < state.lanes; i++) {
        const x = i * laneWidth;
        
        // Lane divider
        ctx.strokeStyle = '#00ff00';
        ctx.lineWidth = 1;
        ctx.globalAlpha = 0.3;
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, canvas.height);
        ctx.stroke();
        ctx.globalAlpha = 1;
        
        // Lane key indicator at bottom with press feedback
        const keys = ['A', 'S', 'D', 'F', 'J', 'K', 'L', ';'];
        const keyCodes = ['KeyA', 'KeyS', 'KeyD', 'KeyF', 'KeyJ', 'KeyK', 'KeyL', 'Semicolon'];
        
        // Check if key is currently pressed (show for 200ms)
        let keyPressed = false;
        if (state.keyStates && state.keyStates[keyCodes[i]]) {
            const timeSincePress = currentTime - state.keyStates[keyCodes[i]];
            keyPressed = timeSincePress < 200;
        }
        
        // Draw key background
        if (keyPressed) {
            ctx.fillStyle = LANE_COLORS[i];
            ctx.globalAlpha = 0.5;
            ctx.fillRect(x + laneWidth / 2 - 20, canvas.height - 45, 40, 35);
            ctx.globalAlpha = 1;
        }
        
        // Draw key letter
        ctx.fillStyle = keyPressed ? '#ffffff' : LANE_COLORS[i];
        ctx.font = keyPressed ? 'bold 24px monospace' : 'bold 20px monospace';
        ctx.textAlign = 'center';
        ctx.fillText(keys[i], x + laneWidth / 2, canvas.height - 20);
    }
}

/**
 * Draw hit line
 */
function drawHitLine() {
    const y = canvas.height - 80;
    
    // Glowing line
    ctx.shadowBlur = 15;
    ctx.shadowColor = '#00ffff';
    ctx.strokeStyle = '#00ffff';
    ctx.lineWidth = 3;
    ctx.beginPath();
    ctx.moveTo(0, y);
    ctx.lineTo(canvas.width, y);
    ctx.stroke();
    ctx.shadowBlur = 0;
    
    // Hit zone boxes with flash effect
    const laneWidth = canvas.width / state.lanes;
    const currentTime = performance.now();
    
    for (let i = 0; i < state.lanes; i++) {
        const x = i * laneWidth;
        
        // Check if this lane should flash
        let alpha = 0.3;
        if (state.laneFlashes && state.laneFlashes[i]) {
            const timeSinceFlash = currentTime - state.laneFlashes[i];
            if (timeSinceFlash < 200) {
                alpha = 0.8 - (timeSinceFlash / 200) * 0.5;
            }
        }
        
        ctx.strokeStyle = LANE_COLORS[i];
        ctx.lineWidth = 2;
        ctx.globalAlpha = alpha;
        ctx.strokeRect(x + laneWidth / 2 - 25, y - 25, 50, 50);
        ctx.globalAlpha = 1;
    }
}

/**
 * Initialize controls
 */
document.addEventListener('DOMContentLoaded', () => {
    // Initialize knobs
    new MandelbrotKnob(document.getElementById('tempoKnob'), { 
        min: 60, max: 200, value: 120, 
        onChange: (v, p) => { 
            state.tempo = Math.round(v); 
            document.getElementById('tempoValue').textContent = `${state.tempo} BPM`; 
            updateDifficultyInfo();
            sendToPlugin(p, v); 
        } 
    });
    
    new MandelbrotKnob(document.getElementById('scaleKnob'), { 
        min: 0, max: scaleNames.length - 1, value: 0, step: 1,
        onChange: (v, p) => { 
            state.scale = Math.round(v); 
            document.getElementById('scaleValue').textContent = scaleNames[state.scale]; 
            sendToPlugin(p, state.scale); 
        } 
    });
    
    new MandelbrotKnob(document.getElementById('difficultyKnob'), { 
        min: 0, max: 3, value: 1, step: 1,
        onChange: (v, p) => { 
            state.difficulty = Math.round(v); 
            const diffs = ['Easy', 'Normal', 'Hard', 'Expert'];
            const lanes = [3, 4, 6, 8];
            state.lanes = lanes[state.difficulty];
            document.getElementById('difficultyValue').textContent = diffs[state.difficulty]; 
            updateDifficultyInfo();
            sendToPlugin(p, state.difficulty); 
        } 
    });
    
    new MandelbrotKnob(document.getElementById('volumeKnob'), { 
        min: -60, max: 12, value: 0,
        onChange: (v, p) => { 
            state.volume = v; 
            document.getElementById('volumeValue').textContent = `${v.toFixed(1)} dB`; 
            sendToPlugin(p, v); 
        } 
    });
    
    initializeBypassToggle();
    initializeGame();
});

function updateDifficultyInfo() {
    const difficultyInfo = document.getElementById('difficultyInfo');
    if (difficultyInfo) {
        difficultyInfo.textContent = `${state.lanes} Lanes • ${state.tempo} BPM`;
    }
}

function initializeBypassToggle() {
    const bypassButton = document.getElementById('bypassToggle');
    if (!bypassButton) return;
    
    bypassButton.onclick = function() {
        state.gameRunning = !state.gameRunning;
        console.log('Game running state changed to:', state.gameRunning);
        
        if (state.gameRunning) {
            // Reset game
            state.score = 0;
            state.combo = 0;
            state.notes = [];
            noteSpawnTimer = 0;
            updateScoreDisplay();
            console.log('Game started - press A S D F keys to hit notes!');
        } else {
            console.log('Game paused');
        }
        
        bypassButton.classList.toggle('active', state.gameRunning);
        bypassButton.querySelector('.bypass-text').textContent = state.gameRunning ? 'PAUSE' : 'PLAY';
        
        sendToPlugin('bypass', state.gameRunning ? 0.0 : 1.0);
        return false;
    };
}

function sendToPlugin(parameter, value) {
    if (window.juce) {
        window.juce.postMessage({
            type: 'parameterChange',
            parameter: parameter,
            value: value
        });
    }
}
