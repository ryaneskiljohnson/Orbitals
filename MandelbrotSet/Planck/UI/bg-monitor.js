/**
 * @fileoverview Background Image Monitoring - Visible Debug Panel
 * @module Background Monitor
 * 
 * Patches the monitoring function to add a visible debug panel
 */

// Wait a bit for app.js to load
setTimeout(() => {
  if (typeof setupBackgroundImageMonitoring === 'function') {
    const original = setupBackgroundImageMonitoring;
    
    setupBackgroundImageMonitoring = function() {
      const container = document.querySelector('.plugin-container');
      if (!container) return;
      
      // Create visible debug panel
      let debugPanel = document.getElementById('bg-debug-panel');
      if (!debugPanel) {
        debugPanel = document.createElement('div');
        debugPanel.id = 'bg-debug-panel';
        debugPanel.style.cssText = 'position: fixed; top: 10px; right: 10px; width: 400px; max-height: 500px; overflow-y: auto; background: rgba(0, 0, 0, 0.9); border: 2px solid #fbbf24; border-radius: 8px; padding: 10px; font-family: monospace; font-size: 11px; color: #fbbf24; z-index: 10000; pointer-events: auto;';
        document.body.appendChild(debugPanel);
      }
      
      function logToPanel(message, type = 'info') {
        const entry = document.createElement('div');
        const color = type === 'error' ? '#ff0000' : type === 'warn' ? '#ffaa00' : '#00ff00';
        entry.style.cssText = `margin: 4px 0; padding: 4px; border-left: 3px solid ${color}; padding-left: 8px;`;
        entry.textContent = `[${new Date().toLocaleTimeString()}] ${message}`;
        debugPanel.appendChild(entry);
        debugPanel.scrollTop = debugPanel.scrollHeight;
        if (type === 'error') console.error(message);
        else if (type === 'warn') console.warn(message);
        else console.log(message);
      }
      
      // Intercept console methods to also log to panel
      const originalLog = console.log;
      const originalWarn = console.warn;
      const originalError = console.error;
      
      console.log = function(...args) {
        originalLog.apply(console, args);
        const msg = args[0];
        if (typeof msg === 'string') {
          if (msg.includes('::before') || msg.includes('::after') || msg.includes('Initial')) {
            const details = args[1] ? JSON.stringify(args[1]) : '';
            logToPanel(msg.replace('🔴 ::before BACKGROUND CHANGED:', '🔴 ::before CHANGED').replace('🔴 ::after BACKGROUND CHANGED:', '🔴 ::after CHANGED').replace('📊 Initial ::before background:', '📊 Initial ::before').replace('📊 Initial ::after background:', '📊 Initial ::after') + (details ? ' ' + details : ''), 'info');
          }
        }
      };
      
      console.warn = function(...args) {
        originalWarn.apply(console, args);
        const msg = args[0];
        if (typeof msg === 'string' && msg.includes('MISMATCH')) {
          const details = args[1] ? JSON.stringify(args[1]) : '';
          logToPanel(msg.replace('⚠️ INITIAL MISMATCH between ::before and ::after:', '⚠️ INITIAL MISMATCH!').replace('⚠️ CURRENT MISMATCH between ::before and ::after:', '⚠️ MISMATCH') + (details ? ' ' + details : ''), 'warn');
        }
      };
      
      console.error = function(...args) {
        originalError.apply(console, args);
        const msg = args[0];
        if (typeof msg === 'string' && msg.includes('CHANGED')) {
          const details = args[1] ? JSON.stringify(args[1]) : '';
          logToPanel(msg + (details ? ' ' + details : ''), 'error');
        }
      };
      
      // Call original function
      original();
      
      // Restore console after monitoring period
      setTimeout(() => {
        console.log = originalLog;
        console.warn = originalWarn;
        console.error = originalError;
      }, 10000);
    };
  }
}, 100);
