# Zenith Animation Debug

## Added Visible Debug Info

The canvas now draws:

1. **Debug Text** (top-left corner):
   - Canvas dimensions
   - Center point coordinates
   - All 4 parameter values
   - Cloud count
   - Current time

2. **Test Circle**:
   - Big cyan circle (150px radius) at center
   - White stroke (8px thick)
   - 50% opacity fill

3. **Red Cross**:
   - 100px wide cross at center point
   - Bright red, 4px thick
   - Shows exact center position

If you don't see these elements, the canvas isn't rendering at all.

Possible issues:
- Canvas element not found
- WebView not loading scripts
- Z-index issues
- Canvas size 0x0

The debug info should tell us what's happening!
