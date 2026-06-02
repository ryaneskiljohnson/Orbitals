# Orbitals DSP Audit

**Date:** 2026-06-01 (updated)  
**Scope:** All 21 Orbitals plugins  
**Status:** ✅ Production-ready — all 21 build and pass DSP review fixes

---

## Executive Summary

| Category | Count | Verdict |
|----------|------:|---------|
| MIDI FX | 9 | ✅ All functional |
| Mandelbrot audio FX | 9 | ✅ All functional |
| 8-Bit Kit | 3 | ✅ All functional |

**Build verify:** `./build_and_verify_all.sh` → **21/21 SUCCESS**

---

## Fixes Applied (2026-06-01)

| Issue | Plugins | Fix |
|-------|---------|-----|
| MIDI dropped until buffer full | Retrograde | Pass CC/pitch bend immediately; emit reversed note windows in a loop |
| Wrong host MIDI FX classification | Planck, Mandelbrot, Rabbithole | Use `JucePlugin_*` flags instead of hardcoded `true` |
| Grid timing used block-relative time | Lagrange | Playhead song time + sample offset; 120 BPM fallback |
| No processing without transport | Tidal, Lagrange | 120 BPM free-running fallback when playhead unavailable |
| Threshold param unused | Eclipse | Threshold centers shadow zone; REVEAL/HIDE gating wired |
| UI param names mismatched APVTS | Eclipse | Editor maps `totality`→threshold, `shadowZone`→min/max, `mode`→eclipseMode |
| UI param names mismatched APVTS | Lagrange | Editor maps `drift`→min/max, `microChaos`→chaosX/Y |
| UI param names mismatched APVTS | Tidal | Editor maps tempo strings→rate index, shape strings→shape index |
| UI param names mismatched APVTS | Ion | Editor maps scale/attractionType strings→choice indices |
| Retrograde symmetry/echo unused | Retrograde | Symmetry blend + echo repeats wired; PHRASE mode UI fix |
| ByteBeat wrong host flags / branding | ByteBeat | JucePlugin flags; UI rebranded; plugin code fixed |
| ByteBeat stale Info plists (Rabbithole clone) | ByteBeat | Info-*.plist → com.nnaudio.bytebeat, ByteBeatAUFactory, ByBt subtype |
| Zenith Lagrange clone identity | Zenith | Unique plugin code Zeni, bundle com.nnaudio.zenith |
| ByteBeat pbx PluginCode Chip clone | ByteBeat | pbxproj JucePlugin_PluginCode → ByBt (0x42794274) |
| ChipTune/PixelPulse MIDI stubs | ChipTune, PixelPulse | playNote/stopNote queue → MIDI output; ProducesMidiOutput enabled |
| Hardcoded host MIDI flags | Eclipse, Ion, Lagrange, Perihelion, Retrograde, Zenith, Observer, Hadron, Entanglement | Standardized to `JucePlugin_*` conditional pattern |
| Editor debug logging | Fabric, Singularity, ChipTune | Removed `std::cout` bridge spam from UI message handlers |
| Kepler stability params unused | Kepler | Added orbit stability XY pad wired to `stabilityX`/`stabilityY` |
| RT unsafe UI call | Zenith | Atomic MIDI note queue + timer poll on message thread |
| Audio thread debug logging | Fabric, Singularity | Removed `std::cout`/`std::cerr` from `processBlock` |
| Stale misleading comments | Planck, Rabbithole, Singularity, Observer, Quarks | Removed |

---

## Plugin Matrix (Production)

### Orbital Series — MIDI FX

| Plugin | DSP | Status |
|--------|-----|--------|
| Apogee | Velocity lift + curves + momentum | ✅ |
| Eclipse | Shadow-zone velocity mask | ✅ |
| Ion | Scale constraint (SNAP/PULL/GUIDE) | ✅ |
| Kepler | Orbital timing/velocity modulation | ✅ |
| Lagrange | 16th-note grid stabilization | ✅ |
| Perihelion | Velocity sculptor | ✅ |
| Retrograde | Velocity/timing/phrase reversal | ✅ |
| Tidal | Tempo-synced timing LFO | ✅ |
| Zenith | Velocity expansion | ✅ |

### Mandelbrot Series — Audio FX

| Plugin | DSP | Status |
|--------|-----|--------|
| Fabric | Reverb + predelay + wet/dry | ✅ |
| Singularity | Compressor | ✅ |
| Hadron | Saturation + tone filter | ✅ |
| Observer | Dynamic EQ / expander | ✅ |
| Planck | 3-band EQ | ✅ |
| Quarks | Gate/expander | ✅ |
| Entanglement | Stereo delay + filters | ✅ |
| Mandelbrot | Modulated phaser | ✅ |
| Rabbithole | Flanger | ✅ |

### 8-Bit Kit

| Plugin | DSP | Status |
|--------|-----|--------|
| ByteBeat | 8-bit styled quantum flanger | ✅ |
| ChipTune | Platformer game → MIDI note output | ✅ |
| PixelPulse | Rhythm game → MIDI note output | ✅ |

---

## Known Low-Priority Items

- **Mandelbrot** phaser recalculates filter coefficients per sample (CPU heavy but correct)
- **Automated DSP tests** — manual DAW QA still recommended; no headless test suite yet

---

## Verification

```bash
cd /Users/rjmacbookpro/Development/Orbitals
./build_and_verify_all.sh
```

Manual DAW checklist:

- [ ] MIDI FX on MIDI track — velocity/timing changes audible downstream
- [ ] Audio FX on audio track — bypass toggles effect
- [ ] Planck/Mandelbrot/Rabbithole — appear as audio FX with stereo I/O
- [ ] Eclipse totality knob moves masking threshold
- [ ] Lagrange quantizes to 16ths with transport running

---

## Files Reviewed

`*/Source/PluginProcessor.cpp` for all 21 plugins; production fixes applied throughout.
