# 🌀 MANDELBROT SET - COMPLETE CHECKLIST

## ✅ COMPLETED

### .jucer Files (5/9 done)
- [x] Singularity.jucer
- [x] Fabric.jucer
- [x] Entanglement.jucer
- [x] Planck.jucer
- [x] Observer.jucer
- [ ] Hadron.jucer (Perihelion.jucer → Hadron.jucer)
- [ ] Mandelbrot.jucer (Retrograde.jucer → Mandelbrot.jucer)
- [ ] Quarks.jucer (Kepler.jucer → Quarks.jucer)
- [ ] Rabbithole.jucer (Zenith.jucer → Rabbithole.jucer)

### UI Files
- [x] All 9 plugins have HTML/CSS/JS files
- [x] All use mandelbrot-design-system.css
- [x] All have unique themed parameters
- [x] All have unique animations

### C++ Files - NEEDS UPDATE
- [ ] All PluginProcessor.cpp files
- [ ] All PluginProcessor.h files
- [ ] All PluginEditor.cpp files
- [ ] All PluginEditor.h files

**Replacements needed in C++:**
- `TidalAudioProcessor` → `SingularityAudioProcessor`
- `ApogeeAudioProcessor` → `FabricAudioProcessor`
- `LagrangeAudioProcessor` → `EntanglementAudioProcessor`
- `EclipseAudioProcessor` → `PlanckAudioProcessor`
- `IonAudioProcessor` → `ObserverAudioProcessor`
- `PerihelionAudioProcessor` → `HadronAudioProcessor`
- `RetrogradeAudioProcessor` → `MandelbrotAudioProcessor`
- `KeplerAudioProcessor` → `QuarksAudioProcessor`
- `ZenithAudioProcessor` → `RabbitholeAudioProcessor`

**File references:**
- `orbitals-design-system.css` → `mandelbrot-design-system.css`
- `orbitals-components.js` → `mandelbrot-components.js`
- `orbitals-animations.js` → `mandelbrot-animations.js`
- `orbitals-particles.js` → `mandelbrot-particles.js`

**Background images:**
- `tidal-background.png` → `singularity.png`
- `apogee-background.png` → `fabric.png`
- `lagrange-background.png` → `entanglement.png`
- `eclipse-background.png` → `planck.png`
- `ion-background.png` → `observer.png`
- `perihelion-background.png` → `hadron.png`
- `retrograde-background.png` → `mandelbrot.png`
- `kepler-background.png` → `quarks.png`
- `zenith-background.png` → `rabbithole.png`

**Paths:**
- `getChildFile ("Tidal")` → `getChildFile ("MandelbrotSet").getChildFile ("Singularity")`
- etc. for all plugins

**Comments:**
- `MIDI FX` → `Audio FX`
- Plugin name references in comments

## 🚀 NEXT STEPS

1. Finish remaining 4 .jucer files
2. Update all C++ files (36 files total: 9 plugins × 4 files each)
3. Verify all UI files are complete
4. Test build one plugin
