# Authentication Implementation - Orbitals Plugins

## Overview
All Orbitals plugins now include authentication using the `HubAuthComponent` system, which checks for a valid license file and displays an overlay if authentication is required.

## Architecture

### Core Component
- **Location**: `_Shared/Authentication/HubAuthComponent.h/cpp`
- **Namespace**: `NNAudio::Authentication`
- **Purpose**: Displays authentication overlay and validates license file

### License File
- **Location**: 
  - macOS: `~/Library/Application Support/NNAudio Access/license.dat`
  - Windows: `%APPDATA%/NNAudio Access/license.dat`
- **Format**: Base64 encoded, BlowFish encrypted
- **Content**: ISO8601 expiration date (first token), followed by comma-separated product IDs

### Encryption
- **Algorithm**: BlowFish
- **Key**: Hardcoded in `HubAuthComponent::loadAndDecryptFileContents()`
- **Encoding**: Base64

## Integration

### Plugin Editor Changes
Each plugin's `PluginEditor` includes:

1. **Header** (`PluginEditor.h`):
   ```cpp
   #include "../../_Shared/Authentication/HubAuthComponent.h"
   
   private:
       NNAudio::Authentication::HubAuthComponent m_auth_component;
   ```

2. **Implementation** (`PluginEditor.cpp`):
   ```cpp
   // Constructor
   PluginEditor::PluginEditor(Processor& p)
       : AudioProcessorEditor(&p), audioProcessor(p), m_auth_component("product_id")
   {
       // ...
       addChildComponent(m_auth_component);
   }
   
   // Resized
   void PluginEditor::resized()
   {
       m_auth_component.setBounds(getLocalBounds());
       // ... other resizing
   }
   ```

### Resources in .jucer Files
Each plugin's `.jucer` file includes these resources:
- `hub_auth_background.png` - Background image for auth overlay
- `hub_auth_logo.png` - Logo image for auth overlay
- `product_id.txt` - Product ID for this plugin

## Product IDs
- `tidal` - Tidal plugin
- `apogee` - Apogee plugin
- `kepler` - Kepler plugin
- `lagrange` - Lagrange plugin
- `eclipse` - Eclipse plugin
- `ion` - Ion plugin
- `perihelion` - Perihelion plugin
- `retrograde` - Retrograde plugin

## Behavior

### When Authorized
- Auth overlay is hidden
- Checks license every 15 minutes
- Plugin functions normally

### When Not Authorized
- Auth overlay is displayed
- Blocks user interaction
- Shows message: "Your plugin's authentication needs to be refreshed. Please launch the NNAudio Access app to continue."
- Checks license every 5 seconds

### Initial Check
- First check occurs after 1ms (immediate on startup)

## Security Considerations

1. **Hardcoded Encryption Key**: The BlowFish key is hardcoded in the source code. This provides basic obfuscation but is not cryptographically secure.

2. **License File Location**: The license file is stored in user-accessible directory, allowing manual inspection (though encrypted).

3. **Product ID Validation**: Each plugin validates its specific product ID against the license file.

4. **Expiration Checking**: License expiration is checked against current system time.

## Testing Checklist

- [ ] Auth overlay appears when license file is missing
- [ ] Auth overlay appears when license file is invalid
- [ ] Auth overlay appears when product ID is not in license
- [ ] Auth overlay appears when license is expired
- [ ] Auth overlay hides when license is valid
- [ ] Auth overlay resizes correctly with editor
- [ ] Auth overlay blocks interaction when visible
- [ ] Periodic checks work (15 min when authorized, 5 sec when not)
- [ ] All 8 plugins have correct product IDs
- [ ] All 8 plugins have auth resources in .jucer files

## Next Steps

1. **Regenerate Xcode Projects**: Open each `.jucer` file in Projucer and click "Save and Open in IDE" to regenerate projects with new resources.

2. **Test Authentication Flow**: 
   - Test with missing license file
   - Test with invalid license file
   - Test with valid license file
   - Test expiration handling

3. **NNAudio Access App**: Ensure the NNAudio Access app can create/update the license.dat file in the correct location.

4. **Production Considerations**:
   - Consider using a more secure key management system
   - Add logging for authentication failures (optional)
   - Consider server-side validation for enhanced security
