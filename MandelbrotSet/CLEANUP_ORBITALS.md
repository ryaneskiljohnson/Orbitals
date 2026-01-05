# Cleanup Orbitals References - Manual Instructions

Since the terminal is having issues, here's what needs to be done:

## Quick Fix Script

Run this Python script I created:
```bash
cd /Users/rjmacbookpro/Development/Orbitals/MandelbrotSet
python3 remove_orbitals_references.py
```

## Manual Replacements Needed

For each plugin, replace:

### File References (in all .cpp and .h files):
- `orbitals-design-system.css` → `mandelbrot-design-system.css`
- `orbitals-components.js` → `mandelbrot-components.js`
- `orbitals-animations.js` → `mandelbrot-animations.js`
- `orbitals-particles.js` → `mandelbrot-particles.js`

### Class Names (per plugin):

**Singularity:**
- `TidalAudioProcessor` → `SingularityAudioProcessor`
- `TidalAudioProcessorEditor` → `SingularityAudioProcessorEditor`
- `Tidal` → `Singularity` (in comments/paths)

**Fabric:**
- `ApogeeAudioProcessor` → `FabricAudioProcessor`
- `ApogeeAudioProcessorEditor` → `FabricAudioProcessorEditor`
- `Apogee` → `Fabric`

**Entanglement:**
- `LagrangeAudioProcessor` → `EntanglementAudioProcessor`
- `LagrangeAudioProcessorEditor` → `EntanglementAudioProcessorEditor`
- `Lagrange` → `Entanglement`

**Planck:**
- `EclipseAudioProcessor` → `PlanckAudioProcessor`
- `EclipseAudioProcessorEditor` → `PlanckAudioProcessorEditor`
- `Eclipse` → `Planck`

**Observer:**
- `IonAudioProcessor` → `ObserverAudioProcessor`
- `IonAudioProcessorEditor` → `ObserverAudioProcessorEditor`
- `Ion` → `Observer`

**Hadron:**
- `PerihelionAudioProcessor` → `HadronAudioProcessor`
- `PerihelionAudioProcessorEditor` → `HadronAudioProcessorEditor`
- `Perihelion` → `Hadron`

**Mandelbrot:**
- `RetrogradeAudioProcessor` → `MandelbrotAudioProcessor`
- `RetrogradeAudioProcessorEditor` → `MandelbrotAudioProcessorEditor`
- `Retrograde` → `Mandelbrot`

**Quarks:**
- `KeplerAudioProcessor` → `QuarksAudioProcessor`
- `KeplerAudioProcessorEditor` → `QuarksAudioProcessorEditor`
- `Kepler` → `Quarks`

**Rabbithole:**
- `ZenithAudioProcessor` → `RabbitholeAudioProcessor`
- `ZenithAudioProcessorEditor` → `RabbitholeAudioProcessorEditor`
- `Zenith` → `Rabbithole`

### Background Images:
- `tidal-background.png` → `singularity.png`
- `apogee-background.png` → `fabric.png`
- `lagrange-background.png` → `entanglement.png`
- `eclipse-background.png` → `planck.png`
- `ion-background.png` → `observer.png`
- `perihelion-background.png` → `hadron.png`
- `retrograde-background.png` → `mandelbrot.png`
- `kepler-background.png` → `quarks.png`
- `zenith-background.png` → `rabbithole.png`

### Path Updates:
- `getChildFile ("Tidal")` → `getChildFile ("Singularity")`
- `getChildFile ("Apogee")` → `getChildFile ("Fabric")`
- etc. for all plugins

The Python script `remove_orbitals_references.py` should handle all of this automatically.
