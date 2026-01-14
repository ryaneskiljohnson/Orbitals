# Orbitals Plugin Help Text

Contextual help descriptions for all plugin parameters.

## Perihelion - Velocity Sculptor

- **GRAVITY**: Controls how tightly notes are pulled toward the solar point. Higher values create tighter orbits and more intense velocity shaping.
- **ORBIT**: Controls orbital eccentricity. Low values create circular orbits, high values create highly elliptical paths with varying velocity.
- **SOLAR POINT**: Target velocity value (1-127) that notes are pulled toward. Acts as the gravitational center for velocity shaping.
- **INNER / OUTER BIAS**: Rotates the orbital plane. Negative values favor notes below the solar point (inner orbit), positive values favor notes above (outer orbit).

## Lagrange - Timing Stabilizer

- **STABILITY**: Controls how strongly timing is stabilized. Higher values reduce timing drift without quantization.
- **MASS**: Affects the gravitational balance between grid and human feel. Higher mass increases pull toward the grid.
- **DRIFT RANGE**: Sets the allowed timing deviation range in milliseconds. Notes stay within this window.
- **MICRO CHAOS**: X/Y pad for subtle randomization. X controls timing variance, Y controls velocity variance.

## Apogee - Velocity & Energy Booster

- **LIFT**: Controls the amount of velocity boost applied. Higher values raise velocities more dramatically.
- **CURVE**: Selects the boost curve shape (LINEAR, EXP, S-CURVE, LOG). Changes how velocity increases from input to output.
- **CEILING**: Maximum output velocity (1-127). Prevents velocities from exceeding this limit.
- **MOMENTUM**: Controls how quickly the boost is applied. Higher values create more immediate velocity changes.

## Retrograde - MIDI Motion Reversal

- **MODE**: Selects what to reverse - VELOCITY (velocity contours), TIMING (note timing), or PHRASE (note order).
- **SCOPE**: Number of notes or bars to reverse (1-4). Larger scope reverses longer phrases.
- **SYMMETRY**: Rotation angle of the mirror axis. Changes the reflection plane for reversal.
- **ECHO**: Creates mirrored copies of reversed notes (0-8 echoes). Adds rhythmic complexity.

## Tidal - Rhythmic Push & Pull

- **TIDE STRENGTH**: Amplitude of the timing modulation. Higher values create more pronounced push/pull.
- **CYCLE LENGTH**: Tempo-synced rate of the tidal cycle (1/4, 1/8, 1/16, 1/32).
- **PHASE**: Shifts the wave pattern forward or backward (0-360°). Changes where in the cycle notes are affected.
- **FLOW**: Toggle between SINE, TRI, SAW, and SQR wave shapes for different groove feels.

## Ion - Harmonic Constraint Engine

- **CENTER KEY**: Root note of the harmonic field (C, C#, D, etc.). All notes are pulled toward this key.
- **FIELD WIDTH**: Size of the harmonic attraction field. Wider fields allow more note deviation.
- **TENSION**: Strength of the magnetic pull toward scale notes. Higher tension snaps notes more aggressively.
- **SCALE**: Selects the scale/mode (Major, Minor, Dorian, etc.). Defines which notes are "in key".
- **ATTRACTION**: Mode selector - SNAP (immediate), PULL (gradual), or GUIDE (subtle suggestion).

## Eclipse - Velocity Masking

- **THRESHOLD**: Velocity level where the eclipse occurs (0-127). Notes below are hidden, above are revealed.
- **FADE ZONE**: Range around threshold where notes gradually fade in/out. Creates smooth transitions.
- **INVERT**: Toggle between REVEAL (show above threshold) and HIDE (hide above threshold).
- **GHOST GAIN**: Volume of suppressed notes in dB (-60 to 0). Allows "ghost notes" to remain audible.

## Kepler - MIDI Orbit Generator

- **ORBIT COUNT**: Number of simultaneous orbital paths (1-8). More orbits create complex rhythmic patterns.
- **ECCENTRICITY**: Shape of the orbits (0-0.95). Higher values create more elliptical, less circular orbits.
- **ROTATION SPEED**: How fast notes move through their orbits. Affects rhythmic timing transformation.
- **STABILITY**: X/Y pad for orbit stability. X controls timing jitter, Y controls velocity variation.
- **ORBITAL PERIOD**: Tempo-synced orbit duration (1/8, 1/4, 1/2, 1 bar, 2 bars).
- **ORBIT DIRECTION**: Clockwise, counter-clockwise, or alternating orbital motion.
