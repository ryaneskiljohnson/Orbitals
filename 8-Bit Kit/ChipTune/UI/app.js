/**
 * @fileoverview ChipTune - Classic Arcade Music Generator
 * @module ChipTune App
 * 
 * A Mario-style platformer where EVERYTHING generates music!
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

// Chord progressions
const CHORD_PROGRESSIONS = {
    'Major': [[0, 4, 7], [2, 5, 9], [4, 7, 11], [5, 9, 0]],
    'Minor': [[0, 3, 7], [2, 5, 8], [3, 7, 10], [5, 8, 0]],
    'Pentatonic': [[0, 4, 7], [2, 7, 9], [4, 9, 0]],
    'Blues': [[0, 3, 7], [3, 6, 10], [5, 7, 10]],
    'Dorian': [[0, 3, 7], [2, 5, 9], [3, 7, 10]],
    'Mixolydian': [[0, 4, 7], [2, 5, 9], [4, 7, 10]]
};

// Game state
const state = {
    tempo: 120,
    scale: 0,
    difficulty: 1,
    volume: 0,
    gameRunning: false,
    isDead: false,
    score: 0,
    combo: 0,
    bestCombo: 0,
    tokensCollected: 0,
    distance: 0,
    lives: 3,
    currentChord: 0,
    lastNoteTime: 0,
    activeNotes: new Set(),
    
    // Power-ups
    powerUps: {
        doubleJump: false,
        invincible: false,
        magnet: false,
        speedBoost: false,
        powerUpTime: 0
    },
    
    // Character state
    player: {
        x: 100,
        y: 400,
        width: 32,
        height: 32,
        velocityX: 0,
        velocityY: 0,
        speed: 3,
        jumpPower: -14,
        gravity: 0.8,
        onGround: false,
        facingRight: true,
        canJump: true,
        hasDoubleJump: false,
        invulnerable: false,
        invulnerableTime: 0,
        jumpCount: 0
    },
    
    // Game objects
    platforms: [],
    tokens: [],
    obstacles: [],
    enemies: [],
    powerUps: [],
    musicalNotes: [], // Visual note particles
    nextPlatformId: 0,
    nextTokenId: 0,
    nextObstacleId: 0,
    nextEnemyId: 0,
    nextPowerUpId: 0,
    cameraX: 0,
    
    // Game timing
    lastSpawnTime: 0,
    gameStartTime: 0,
    lastEnemySpawn: 0,
    lastPlatformSpawn: 0,
    lastPowerUpSpawn: 0,
    beatTime: 0,
    lastBeat: 0,
    
    // Musical sequence system
    sequenceChallenge: null,
    collectedSequence: [],
    sequenceReward: null,
    musicalBridges: [],
    nextBridgeId: 0,
    
    // Rhythm-based bonuses
    lastActionTime: 0,
    rhythmBonus: 1.0,
    rhythmStreak: 0,
    
    // Visual music representation
    frequencyBars: [],
    activeFrequencies: new Map(),
    
    // Boss challenges
    bossActive: false,
    bossSequence: [],
    bossHealth: 0,
    
    // Secret areas
    secretAreas: [],
    unlockedSecrets: new Set()
};

// Canvas and context
let canvas, ctx;
let animationId = null;

/**
 * Musical Note Particle (visual effect)
 */
class MusicalNote {
    constructor(x, y, noteValue, color = '#ffff00') {
        this.x = x;
        this.y = y;
        this.noteValue = noteValue;
        this.color = color;
        this.life = 60;
        this.velocityY = -2;
        this.alpha = 1.0;
        this.scale = 1.0;
    }
    
    update() {
        this.y += this.velocityY;
        this.life--;
        this.alpha = this.life / 60;
        this.velocityY *= 0.95;
        // Pulse to beat
        const beatInterval = (60 / state.tempo) * 1000;
        const beatPhase = (performance.now() % beatInterval) / beatInterval;
        this.scale = 1.0 + Math.sin(beatPhase * Math.PI * 2) * 0.2;
    }
    
    draw() {
        if (this.life <= 0) return;
        
        ctx.save();
        ctx.globalAlpha = this.alpha;
        ctx.translate(this.x - state.cameraX, this.y);
        ctx.scale(this.scale, this.scale);
        ctx.fillStyle = this.color;
        ctx.font = 'bold 20px monospace';
        ctx.textAlign = 'center';
        ctx.fillText('♪', 0, 0);
        ctx.restore();
    }
}

/**
 * Musical Bridge - appears when you play the right sequence
 */
class MusicalBridge {
    constructor(x, y, width, sequence) {
        this.id = state.nextBridgeId++;
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = 16;
        this.sequence = sequence; // Required sequence to activate
        this.active = false;
        this.alpha = 0.3;
        this.pulse = 0;
    }
    
    update() {
        this.x -= state.player.speed;
        this.pulse = Math.sin(Date.now() / 300) * 0.2;
        
        // Check if sequence matches
        if (!this.active && state.collectedSequence.length >= this.sequence.length) {
            let matches = true;
            for (let i = 0; i < this.sequence.length; i++) {
                const collectedNote = state.collectedSequence[state.collectedSequence.length - this.sequence.length + i];
                if (collectedNote !== this.sequence[i]) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                this.active = true;
                this.alpha = 1.0;
                // Play activation chord
                const scaleName = scaleNames[state.scale];
                const progression = CHORD_PROGRESSIONS[scaleName] || CHORD_PROGRESSIONS['Major'];
                const chord = progression[0];
                chord.forEach((note, index) => {
                    setTimeout(() => {
                        playNote(60 + 4 * 12 + note, 100, 300);
                    }, index * 30);
                });
                showFeedback('BRIDGE ACTIVATED!');
            }
        }
    }
    
    draw() {
        if (!this.active) {
            // Draw as ghostly outline
            const screenX = this.x - state.cameraX;
            if (screenX + this.width < 0 || screenX > canvas.width) return;
            
            ctx.save();
            ctx.globalAlpha = this.alpha;
            ctx.strokeStyle = '#00ffff';
            ctx.lineWidth = 2;
            ctx.setLineDash([5, 5]);
            ctx.strokeRect(screenX, this.y, this.width, this.height);
            ctx.setLineDash([]);
            
            // Show required sequence
            ctx.fillStyle = '#00ffff';
            ctx.font = '10px monospace';
            ctx.textAlign = 'center';
            ctx.fillText('PLAY SEQUENCE', screenX + this.width / 2, this.y - 10);
            ctx.restore();
            return;
        }
        
        const screenX = this.x - state.cameraX;
        if (screenX + this.width < 0 || screenX > canvas.width) return;
        
        const scale = 1.0 + this.pulse;
        const drawY = this.y;
        
        // Glowing bridge
        ctx.save();
        ctx.shadowBlur = 20;
        ctx.shadowColor = '#00ffff';
        ctx.fillStyle = `rgba(0, 255, 255, ${0.8 + this.pulse})`;
        ctx.fillRect(screenX, drawY, this.width, this.height);
        ctx.shadowBlur = 0;
        
        // Musical notes on bridge
        ctx.fillStyle = '#ffffff';
        ctx.font = 'bold 12px monospace';
        ctx.textAlign = 'center';
        for (let i = 0; i < 3; i++) {
            ctx.fillText('♪', screenX + (i + 1) * (this.width / 4), drawY + 10);
        }
        ctx.restore();
    }
    
    checkCollision(player) {
        if (!this.active) return false;
        
        return player.x + player.width > this.x &&
               player.x < this.x + this.width &&
               player.y + player.height > this.y &&
               player.y + player.height < this.y + this.height + 5 &&
               player.velocityY >= 0;
    }
}

/**
 * Power-up item
 */
class PowerUp {
    constructor(x, y, type) {
        this.id = state.nextPowerUpId++;
        this.x = x;
        this.y = y;
        this.width = 28;
        this.height = 28;
        this.type = type; // 'doubleJump', 'invincible', 'magnet', 'speedBoost'
        this.collected = false;
        this.rotation = 0;
        this.bobOffset = 0;
        this.pulse = 0;
    }
    
    update() {
        if (this.collected) return;
        
        this.x -= state.player.speed;
        this.rotation += 0.15;
        this.bobOffset = Math.sin(Date.now() / 200) * 4;
        this.pulse = Math.sin(Date.now() / 100) * 0.3 + 1;
    }
    
    draw() {
        if (this.collected) return;
        
        const screenX = this.x - state.cameraX;
        if (screenX + this.width < 0 || screenX > canvas.width) return;
        
        const drawY = this.y + this.bobOffset;
        
        ctx.save();
        ctx.translate(screenX + this.width / 2, drawY + this.height / 2);
        ctx.scale(this.pulse, this.pulse);
        ctx.rotate(this.rotation);
        
        // Glow
        ctx.shadowBlur = 20;
        const colors = {
            'doubleJump': '#00ffff',
            'invincible': '#ff00ff',
            'magnet': '#ffff00',
            'speedBoost': '#00ff00'
        };
        ctx.shadowColor = colors[this.type] || '#ffffff';
        
        // Power-up circle
        ctx.fillStyle = colors[this.type] || '#ffffff';
        ctx.beginPath();
        ctx.arc(0, 0, this.width / 2, 0, Math.PI * 2);
        ctx.fill();
        
        // Icon
        ctx.fillStyle = '#000000';
        ctx.font = 'bold 16px monospace';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        const icons = {
            'doubleJump': '↑↑',
            'invincible': '★',
            'magnet': '⚡',
            'speedBoost': '⚡'
        };
        ctx.fillText(icons[this.type] || '?', 0, 0);
        
        ctx.shadowBlur = 0;
        ctx.restore();
    }
    
    checkCollection(player) {
        if (this.collected) return false;
        
        const centerX = this.x + this.width / 2;
        const centerY = this.y + this.height / 2;
        const playerCenterX = player.x + player.width / 2;
        const playerCenterY = player.y + player.height / 2;
        
        const distance = Math.sqrt(
            Math.pow(centerX - playerCenterX, 2) + 
            Math.pow(centerY - playerCenterY, 2)
        );
        
        if (distance < (this.width / 2 + player.width / 2)) {
            this.collected = true;
            return true;
        }
        return false;
    }
}

/**
 * Platform object with musical properties
 */
class Platform {
    constructor(x, y, width, type = 'normal', noteValue = null) {
        this.id = state.nextPlatformId++;
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = 16;
        this.type = type; // 'normal', 'moving', 'breakable', 'bouncy', 'musical'
        this.color = '#8B4513';
        this.velocityX = 0;
        this.originalX = x;
        this.bounceOffset = 0;
        this.health = type === 'breakable' ? 1 : Infinity;
        this.noteValue = noteValue;
        this.lastPlayedNote = 0;
        this.musical = type === 'musical';
        
        if (type === 'moving') {
            this.velocityX = (Math.random() > 0.5 ? 1 : -1) * 0.5;
        } else if (type === 'bouncy') {
            this.color = '#00ff00';
        } else if (type === 'breakable') {
            this.color = '#ff8800';
        } else if (type === 'musical') {
            this.color = '#ff00ff';
        }
    }
    
    update() {
        this.x -= state.player.speed;
        this.originalX -= state.player.speed;
        
        if (this.type === 'moving') {
            this.x += this.velocityX;
            if (this.x < this.originalX - 30 || this.x > this.originalX + 30) {
                this.velocityX *= -1;
            }
        }
        
        if (this.type === 'bouncy') {
            this.bounceOffset = Math.sin(Date.now() / 200) * 3;
        }
        
        // Musical platforms play notes periodically
        if (this.type === 'musical' && this.noteValue) {
            const currentTime = performance.now();
            const beatInterval = (60 / state.tempo) * 1000;
            if (currentTime - this.lastPlayedNote >= beatInterval) {
                playNote(this.noteValue, 60, 100);
                this.lastPlayedNote = currentTime;
                // Visual note
                state.musicalNotes.push(new MusicalNote(
                    this.x + this.width / 2,
                    this.y - 10,
                    this.noteValue,
                    '#ff00ff'
                ));
            }
        }
    }
    
    draw() {
        const screenX = this.x - state.cameraX;
        if (screenX + this.width < 0 || screenX > canvas.width) return;
        
        const drawY = this.y + (this.type === 'bouncy' ? this.bounceOffset : 0);
        
        // Shadow
        ctx.fillStyle = 'rgba(0, 0, 0, 0.3)';
        ctx.fillRect(screenX + 2, drawY + this.height, this.width, 4);
        
        // Platform
        ctx.fillStyle = this.color;
        ctx.fillRect(screenX, drawY, this.width, this.height);
        
        // Border
        ctx.strokeStyle = '#654321';
        ctx.lineWidth = 2;
        ctx.strokeRect(screenX, drawY, this.width, this.height);
        
        // Highlight
        ctx.fillStyle = 'rgba(255, 255, 255, 0.2)';
        ctx.fillRect(screenX, drawY, this.width, 4);
        
        // Type indicator
        if (this.type === 'bouncy') {
            ctx.fillStyle = '#ffff00';
            ctx.font = 'bold 10px monospace';
            ctx.textAlign = 'center';
            ctx.fillText('↑', screenX + this.width / 2, drawY - 5);
        } else if (this.type === 'breakable') {
            ctx.fillStyle = '#ff0000';
            ctx.font = 'bold 10px monospace';
            ctx.textAlign = 'center';
            ctx.fillText('!', screenX + this.width / 2, drawY - 5);
        } else if (this.type === 'musical') {
            ctx.fillStyle = '#ffffff';
            ctx.font = 'bold 12px monospace';
            ctx.textAlign = 'center';
            ctx.fillText('♪', screenX + this.width / 2, drawY - 5);
        }
    }
    
    checkCollision(player) {
        const drawY = this.y + (this.type === 'bouncy' ? this.bounceOffset : 0);
        return player.x + player.width > this.x &&
               player.x < this.x + this.width &&
               player.y + player.height > drawY &&
               player.y + player.height < drawY + this.height + 5 &&
               player.velocityY >= 0;
    }
}

/**
 * Obstacle (wall or pit)
 */
class Obstacle {
    constructor(x, y, width, height, type) {
        this.id = state.nextObstacleId++;
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
        this.type = type; // 'wall', 'pit', 'spike'
    }
    
    update() {
        this.x -= state.player.speed;
    }
    
    draw() {
        const screenX = this.x - state.cameraX;
        if (screenX + this.width < 0 || screenX > canvas.width) return;
        
        if (this.type === 'wall') {
            ctx.fillStyle = '#654321';
            ctx.fillRect(screenX, this.y, this.width, this.height);
            ctx.strokeStyle = '#8B4513';
            ctx.lineWidth = 2;
            ctx.strokeRect(screenX, this.y, this.width, this.height);
        } else if (this.type === 'pit') {
            ctx.fillStyle = 'rgba(0, 0, 0, 0.5)';
            ctx.fillRect(screenX, this.y, this.width, this.height);
        } else if (this.type === 'spike') {
            ctx.fillStyle = '#ff0000';
            const spikeWidth = 10;
            for (let i = 0; i < this.width; i += spikeWidth) {
                ctx.beginPath();
                ctx.moveTo(screenX + i, this.y);
                ctx.lineTo(screenX + i + spikeWidth / 2, this.y - 10);
                ctx.lineTo(screenX + i + spikeWidth, this.y);
                ctx.closePath();
                ctx.fill();
            }
        }
    }
    
    checkCollision(player) {
        if (this.type === 'wall') {
            return player.x + player.width > this.x &&
                   player.x < this.x + this.width &&
                   player.y + player.height > this.y &&
                   player.y < this.y + this.height;
        } else if (this.type === 'spike') {
            return player.x + player.width > this.x &&
                   player.x < this.x + this.width &&
                   player.y + player.height > this.y - 10 &&
                   player.y < this.y;
        }
        return false;
    }
}

/**
 * Enemy creature with musical properties
 */
class Enemy {
    constructor(x, y, type) {
        this.id = state.nextEnemyId++;
        this.x = x;
        this.y = y;
        this.type = type; // 'goomba', 'fly', 'spike', 'patrol', 'drummer'
        this.width = 24;
        this.height = 24;
        this.velocityX = 0;
        this.velocityY = 0;
        this.animationFrame = 0;
        this.direction = -1;
        this.patrolDistance = 0;
        this.patrolStartX = x;
        this.onPlatform = null;
        this.lastBeatTime = 0;
        this.noteValue = null;
        
        // Assign musical note based on type
        const scaleName = scaleNames[state.scale];
        const scale = SCALES[scaleName];
        const octave = 3 + Math.floor(Math.random() * 2);
        const scaleIndex = Math.floor(Math.random() * scale.length);
        this.noteValue = 60 + (octave - 4) * 12 + scale[scaleIndex];
        
        if (type === 'goomba') {
            this.width = 28;
            this.height = 24;
            this.velocityX = -1.5;
        } else if (type === 'fly') {
            this.width = 20;
            this.height = 20;
            this.velocityX = -2;
            this.velocityY = Math.sin(Date.now() / 500) * 0.5;
            this.flyOffset = 0;
        } else if (type === 'spike') {
            this.width = 30;
            this.height = 30;
            this.velocityX = -1;
        } else if (type === 'patrol') {
            this.width = 26;
            this.height = 26;
            this.velocityX = -1;
            this.patrolDistance = 80;
        } else if (type === 'drummer') {
            this.width = 28;
            this.height = 28;
            this.velocityX = -1;
        }
    }
    
    update() {
        this.x -= state.player.speed;
        this.patrolStartX -= state.player.speed;
        
        // Drummer enemies play beats
        if (this.type === 'drummer') {
            const currentTime = performance.now();
            const beatInterval = (60 / state.tempo) * 1000;
            if (currentTime - this.lastBeatTime >= beatInterval * 2) {
                playNote(this.noteValue, 80, 50);
                this.lastBeatTime = currentTime;
                state.musicalNotes.push(new MusicalNote(
                    this.x + this.width / 2,
                    this.y - 10,
                    this.noteValue,
                    '#ff0000'
                ));
            }
        }
        
        if (this.type === 'goomba' || this.type === 'spike' || this.type === 'drummer') {
            this.velocityY += 0.8;
            this.y += this.velocityY;
            
            const groundY = canvas.height - 50;
            if (this.y + this.height >= groundY) {
                this.y = groundY - this.height;
                this.velocityY = 0;
            }
            
            for (const platform of state.platforms) {
                if (platform.checkCollision({x: this.x, y: this.y, width: this.width, height: this.height, velocityY: this.velocityY})) {
                    this.y = platform.y - this.height;
                    this.velocityY = 0;
                    this.onPlatform = platform;
                    break;
                }
            }
        } else if (this.type === 'fly') {
            this.flyOffset = Math.sin(Date.now() / 500) * 15;
            this.y += this.velocityY;
            if (this.y < 100 || this.y > 300) {
                this.velocityY *= -1;
            }
        } else if (this.type === 'patrol') {
            this.velocityX = this.direction * 1.5;
            this.x += this.velocityX;
            
            if (this.x < this.patrolStartX - this.patrolDistance) {
                this.direction = 1;
            } else if (this.x > this.patrolStartX + this.patrolDistance) {
                this.direction = -1;
            }
        }
        
        this.animationFrame += 0.2;
    }
    
    draw() {
        const screenX = this.x - state.cameraX;
        if (screenX + this.width < 0 || screenX > canvas.width) return;
        
        const drawY = this.y + (this.type === 'fly' ? this.flyOffset : 0);
        
        if (this.type === 'goomba') {
            ctx.fillStyle = '#8B4513';
            ctx.beginPath();
            ctx.ellipse(screenX + this.width / 2, drawY + this.height - 8, this.width / 2, 8, 0, 0, Math.PI * 2);
            ctx.fill();
            
            ctx.fillStyle = '#654321';
            ctx.beginPath();
            ctx.ellipse(screenX + this.width / 2, drawY + this.height - 12, this.width / 2 - 2, 6, 0, 0, Math.PI * 2);
            ctx.fill();
            
            const eyeOffset = Math.sin(this.animationFrame) * 2;
            ctx.fillStyle = '#ffffff';
            ctx.beginPath();
            ctx.arc(screenX + this.width / 2 - 6, drawY + this.height - 10 + eyeOffset, 3, 0, Math.PI * 2);
            ctx.arc(screenX + this.width / 2 + 6, drawY + this.height - 10 + eyeOffset, 3, 0, Math.PI * 2);
            ctx.fill();
            
            ctx.fillStyle = '#000000';
            ctx.beginPath();
            ctx.arc(screenX + this.width / 2 - 6, drawY + this.height - 10 + eyeOffset, 2, 0, Math.PI * 2);
            ctx.arc(screenX + this.width / 2 + 6, drawY + this.height - 10 + eyeOffset, 2, 0, Math.PI * 2);
            ctx.fill();
        } else if (this.type === 'fly') {
            ctx.fillStyle = '#000000';
            ctx.beginPath();
            ctx.ellipse(screenX + this.width / 2, drawY + this.height / 2, this.width / 2, this.height / 2, 0, 0, Math.PI * 2);
            ctx.fill();
            
            const wingOffset = Math.sin(this.animationFrame * 2) * 3;
            ctx.fillStyle = 'rgba(200, 200, 200, 0.6)';
            ctx.beginPath();
            ctx.ellipse(screenX + this.width / 2 - 5, drawY + this.height / 2, 6, 8, wingOffset, 0, Math.PI * 2);
            ctx.ellipse(screenX + this.width / 2 + 5, drawY + this.height / 2, 6, 8, -wingOffset, 0, Math.PI * 2);
            ctx.fill();
            
            ctx.fillStyle = '#ff0000';
            ctx.beginPath();
            ctx.arc(screenX + this.width / 2 - 4, drawY + this.height / 2 - 2, 2, 0, Math.PI * 2);
            ctx.arc(screenX + this.width / 2 + 4, drawY + this.height / 2 - 2, 2, 0, Math.PI * 2);
            ctx.fill();
        } else if (this.type === 'spike') {
            ctx.fillStyle = '#ff0000';
            ctx.beginPath();
            ctx.arc(screenX + this.width / 2, drawY + this.height / 2, this.width / 2, 0, Math.PI * 2);
            ctx.fill();
            
            ctx.strokeStyle = '#8B0000';
            ctx.lineWidth = 2;
            ctx.stroke();
            
            for (let i = 0; i < 8; i++) {
                const angle = (i / 8) * Math.PI * 2;
                const spikeLength = 6;
                ctx.beginPath();
                ctx.moveTo(
                    screenX + this.width / 2 + Math.cos(angle) * (this.width / 2),
                    drawY + this.height / 2 + Math.sin(angle) * (this.height / 2)
                );
                ctx.lineTo(
                    screenX + this.width / 2 + Math.cos(angle) * (this.width / 2 + spikeLength),
                    drawY + this.height / 2 + Math.sin(angle) * (this.height / 2 + spikeLength)
                );
                ctx.stroke();
            }
        } else if (this.type === 'patrol') {
            ctx.fillStyle = '#666666';
            ctx.fillRect(screenX, drawY, this.width, this.height);
            
            ctx.fillStyle = '#888888';
            ctx.fillRect(screenX + 4, drawY, 18, 10);
            
            ctx.fillStyle = '#ff0000';
            ctx.fillRect(screenX + 8, drawY + 2, 3, 3);
            ctx.fillRect(screenX + 15, drawY + 2, 3, 3);
            
            ctx.fillStyle = '#ffff00';
            ctx.beginPath();
            if (this.direction > 0) {
                ctx.moveTo(screenX + this.width - 4, drawY + this.height / 2);
                ctx.lineTo(screenX + this.width - 10, drawY + this.height / 2 - 3);
                ctx.lineTo(screenX + this.width - 10, drawY + this.height / 2 + 3);
            } else {
                ctx.moveTo(screenX + 4, drawY + this.height / 2);
                ctx.lineTo(screenX + 10, drawY + this.height / 2 - 3);
                ctx.lineTo(screenX + 10, drawY + this.height / 2 + 3);
            }
            ctx.closePath();
            ctx.fill();
        } else if (this.type === 'drummer') {
            // Draw drummer enemy
            ctx.fillStyle = '#8B4513';
            ctx.fillRect(screenX, drawY + 8, this.width, this.height - 8);
            
            ctx.fillStyle = '#654321';
            ctx.beginPath();
            ctx.arc(screenX + this.width / 2, drawY + 8, this.width / 2, 0, Math.PI * 2);
            ctx.fill();
            
            // Drum
            ctx.fillStyle = '#ff0000';
            ctx.beginPath();
            ctx.arc(screenX + this.width / 2, drawY + this.height - 5, 8, 0, Math.PI * 2);
            ctx.fill();
            
            // Drumsticks
            const stickAngle = Math.sin(this.animationFrame * 3) * 0.3;
            ctx.strokeStyle = '#8B4513';
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.moveTo(screenX + this.width / 2, drawY + this.height - 5);
            ctx.lineTo(
                screenX + this.width / 2 + Math.cos(stickAngle) * 10,
                drawY + this.height - 5 + Math.sin(stickAngle) * 10
            );
            ctx.stroke();
        }
    }
    
    checkCollision(player) {
        if (player.invulnerable || state.powerUps.invincible) return false;
        
        return player.x + player.width > this.x &&
               player.x < this.x + this.width &&
               player.y + player.height > this.y &&
               player.y < this.y + this.height;
    }
}

/**
 * NNAudio Token
 */
class Token {
    constructor(x, y, noteValue) {
        this.id = state.nextTokenId++;
        this.x = x;
        this.y = y;
        this.width = 24;
        this.height = 24;
        this.noteValue = noteValue;
        this.collected = false;
        this.rotation = 0;
        this.bobOffset = 0;
    }
    
    update() {
        if (this.collected) return;
        
        this.x -= state.player.speed;
        this.rotation += 0.1;
        this.bobOffset = Math.sin(Date.now() / 200) * 3;
        
        // Sequence token glow
        if (this.isSequenceToken) {
            this.glow = true;
        }
        
        // Magnet effect
        if (state.powerUps.magnet) {
            const dx = state.player.x - this.x;
            const dy = state.player.y - this.y;
            const distance = Math.sqrt(dx * dx + dy * dy);
            if (distance < 150) {
                this.x += (dx / distance) * 3;
                this.y += (dy / distance) * 3;
            }
        }
    }
    
    draw() {
        if (this.collected) return;
        
        const screenX = this.x - state.cameraX;
        if (screenX + this.width < 0 || screenX > canvas.width) return;
        
        const drawY = this.y + this.bobOffset;
        
        ctx.save();
        ctx.translate(screenX + this.width / 2, drawY + this.height / 2);
        ctx.rotate(this.rotation);
        
        // Enhanced glow for sequence tokens
        if (this.isSequenceToken) {
            const pulse = Math.sin(Date.now() / 200) * 0.3 + 1;
            ctx.shadowBlur = 25;
            ctx.shadowColor = '#00ffff';
            ctx.scale(pulse, pulse);
        } else {
            ctx.shadowBlur = 15;
            ctx.shadowColor = '#ffff00';
        }
        
        ctx.fillStyle = this.isSequenceToken ? '#00ffff' : '#ffd700';
        ctx.beginPath();
        ctx.arc(0, 0, this.width / 2, 0, Math.PI * 2);
        ctx.fill();
        
        ctx.fillStyle = '#000000';
        ctx.font = 'bold 12px monospace';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText('NN', 0, 0);
        
        ctx.shadowBlur = 0;
        ctx.restore();
    }
    
    checkCollection(player) {
        if (this.collected) return false;
        
        const centerX = this.x + this.width / 2;
        const centerY = this.y + this.height / 2;
        const playerCenterX = player.x + player.width / 2;
        const playerCenterY = player.y + player.height / 2;
        
        const distance = Math.sqrt(
            Math.pow(centerX - playerCenterX, 2) + 
            Math.pow(centerY - playerCenterY, 2)
        );
        
        if (distance < (this.width / 2 + player.width / 2)) {
            this.collected = true;
            return true;
        }
        return false;
    }
}

/**
 * Draw the player character with NN hat
 */
function drawPlayer() {
    const p = state.player;
    const screenX = p.x - state.cameraX;
    const y = p.y;
    
    if ((p.invulnerable || state.powerUps.invincible) && Math.floor(p.invulnerableTime / 5) % 2) {
        ctx.globalAlpha = 0.5;
    }
    
    // Power-up visual effects
    if (state.powerUps.invincible) {
        ctx.shadowBlur = 20;
        ctx.shadowColor = '#ff00ff';
    } else if (state.powerUps.speedBoost) {
        ctx.shadowBlur = 15;
        ctx.shadowColor = '#00ff00';
    }
    
    // Shadow
    ctx.fillStyle = 'rgba(0, 0, 0, 0.3)';
    ctx.beginPath();
    ctx.ellipse(screenX + p.width / 2, y + p.height + 5, p.width / 3, 5, 0, 0, Math.PI * 2);
    ctx.fill();
    
    // Body (red overalls)
    ctx.fillStyle = '#ff0000';
    ctx.fillRect(screenX + 8, y + 16, 16, 16);
    
    // Shirt (blue)
    ctx.fillStyle = '#0000ff';
    ctx.fillRect(screenX + 8, y + 8, 16, 12);
    
    // Arms
    ctx.fillStyle = '#ffdbac';
    ctx.fillRect(screenX + 4, y + 12, 4, 12);
    ctx.fillRect(screenX + 24, y + 12, 4, 12);
    
    // Head
    ctx.fillStyle = '#ffdbac';
    ctx.fillRect(screenX + 10, y + 2, 12, 10);
    
    // Eyes
    ctx.fillStyle = '#000000';
    ctx.fillRect(screenX + 12, y + 4, 2, 2);
    ctx.fillRect(screenX + 18, y + 4, 2, 2);
    
    // Hat with NN logo
    ctx.fillStyle = '#ff0000';
    ctx.fillRect(screenX + 6, y, 20, 6);
    ctx.fillRect(screenX + 6, y - 2, 20, 2);
    
    // NN text on hat
    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 8px monospace';
    ctx.textAlign = 'center';
    ctx.fillText('NN', screenX + p.width / 2, y + 4);
    
    // Legs
    ctx.fillStyle = '#0000ff';
    ctx.fillRect(screenX + 10, y + 28, 4, 4);
    ctx.fillRect(screenX + 18, y + 28, 4, 4);
    
    // Shoes
    ctx.fillStyle = '#8B4513';
    ctx.fillRect(screenX + 8, y + 30, 6, 2);
    ctx.fillRect(screenX + 18, y + 30, 6, 2);
    
    ctx.globalAlpha = 1.0;
    ctx.shadowBlur = 0;
}

/**
 * Initialize the game
 */
function initializeGame() {
    canvas = document.getElementById('gameCanvas');
    if (!canvas) return;
    
    ctx = canvas.getContext('2d');
    canvas.width = 600;
    canvas.height = 600;
    
    window.addEventListener('keydown', handleKeyPress, true);
    document.addEventListener('keydown', handleKeyPress, true);
    canvas.addEventListener('click', handleJump);
    
    canvas.tabIndex = 1000;
    canvas.style.outline = 'none';
    
    setTimeout(() => canvas.focus(), 100);
    
    gameLoop();
}

/**
 * Handle keyboard input from JUCE (called from C++)
 */
window.handleKeyFromJUCE = function(keyCode) {
    if (keyCode === 'Space') {
        handleJump();
    }
};

function handleKeyPress(e) {
    if (e.code === 'Space' || e.key === ' ') {
        e.preventDefault();
        handleJump();
    }
}

function handleJump() {
    if (!state.gameRunning || state.isDead) return;
    
    const p = state.player;
    const currentTime = performance.now();
    const beatInterval = (60 / state.tempo) * 1000;
    
    // Check rhythm timing for bonus
    const timeSinceLastBeat = (currentTime - state.lastBeat) % beatInterval;
    const isOnBeat = timeSinceLastBeat < beatInterval * 0.2 || timeSinceLastBeat > beatInterval * 0.8;
    
    // Normal jump
    if (p.onGround && p.canJump) {
        p.velocityY = p.jumpPower;
        p.onGround = false;
        p.canJump = false;
        p.jumpCount = 1;
        p.hasDoubleJump = state.powerUps.doubleJump;
        
        // Play jump note
        const scaleName = scaleNames[state.scale];
        const scale = SCALES[scaleName];
        const noteValue = 60 + scale[Math.floor(Math.random() * scale.length)];
        
        // Rhythm bonus
        let velocity = 70;
        if (isOnBeat) {
            velocity = 100;
            state.rhythmStreak++;
            state.rhythmBonus = Math.min(2.0, 1.0 + state.rhythmStreak * 0.1);
            if (state.rhythmStreak > 3) {
                showFeedback('RHYTHM BONUS!');
            }
        } else {
            state.rhythmStreak = 0;
            state.rhythmBonus = 1.0;
        }
        
        playNote(noteValue, velocity, 100);
        state.musicalNotes.push(new MusicalNote(
            p.x + p.width / 2,
            p.y,
            noteValue,
            isOnBeat ? '#00ff00' : '#00ffff'
        ));
        
        state.lastActionTime = currentTime;
    } 
    // Double jump
    else if (p.hasDoubleJump && p.jumpCount < 2) {
        p.velocityY = p.jumpPower * 0.9;
        p.jumpCount++;
        p.hasDoubleJump = false;
        
        // Play double jump note (higher)
        const scaleName = scaleNames[state.scale];
        const scale = SCALES[scaleName];
        const noteValue = 72 + scale[Math.floor(Math.random() * scale.length)];
        
        const velocity = isOnBeat ? 100 : 80;
        playNote(noteValue, velocity, 100);
        state.musicalNotes.push(new MusicalNote(
            p.x + p.width / 2,
            p.y,
            noteValue,
            isOnBeat ? '#00ff00' : '#00ffff'
        ));
        
        state.lastActionTime = currentTime;
    }
}

/**
 * Spawn game objects
 */
function spawnObjects() {
    if (!state.gameRunning || state.isDead) return;
    
    const currentTime = performance.now();
    const beatInterval = (60 / state.tempo) * 1000;
    const difficultyMultiplier = 1 + (state.difficulty * 0.3);
    
    // Update beat timing
    const timeSinceLastBeat = (currentTime - state.lastBeat) % beatInterval;
    if (timeSinceLastBeat < 50) { // New beat detected
        state.lastBeat = currentTime;
        // Visual beat pulse
        state.beatTime = 0;
    }
    
    // Spawn platforms ON THE BEAT for rhythm-based gameplay
    const beatPhase = (currentTime - state.lastBeat) / beatInterval;
    const shouldSpawnOnBeat = beatPhase < 0.1 && currentTime - state.lastPlatformSpawn >= beatInterval * 0.5;
    
    if (shouldSpawnOnBeat || currentTime - state.lastPlatformSpawn >= beatInterval * 1.5 / difficultyMultiplier) {
        state.lastPlatformSpawn = currentTime;
        
        const platformX = state.cameraX + canvas.width + 50;
        const platformY = 450 - Math.floor(Math.random() * 4) * 50;
        const platformWidth = 80 + Math.random() * 100;
        
        let platformType = 'normal';
        const typeRoll = Math.random();
        if (typeRoll > 0.75) {
            platformType = 'moving';
        } else if (typeRoll > 0.6) {
            platformType = 'bouncy';
        } else if (typeRoll > 0.45) {
            platformType = 'breakable';
        } else if (typeRoll > 0.3) {
            platformType = 'musical';
        }
        
        // Assign note to musical platform
        let noteValue = null;
        if (platformType === 'musical') {
            const scaleName = scaleNames[state.scale];
            const scale = SCALES[scaleName];
            const octave = 4 + Math.floor(Math.random() * 2);
            const scaleIndex = Math.floor(Math.random() * scale.length);
            noteValue = 60 + (octave - 4) * 12 + scale[scaleIndex];
        }
        
        const platform = new Platform(platformX, platformY, platformWidth, platformType, noteValue);
        state.platforms.push(platform);
        
        // Spawn token on platform
        if (Math.random() > 0.4) {
            const scaleName = scaleNames[state.scale];
            const scale = SCALES[scaleName];
            const octave = 4 + Math.floor(Math.random() * 2);
            const scaleIndex = Math.floor(Math.random() * scale.length);
            const tokenNoteValue = 60 + (octave - 4) * 12 + scale[scaleIndex];
            
            const token = new Token(
                platformX + platformWidth / 2 - 12,
                platformY - 30,
                tokenNoteValue
            );
            state.tokens.push(token);
        }
    }
    
    // Spawn enemies
    if (currentTime - state.lastEnemySpawn >= beatInterval * 2 / difficultyMultiplier) {
        state.lastEnemySpawn = currentTime;
        
        const enemyX = state.cameraX + canvas.width + 50;
        const enemyY = 450 - Math.floor(Math.random() * 3) * 50;
        
        const enemyTypes = ['goomba', 'fly', 'spike', 'patrol', 'drummer'];
        const enemyType = enemyTypes[Math.floor(Math.random() * enemyTypes.length)];
        
        const enemy = new Enemy(enemyX, enemyY, enemyType);
        state.enemies.push(enemy);
    }
    
    // Spawn power-ups
    if (currentTime - state.lastPowerUpSpawn >= beatInterval * 8) {
        state.lastPowerUpSpawn = currentTime;
        
        if (Math.random() > 0.7) {
            const powerUpX = state.cameraX + canvas.width + 50;
            const powerUpY = 400 - Math.floor(Math.random() * 3) * 50;
            const powerUpTypes = ['doubleJump', 'invincible', 'magnet', 'speedBoost'];
            const powerUpType = powerUpTypes[Math.floor(Math.random() * powerUpTypes.length)];
            
            const powerUp = new PowerUp(powerUpX, powerUpY, powerUpType);
            state.powerUps.push(powerUp);
        }
    }
    
    // Spawn obstacles
    if (currentTime - state.lastSpawnTime >= beatInterval * 3 / difficultyMultiplier) {
        state.lastSpawnTime = currentTime;
        
        const obstacleX = state.cameraX + canvas.width + 50;
        const obstacleY = 450 - Math.floor(Math.random() * 3) * 50;
        
        const obstacleTypes = ['wall', 'pit', 'spike'];
        const obstacleType = obstacleTypes[Math.floor(Math.random() * obstacleTypes.length)];
        
        let obstacle;
        if (obstacleType === 'wall') {
            obstacle = new Obstacle(obstacleX, obstacleY - 40, 30, 40, 'wall');
        } else if (obstacleType === 'pit') {
            obstacle = new Obstacle(obstacleX, obstacleY, 60, 100, 'pit');
        } else if (obstacleType === 'spike') {
            obstacle = new Obstacle(obstacleX, obstacleY, 40, 10, 'spike');
        }
        
        if (obstacle) {
            state.obstacles.push(obstacle);
        }
    }
    
    // Spawn musical bridges periodically
    if (currentTime - state.lastSpawnTime >= beatInterval * 10 && Math.random() > 0.8) {
        const bridgeX = state.cameraX + canvas.width + 200;
        const bridgeY = 400;
        const bridgeWidth = 150;
        
        // Generate a sequence challenge (3-4 notes)
        const scaleName = scaleNames[state.scale];
        const scale = SCALES[scaleName];
        const sequenceLength = 3 + Math.floor(Math.random() * 2);
        const sequence = [];
        for (let i = 0; i < sequenceLength; i++) {
            const scaleIndex = Math.floor(Math.random() * scale.length);
            sequence.push(scale[scaleIndex]);
        }
        
        const bridge = new MusicalBridge(bridgeX, bridgeY, bridgeWidth, sequence);
        state.musicalBridges.push(bridge);
        state.lastSpawnTime = currentTime;
    }
    
    // Spawn sequence challenge tokens
    if (state.sequenceChallenge && currentTime - state.lastSpawnTime >= beatInterval * 2) {
        // Spawn special sequence tokens
        const tokenX = state.cameraX + canvas.width + 50;
        const tokenY = 400 - Math.floor(Math.random() * 3) * 50;
        
        const scaleName = scaleNames[state.scale];
        const scale = SCALES[scaleName];
        const nextNoteInSequence = state.sequenceChallenge.sequence[state.collectedSequence.length];
        if (nextNoteInSequence !== undefined) {
            const noteValue = 60 + 4 * 12 + nextNoteInSequence;
            const token = new Token(tokenX, tokenY, noteValue);
            token.isSequenceToken = true;
            token.glow = true;
            state.tokens.push(token);
        }
    }
}

/**
 * Update player physics
 */
function updatePlayer() {
    const p = state.player;
    
    // Update power-ups
    if (state.powerUps.powerUpTime > 0) {
        state.powerUps.powerUpTime--;
        if (state.powerUps.powerUpTime <= 0) {
            state.powerUps.doubleJump = false;
            state.powerUps.invincible = false;
            state.powerUps.magnet = false;
            state.powerUps.speedBoost = false;
        }
    }
    
    // Speed boost
    const currentSpeed = state.powerUps.speedBoost ? p.speed * 1.5 : p.speed;
    
    // Update invulnerability
    if (p.invulnerable) {
        p.invulnerableTime--;
        if (p.invulnerableTime <= 0) {
            p.invulnerable = false;
        }
    }
    
    // Apply gravity
    p.velocityY += p.gravity;
    
    // Update position
    p.x += p.velocityX;
    p.y += p.velocityY;
    
    state.distance = Math.floor((p.x - 100) / 10);
    
    // Ground collision
    const groundY = canvas.height - 50;
    if (p.y + p.height >= groundY) {
        p.y = groundY - p.height;
        p.velocityY = 0;
        p.onGround = true;
        p.canJump = true;
        p.jumpCount = 0;
        
        if (state.combo > 0) {
            state.combo = 0;
            updateScoreDisplay();
        }
    } else {
        p.onGround = false;
    }
    
    // Platform collisions
    for (const platform of state.platforms) {
        if (platform.checkCollision(p)) {
            p.y = platform.y - p.height;
            p.velocityY = 0;
            p.onGround = true;
            p.canJump = true;
            p.jumpCount = 0;
            
            // Check rhythm timing
            const currentTime = performance.now();
            const beatInterval = (60 / state.tempo) * 1000;
            const timeSinceLastBeat = (currentTime - state.lastBeat) % beatInterval;
            const isOnBeat = timeSinceLastBeat < beatInterval * 0.2 || timeSinceLastBeat > beatInterval * 0.8;
            
            // Bouncy platform
            if (platform.type === 'bouncy') {
                p.velocityY = p.jumpPower * 1.3;
                p.onGround = false;
                p.canJump = false;
                const velocity = isOnBeat ? 110 : 90;
                playNote(platform.noteValue || 72, velocity, 100);
                if (isOnBeat) {
                    state.rhythmStreak++;
                    state.rhythmBonus = Math.min(2.0, 1.0 + state.rhythmStreak * 0.1);
                }
            }
            
            // Breakable platform
            if (platform.type === 'breakable') {
                platform.health--;
                if (platform.health <= 0) {
                    state.platforms = state.platforms.filter(pl => pl.id !== platform.id);
                    // Play break sound (low note)
                    playNote(48, 100, 50);
                }
            }
            
            // Musical platform plays note on landing
            if (platform.type === 'musical' && platform.noteValue) {
                const velocity = isOnBeat ? 120 : 100;
                playNote(platform.noteValue, velocity, 200);
                state.musicalNotes.push(new MusicalNote(
                    p.x + p.width / 2,
                    p.y,
                    platform.noteValue,
                    isOnBeat ? '#00ff00' : '#ff00ff'
                ));
            } else if (platform.type === 'normal') {
                // Normal landing note
                const scaleName = scaleNames[state.scale];
                const scale = SCALES[scaleName];
                const noteValue = 60 + scale[Math.floor(Math.random() * scale.length)];
                const velocity = isOnBeat ? 100 : 80;
                playNote(noteValue, velocity, 100);
            }
            
            break;
        }
    }
    
    // Musical bridge collisions
    for (const bridge of state.musicalBridges) {
        if (bridge.checkCollision(p)) {
            p.y = bridge.y - p.height;
            p.velocityY = 0;
            p.onGround = true;
            p.canJump = true;
            p.jumpCount = 0;
            
            // Play bridge note
            const scaleName = scaleNames[state.scale];
            const scale = SCALES[scaleName];
            const noteValue = 60 + 5 * 12 + scale[Math.floor(Math.random() * scale.length)];
            playNote(noteValue, 100, 200);
            break;
        }
    }
    
    // Obstacle collisions
    for (const obstacle of state.obstacles) {
        if (obstacle.checkCollision(p)) {
            if (obstacle.type === 'wall') {
                p.x = obstacle.x - p.width;
                p.velocityX = -2;
                // Play collision note (dissonant)
                playNote(45, 60, 50);
            } else if (obstacle.type === 'spike') {
                die();
                return;
            }
        }
    }
    
    // Enemy collisions
    for (const enemy of state.enemies) {
        if (enemy.checkCollision(p)) {
            // Play enemy hit note before dying
            if (enemy.noteValue) {
                playNote(enemy.noteValue, 120, 100);
            }
            die();
            return;
        }
    }
    
    // Power-up collection
    for (const powerUp of state.powerUps) {
        if (powerUp.checkCollection(p)) {
            // Activate power-up
            state.powerUps[powerUp.type] = true;
            state.powerUps.powerUpTime = 600; // 10 seconds at 60fps
            
            // Play power-up chord
            const scaleName = scaleNames[state.scale];
            const progression = CHORD_PROGRESSIONS[scaleName] || CHORD_PROGRESSIONS['Major'];
            const chord = progression[state.currentChord % progression.length];
            const octave = 4;
            chord.forEach((note, index) => {
                setTimeout(() => {
                    playNote(60 + octave * 12 + note, 80, 200);
                }, index * 50);
            });
            state.currentChord++;
            
            showFeedback('POWER-UP!');
        }
    }
    
    if (p.x < state.cameraX) {
        p.x = state.cameraX;
    }
    
    if (p.y > canvas.height) {
        // Play fall note
        playNote(36, 100, 200);
        die();
        return;
    }
    
    state.cameraX = p.x - 150;
    if (state.cameraX < 0) state.cameraX = 0;
}

/**
 * Player death
 */
function die() {
    if (state.isDead || (state.player.invulnerable && !state.powerUps.invincible)) return;
    
    state.isDead = true;
    state.lives--;
    state.combo = 0;
    
    // Play death chord (dissonant)
    playNote(36, 120, 300);
    playNote(39, 120, 300);
    playNote(42, 120, 300);
    
    showFeedback('GAME OVER!');
    updateScoreDisplay();
    
    setTimeout(() => {
        if (state.lives > 0) {
            respawn();
        } else {
            gameOver();
        }
    }, 2000);
}

/**
 * Respawn player
 */
function respawn() {
    state.isDead = false;
    state.player.x = 100;
    state.player.y = 400;
    state.player.velocityY = 0;
    state.player.velocityX = 0;
    state.player.invulnerable = true;
    state.player.invulnerableTime = 120;
    state.cameraX = 0;
    state.combo = 0;
    
    // Play respawn note
    const scaleName = scaleNames[state.scale];
    const scale = SCALES[scaleName];
    playNote(60 + scale[0], 100, 200);
    
    state.enemies = state.enemies.filter(e => e.x > state.cameraX + canvas.width);
    state.obstacles = state.obstacles.filter(o => o.x > state.cameraX + canvas.width);
}

/**
 * Game over
 */
function gameOver() {
    state.gameRunning = false;
    state.isDead = false;
    state.lives = 3;
    
    // Play game over chord
    setTimeout(() => playNote(36, 100, 500), 0);
    setTimeout(() => playNote(32, 100, 500), 200);
    
    state.platforms = [];
    state.tokens = [];
    state.obstacles = [];
    state.enemies = [];
    state.powerUps = [];
    state.score = 0;
    state.combo = 0;
    state.tokensCollected = 0;
    state.distance = 0;
    
    const bypassButton = document.getElementById('bypassToggle');
    if (bypassButton) {
        bypassButton.classList.remove('active');
        bypassButton.querySelector('.bypass-text').textContent = 'PLAY';
    }
}

/**
 * Check token collection
 */
function checkTokenCollection() {
    for (const token of state.tokens) {
        if (token.checkCollection(state.player)) {
            state.tokensCollected++;
            state.combo++;
            if (state.combo > state.bestCombo) {
                state.bestCombo = state.combo;
            }
            
            // Sequence challenge system
            if (token.isSequenceToken) {
                const scaleName = scaleNames[state.scale];
                const scale = SCALES[scaleName];
                const noteIndex = scale.indexOf(token.noteValue % 12 - (Math.floor(token.noteValue / 12) - 4) * 12);
                
                if (noteIndex !== -1) {
                    state.collectedSequence.push(scale[noteIndex]);
                    
                    // Check if sequence is complete
                    if (state.sequenceChallenge && 
                        state.collectedSequence.length >= state.sequenceChallenge.sequence.length) {
                        let matches = true;
                        for (let i = 0; i < state.sequenceChallenge.sequence.length; i++) {
                            if (state.collectedSequence[state.collectedSequence.length - state.sequenceChallenge.sequence.length + i] !== 
                                state.sequenceChallenge.sequence[i]) {
                                matches = false;
                                break;
                            }
                        }
                        
                        if (matches) {
                            // Sequence complete!
                            const reward = state.sequenceChallenge.reward;
                            state.score += reward.points;
                            showFeedback('SEQUENCE COMPLETE!');
                            
                            // Play reward chord
                            const progression = CHORD_PROGRESSIONS[scaleName] || CHORD_PROGRESSIONS['Major'];
                            const chord = progression[0];
                            chord.forEach((note, index) => {
                                setTimeout(() => {
                                    playNote(60 + 5 * 12 + note, 120, 400);
                                }, index * 40);
                            });
                            
                            state.sequenceChallenge = null;
                            state.collectedSequence = [];
                        }
                    }
                }
            }
            
            // Play token note with rhythm bonus
            const currentTime = performance.now();
            const beatInterval = (60 / state.tempo) * 1000;
            const timeSinceLastBeat = (currentTime - state.lastBeat) % beatInterval;
            const isOnBeat = timeSinceLastBeat < beatInterval * 0.2 || timeSinceLastBeat > beatInterval * 0.8;
            
            const velocity = isOnBeat ? 120 : 100;
            playNote(token.noteValue, velocity, 200);
            state.musicalNotes.push(new MusicalNote(
                token.x + token.width / 2,
                token.y + token.height / 2,
                token.noteValue,
                isOnBeat ? '#00ff00' : '#ffff00'
            ));
            
            // Play chord when combo reaches certain thresholds
            if (state.combo % 5 === 0) {
                const scaleName = scaleNames[state.scale];
                const progression = CHORD_PROGRESSIONS[scaleName] || CHORD_PROGRESSIONS['Major'];
                const chord = progression[state.currentChord % progression.length];
                const octave = 4;
                chord.forEach((note, index) => {
                    setTimeout(() => {
                        playNote(60 + octave * 12 + note, 70, 300);
                    }, index * 30);
                });
                state.currentChord++;
            }
            
            const points = Math.floor(200 * state.combo * state.rhythmBonus);
            state.score += points;
            
            showFeedback(isOnBeat ? 'RHYTHM TOKEN!' : 'TOKEN!');
            updateScoreDisplay();
        }
    }
}

/**
 * Show hit feedback
 */
function showFeedback(text) {
    const feedback = document.getElementById('hitFeedback');
    if (!feedback) return;
    
    feedback.textContent = text;
    feedback.className = 'hit-feedback perfect';
    
    setTimeout(() => feedback.classList.add('show'), 10);
    setTimeout(() => feedback.classList.remove('show'), 300);
    
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
 * Play a note (send to C++)
 */
function playNote(midiNote, velocity = 100, duration = 200) {
    if (window.juce) {
        window.juce.postMessage({
            type: 'playNote',
            note: midiNote,
            velocity: velocity,
            duration: duration
        });
    }
    
    state.activeNotes.add(midiNote);
    setTimeout(() => {
        state.activeNotes.delete(midiNote);
        if (window.juce) {
            window.juce.postMessage({
                type: 'stopNote',
                note: midiNote
            });
        }
    }, duration);
}

/**
 * Game loop
 */
let lastTime = performance.now();
state.lastBeat = performance.now();

function gameLoop() {
    const currentTime = performance.now();
    const deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    // Update beat timing
    const beatInterval = (60 / state.tempo) * 1000;
    state.beatTime = (state.beatTime + deltaTime) % beatInterval;
    
    // Clear canvas with sky gradient
    const gradient = ctx.createLinearGradient(0, 0, 0, canvas.height);
    gradient.addColorStop(0, '#87CEEB');
    gradient.addColorStop(0.5, '#98D8E8');
    gradient.addColorStop(1, '#B0E0E6');
    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    
    drawClouds();
    
    // Draw ground
    ctx.fillStyle = '#8B7355';
    ctx.fillRect(0, canvas.height - 50, canvas.width, 50);
    ctx.fillStyle = '#6B5B4D';
    for (let i = 0; i < canvas.width; i += 20) {
        ctx.fillRect(i, canvas.height - 50, 10, 50);
    }
    
    if (state.gameRunning && !state.isDead) {
        spawnObjects();
        updatePlayer();
        
        for (const platform of state.platforms) {
            platform.update();
        }
        
        for (const obstacle of state.obstacles) {
            obstacle.update();
        }
        
        for (const enemy of state.enemies) {
            enemy.update();
        }
        
        for (const powerUp of state.powerUps) {
            powerUp.update();
        }
        
        for (const token of state.tokens) {
            token.update();
        }
        
        // Update musical bridges
        for (const bridge of state.musicalBridges) {
            bridge.update();
        }
        
        // Update musical notes
        for (let i = state.musicalNotes.length - 1; i >= 0; i--) {
            state.musicalNotes[i].update();
            if (state.musicalNotes[i].life <= 0) {
                state.musicalNotes.splice(i, 1);
            }
        }
        
        // Update frequency bars for visual music representation
        updateFrequencyBars();
        
        // Clean up off-screen objects
        state.platforms = state.platforms.filter(p => p.x + p.width > state.cameraX - 100);
        state.obstacles = state.obstacles.filter(o => o.x + o.width > state.cameraX - 100);
        state.enemies = state.enemies.filter(e => e.x + e.width > state.cameraX - 100);
        state.tokens = state.tokens.filter(t => t.x + t.width > state.cameraX - 100 || t.collected);
        state.powerUps = state.powerUps.filter(p => p.x + p.width > state.cameraX - 100 || p.collected);
        state.musicalBridges = state.musicalBridges.filter(b => b.x + b.width > state.cameraX - 200);
        
        checkTokenCollection();
    }
    
    // Draw everything
    for (const obstacle of state.obstacles) {
        obstacle.draw();
    }
    
    for (const platform of state.platforms) {
        platform.draw();
    }
    
    for (const enemy of state.enemies) {
        enemy.draw();
    }
    
    for (const powerUp of state.powerUps) {
        powerUp.draw();
    }
    
    for (const token of state.tokens) {
        token.draw();
    }
    
    // Draw musical bridges
    for (const bridge of state.musicalBridges) {
        bridge.draw();
    }
    
    // Draw musical note particles
    for (const note of state.musicalNotes) {
        note.draw();
    }
    
    // Draw visual music representation (frequency bars)
    drawFrequencyBars();
    
    if (!state.isDead) {
        drawPlayer();
    }
    
    // Draw rhythm indicator
    drawRhythmIndicator();
    
    // Draw status messages
    if (!state.gameRunning) {
        ctx.fillStyle = '#ff0000';
        ctx.font = 'bold 28px monospace';
        ctx.textAlign = 'center';
        ctx.shadowBlur = 10;
        ctx.shadowColor = '#ff0000';
        ctx.fillText('PRESS PLAY TO START', canvas.width / 2, canvas.height / 2 - 40);
        ctx.font = 'bold 20px monospace';
        ctx.fillText('SPACEBAR TO JUMP', canvas.width / 2, canvas.height / 2 + 10);
        ctx.font = '16px monospace';
        ctx.fillStyle = '#ffff00';
        ctx.fillText('Collect NNAudio tokens!', canvas.width / 2, canvas.height / 2 + 50);
        ctx.font = '14px monospace';
        ctx.fillStyle = '#ff00ff';
        ctx.fillText('Everything makes music!', canvas.width / 2, canvas.height / 2 + 80);
        ctx.font = '12px monospace';
        ctx.fillStyle = '#00ffff';
        ctx.fillText('Time actions to the beat for bonuses!', canvas.width / 2, canvas.height / 2 + 110);
        ctx.shadowBlur = 0;
    } else if (state.isDead) {
        ctx.fillStyle = '#ff0000';
        ctx.font = 'bold 36px monospace';
        ctx.textAlign = 'center';
        ctx.shadowBlur = 15;
        ctx.shadowColor = '#ff0000';
        ctx.fillText('YOU DIED!', canvas.width / 2, canvas.height / 2 - 20);
        ctx.font = '20px monospace';
        ctx.fillText(`Lives: ${state.lives}`, canvas.width / 2, canvas.height / 2 + 20);
        if (state.lives > 0) {
            ctx.fillText('Respawning...', canvas.width / 2, canvas.height / 2 + 50);
        } else {
            ctx.fillText('GAME OVER', canvas.width / 2, canvas.height / 2 + 50);
            ctx.fillText('Press PLAY to restart', canvas.width / 2, canvas.height / 2 + 80);
        }
        ctx.shadowBlur = 0;
    }
    
    animationId = requestAnimationFrame(gameLoop);
}

/**
 * Draw clouds
 */
function drawClouds() {
    ctx.fillStyle = 'rgba(255, 255, 255, 0.6)';
    const cloudPositions = [
        { x: 100 - state.cameraX * 0.1, y: 80 },
        { x: 300 - state.cameraX * 0.1, y: 120 },
        { x: 500 - state.cameraX * 0.1, y: 100 }
    ];
    
    for (const cloud of cloudPositions) {
        ctx.beginPath();
        ctx.arc(cloud.x, cloud.y, 20, 0, Math.PI * 2);
        ctx.arc(cloud.x + 25, cloud.y, 25, 0, Math.PI * 2);
        ctx.arc(cloud.x + 50, cloud.y, 20, 0, Math.PI * 2);
        ctx.fill();
    }
}

/**
 * Update frequency bars for visual music representation
 */
function updateFrequencyBars() {
    const beatInterval = (60 / state.tempo) * 1000;
    const beatPhase = (performance.now() % beatInterval) / beatInterval;
    
    // Update active frequencies based on current notes
    state.activeFrequencies.clear();
    for (const note of state.activeNotes) {
        const frequency = Math.pow(2, (note - 69) / 12) * 440;
        state.activeFrequencies.set(note, {
            frequency: frequency,
            amplitude: 0.8,
            decay: 0.95
        });
    }
    
    // Decay amplitudes
    for (const [note, data] of state.activeFrequencies.entries()) {
        data.amplitude *= data.decay;
        if (data.amplitude < 0.1) {
            state.activeFrequencies.delete(note);
        }
    }
}

/**
 * Draw frequency bars (visual music representation)
 */
function drawFrequencyBars() {
    const barCount = 12;
    const barWidth = canvas.width / barCount;
    const maxHeight = 40;
    const yPos = 10;
    
    const scaleName = scaleNames[state.scale];
    const scale = SCALES[scaleName];
    
    for (let i = 0; i < barCount; i++) {
        const noteValue = 60 + scale[i % scale.length] + Math.floor(i / scale.length) * 12;
        const data = state.activeFrequencies.get(noteValue);
        const amplitude = data ? data.amplitude : 0;
        
        const height = amplitude * maxHeight;
        const x = i * barWidth;
        
        // Color based on scale position
        const hue = (i / barCount) * 360;
        ctx.fillStyle = `hsla(${hue}, 70%, 50%, ${0.3 + amplitude * 0.7})`;
        ctx.fillRect(x, yPos + maxHeight - height, barWidth - 2, height);
        
        // Beat pulse
        const beatInterval = (60 / state.tempo) * 1000;
        const beatPhase = (performance.now() % beatInterval) / beatInterval;
        if (beatPhase < 0.1) {
            ctx.fillStyle = `hsla(${hue}, 100%, 70%, 0.8)`;
            ctx.fillRect(x, yPos + maxHeight - height, barWidth - 2, height);
        }
    }
}

/**
 * Draw rhythm indicator
 */
function drawRhythmIndicator() {
    const beatInterval = (60 / state.tempo) * 1000;
    const beatPhase = (performance.now() % state.lastBeat) / beatInterval;
    
    // Draw beat circle
    const centerX = canvas.width - 30;
    const centerY = 30;
    const radius = 15;
    
    ctx.save();
    ctx.strokeStyle = '#ffffff';
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
    ctx.stroke();
    
    // Beat pulse
    if (beatPhase < 0.1) {
        ctx.fillStyle = '#00ff00';
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius * 0.8, 0, Math.PI * 2);
        ctx.fill();
    } else {
        ctx.fillStyle = `rgba(255, 255, 255, ${0.3 + beatPhase * 0.3})`;
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius * 0.6, 0, Math.PI * 2);
        ctx.fill();
    }
    
    // Rhythm streak indicator
    if (state.rhythmStreak > 0) {
        ctx.fillStyle = '#00ff00';
        ctx.font = 'bold 10px monospace';
        ctx.textAlign = 'center';
        ctx.fillText(`x${state.rhythmStreak}`, centerX, centerY + 25);
    }
    
    ctx.restore();
}

/**
 * Initialize controls
 */
document.addEventListener('DOMContentLoaded', () => {
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
        const diffs = ['Easy', 'Normal', 'Hard', 'Expert'];
        difficultyInfo.textContent = `${diffs[state.difficulty]} • ${state.tempo} BPM`;
    }
}

function initializeBypassToggle() {
    const bypassButton = document.getElementById('bypassToggle');
    if (!bypassButton) return;
    
    bypassButton.onclick = function() {
        if (state.isDead && state.lives === 0) {
            gameOver();
            state.gameRunning = true;
            state.lives = 3;
            state.score = 0;
            state.combo = 0;
            state.tokensCollected = 0;
            state.distance = 0;
            respawn();
        } else if (!state.gameRunning) {
            state.gameRunning = true;
            state.isDead = false;
            state.lives = 3;
            state.score = 0;
            state.combo = 0;
            state.tokensCollected = 0;
            state.distance = 0;
            respawn();
        } else {
            state.gameRunning = !state.gameRunning;
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
