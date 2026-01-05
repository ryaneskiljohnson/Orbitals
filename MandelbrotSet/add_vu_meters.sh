#!/bin/bash

# VU Meter HTML template
VU_METER_HTML='<div class="info-display">
                <div class="meter-container">
                    <label>INPUT</label>
                    <div class="vu-meter">
                        <div class="vu-meter-bar">
                            <div class="vu-meter-fill" id="inputMeterFill"></div>
                            <div class="vu-meter-peak" id="inputMeterPeak"></div>
                        </div>
                        <span class="meter-label" id="inputLevel">-∞ dB</span>
                    </div>
                </div>
                <div class="meter-container">
                    <label>OUTPUT</label>
                    <div class="vu-meter">
                        <div class="vu-meter-bar">
                            <div class="vu-meter-fill" id="outputMeterFill"></div>
                            <div class="vu-meter-peak" id="outputMeterPeak"></div>
                        </div>
                        <span class="meter-label" id="outputLevel">-∞ dB</span>
                    </div>
                </div>
            </div>'

# VU Meter CSS
VU_METER_CSS='
.info-display { display: flex; align-items: center; justify-content: flex-end; gap: var(--spacing-xl); font-family: var(--font-mono); font-size: var(--font-size-xs); }
.meter-container { display: flex; flex-direction: column; align-items: center; gap: var(--spacing-xs); }
.meter-container label { font-family: var(--font-header); font-size: var(--font-size-xs); letter-spacing: 1px; color: var(--planck-white); opacity: 0.7; }
.vu-meter { display: flex; flex-direction: column; align-items: center; gap: var(--spacing-xs); }
.vu-meter-bar { width: 8px; height: 120px; background: var(--glass-bg-dark); border: 1px solid var(--glass-border); border-radius: var(--radius-sm); position: relative; overflow: hidden; box-shadow: inset 0 0 10px rgba(0, 0, 0, 0.5); }
.vu-meter-fill { position: absolute; bottom: 0; left: 0; width: 100%; height: 0%; background: linear-gradient(to top, #00ff00 0%, #88ff00 20%, #ffff00 50%, #ff8800 80%, #ff0000 100%); transition: height 50ms linear; box-shadow: 0 0 8px currentColor; border-radius: 0 0 var(--radius-sm) var(--radius-sm); }
.vu-meter-peak { position: absolute; left: 0; width: 100%; height: 2px; background: white; box-shadow: 0 0 6px white; transition: top 100ms linear; opacity: 0; }
.vu-meter-peak.visible { opacity: 1; }
.meter-label { font-family: var(--font-mono); font-size: var(--font-size-xs); color: var(--planck-white); opacity: 0.8; min-width: 60px; text-align: center; }'

# VU Meter JavaScript function
VU_METER_JS='
// Update VU meter bar and peak
function updateVUMeter(type, levelDb) {
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
    meterLabel.textContent = levelDb > -100 ? `${levelDb.toFixed(1)} dB` : '\''-∞ dB'\'';
    
    // Update peak hold
    if (levelDb > -60) {
        meterPeak.style.top = `${100 - heightPercent}%`;
        meterPeak.classList.add('\''visible'\'');
        
        // Reset peak after 1 second
        clearTimeout(meterPeak._peakTimeout);
        meterPeak._peakTimeout = setTimeout(() => {
            meterPeak.classList.remove('\''visible'\'');
        }, 1000);
    } else {
        meterPeak.classList.remove('\''visible'\'');
    }
}'

echo "VU Meter templates created"
