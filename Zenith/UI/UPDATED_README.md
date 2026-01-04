# Zenith UI - Updated Layout

## Changes Made
1. ✅ **Stopped background animation** - Added aggressive CSS overrides to disable starfield drift
2. ✅ **Standard Orbitals layout** - Using the same grid layout as Perihelion and other plugins
3. ✅ **Zenith theme colors** - Sky blue (#f0f9ff), bright cyan (#38bdf8), deep sky blue (#0ea5e9)

## Layout Structure
```
┌──────────────────────────────────────┐
│  Logo      ZENITH           Help     │
├──────────────────────────────────────┤
│                                      │
│  [EXP]    [THRESHOLD]      [CURVE]  │
│    │          KNOB            │      │
│    │                          │      │
│    │        CEILING           │      │
│    │      ───────────          │      │
│                                      │
├──────────────────────────────────────┤
│  [BYPASS]              Notes: 0     │
└──────────────────────────────────────┘
```

## Components Used
- **OrbitalsSlider** - For Expansion, Curve, and Ceiling
- **OrbitalsKnob** - For Threshold (center)
- **Canvas visualization** - Radial display showing velocity expansion in real-time

## Controls
- **Left**: Expansion slider (0-100%)
- **Center**: Threshold knob (0-127)
- **Right**: Curve slider (0-100%)
- **Bottom**: Ceiling slider (0-127)

Ready to test!
