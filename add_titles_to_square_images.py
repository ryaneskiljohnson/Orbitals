#!/usr/bin/env python3
"""
@fileoverview Add plugin titles and NNAudio logo to square background images
@module Title and Logo Overlay Generator

Superimposes the plugin title text (with proper font, color, and styling) 
at the top center and the NNAudio logo in the top left of each square image.
"""

import os
from PIL import Image, ImageDraw, ImageFont, ImageFilter

# Define directories
SQUARE_DIR = "_Shared/Assets/backgrounds/square"
LOGO_PATH = "_Shared/Assets/logos/nnaudio-logo.png"
SCREENSHOT_DIR = "_Shared/Assets/backgrounds/screenshot"
OUTPUT_DIR = "_Shared/Assets/product-images"
FONTS_DIR = "_Shared/Assets/fonts"  # Project fonts (e.g. Orbitron for CymaSynth)

# Plugin configuration: title text and color
PLUGIN_CONFIG = {
    "apogee": {
        "title": "APOGEE",
        "color": "#00d4ff",  # Orbit Cyan
        "text_shadow": (0, 0, 20, 0.8)  # (blur_x, blur_y, spread, opacity)
    },
    "eclipse": {
        "title": "ECLIPSE",
        "color": "#fef3c7",  # Corona Bright (from enhanced contrast)
        "text_shadow": (0, 0, 20, 0.9)
    },
    "ion": {
        "title": "ION",
        "color": "#c084fc",  # Ion Core (from enhanced contrast)
        "text_shadow": (0, 0, 20, 0.8)
    },
    "kepler": {
        "title": "KEPLER",
        "color": "#3b82f6",  # Celestial Blue
        "text_shadow": (0, 0, 20, 0.8)
    },
    "lagrange": {
        "title": "LAGRANGE",
        "color": "#0094ff",  # Stellar Blue
        "text_shadow": (0, 0, 20, 0.8)
    },
    "perihelion": {
        "title": "PERIHELION",
        "color": "#ffa500",  # Solar Gold
        "text_shadow": (0, 0, 20, 0.8)
    },
    "retrograde": {
        "title": "RETROGRADE",
        "color": "#6b2fb5",  # Nebula Purple
        "text_shadow": (0, 0, 20, 0.8)
    },
    "tidal": {
        "title": "TIDAL",
        "color": "#1e40af",  # Deep Ocean Blue
        "text_shadow": (0, 0, 20, 0.8)
    },
    "zenith": {
        "title": "ZENITH",
        "color": "#38bdf8",  # Bright Cyan
        "text_shadow": (0, 0, 20, 0.8)
    },
    "cymasynth": {
        "title": "CYMASYNTH",
        "color": "#55b5a6",  # Teal from screenshot (fallback)
        "text_shadow": (0, 0, 20, 0.8),
        "screenshot": "CymaSynth1.png",
        "title_parts": [  # CYMA teal, SYNTH white - same font as CymaSynth project (Orbitron)
            {"text": "CYMA", "color": "#55b5a6"},
            {"text": "SYNTH", "color": "#ffffff"}
        ],
        "background_zoom": 1.4,  # Zoom into background (crop center, scale up)
        "font_path": "Orbitron-Bold.woff",  # Same font as CymaSynth (Orbitron from orbitals-design-system.css)
        "screenshot_offset_y": 30  # Push screenshot down (extra pixels below title)
    }
}

def hex_to_rgb(hex_color):
    """
    @brief Convert hex color to RGB tuple
    @param hex_color Hex color string (e.g., "#00d4ff")
    @returns RGB tuple
    """
    hex_color = hex_color.lstrip('#')
    return tuple(int(hex_color[i:i+2], 16) for i in (0, 2, 4))

def get_font(size, bold=True, font_preference=None, font_path=None):
    """
    @brief Get font with fallbacks, prioritizing bold fonts
    @param size Font size in pixels
    @param bold Whether to prefer bold font variants
    @param font_preference Optional: "space_mono" or "montserrat" to prefer specific font
    @param font_path Optional: path to .ttf file (relative to project root or FONTS_DIR)
    @returns Font object or None if system fonts not available
    @note Tries font_path first if provided, then Orbitron, Space Mono, etc.
    """
    # When font_path is provided (e.g. Orbitron from CymaSynth project), use it first
    if font_path:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        # Resolve to absolute path - script lives in project root
        abs_font_path = os.path.abspath(os.path.join(script_dir, FONTS_DIR, os.path.basename(font_path)))
        if os.path.exists(abs_font_path):
            try:
                return ImageFont.truetype(abs_font_path, size)
            except Exception:
                pass
        # Fallback: try CWD-relative
        for p in [os.path.join(FONTS_DIR, font_path), font_path]:
            if os.path.exists(p):
                try:
                    return ImageFont.truetype(os.path.abspath(p), size)
                except Exception:
                    pass
    # When font_preference is "montserrat", try Montserrat first (clean geometric sans like Montserrat)
    if font_preference == "montserrat":
        montserrat_paths = [
            "/System/Library/Fonts/Supplemental/Montserrat-Bold.ttf",
            "/Library/Fonts/Montserrat-Bold.ttf",
            "~/Library/Fonts/Montserrat-Bold.ttf",
            "~/.fonts/Montserrat-Bold.ttf",
            "/System/Library/Fonts/Supplemental/Montserrat-SemiBold.ttf",
            "/Library/Fonts/Montserrat-SemiBold.ttf",
            "/System/Library/Fonts/Supplemental/Montserrat-Regular.ttf",
            "/Library/Fonts/Montserrat-Regular.ttf",
            "~/Library/Fonts/Montserrat-Regular.ttf",
            "~/.fonts/Montserrat-Regular.ttf",
        ] if bold else [
            "/System/Library/Fonts/Supplemental/Montserrat-Regular.ttf",
            "/Library/Fonts/Montserrat-Regular.ttf",
            "~/Library/Fonts/Montserrat-Regular.ttf",
            "~/.fonts/Montserrat-Regular.ttf",
            "/System/Library/Fonts/Supplemental/Montserrat-SemiBold.ttf",
            "/Library/Fonts/Montserrat-SemiBold.ttf",
            "/System/Library/Fonts/Supplemental/Montserrat-Bold.ttf",
            "/Library/Fonts/Montserrat-Bold.ttf",
        ]
        for path in montserrat_paths:
            expanded = os.path.expanduser(path)
            if os.path.exists(expanded):
                try:
                    return ImageFont.truetype(expanded, size)
                except:
                    continue
        # Fallback: Helvetica Neue (similar clean geometric sans on macOS)
        helvetica_paths = [
            "/System/Library/Fonts/Supplemental/Helvetica Neue Bold.ttf",
            "/System/Library/Fonts/Helvetica.ttc",
            "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
            "/System/Library/Fonts/Supplemental/Arial.ttf",
        ] if bold else [
            "/System/Library/Fonts/Supplemental/Helvetica Neue.ttf",
            "/System/Library/Fonts/Helvetica.ttc",
            "/System/Library/Fonts/Supplemental/Arial.ttf",
        ]
        for path in helvetica_paths:
            if os.path.exists(path):
                try:
                    return ImageFont.truetype(path, size)
                except:
                    continue
    # When font_preference is "space_mono", try Space Mono first
    if font_preference == "space_mono":
        space_mono_paths = [
            "/System/Library/Fonts/Supplemental/SpaceMono-Bold.ttf",
            "/Library/Fonts/SpaceMono-Bold.ttf",
            "/System/Library/Fonts/Supplemental/SpaceMono-Regular.ttf",
            "/Library/Fonts/SpaceMono-Regular.ttf",
        ] if bold else [
            "/System/Library/Fonts/Supplemental/SpaceMono-Regular.ttf",
            "/Library/Fonts/SpaceMono-Regular.ttf",
            "/System/Library/Fonts/Supplemental/SpaceMono-Bold.ttf",
            "/Library/Fonts/SpaceMono-Bold.ttf",
        ]
        for path in space_mono_paths:
            expanded = os.path.expanduser(path)
            if os.path.exists(expanded):
                try:
                    return ImageFont.truetype(expanded, size)
                except:
                    continue
    if bold:
        # Try to load Orbitron Bold first
        font_paths = [
            "/System/Library/Fonts/Supplemental/Orbitron-Bold.ttf",
            "/Library/Fonts/Orbitron-Bold.ttf",
            "~/.fonts/Orbitron-Bold.ttf",
            "/System/Library/Fonts/Supplemental/Orbitron-Black.ttf",
            "/Library/Fonts/Orbitron-Black.ttf",
            "~/.fonts/Orbitron-Black.ttf",
            "/System/Library/Fonts/Supplemental/Orbitron-Regular.ttf",
            "/Library/Fonts/Orbitron-Regular.ttf",
            "~/.fonts/Orbitron-Regular.ttf",
        ]
    else:
        # Try regular first
        font_paths = [
            "/System/Library/Fonts/Supplemental/Orbitron-Regular.ttf",
            "/Library/Fonts/Orbitron-Regular.ttf",
            "~/.fonts/Orbitron-Regular.ttf",
            "/System/Library/Fonts/Supplemental/Orbitron-Bold.ttf",
            "/Library/Fonts/Orbitron-Bold.ttf",
        ]
    
    for path in font_paths:
        expanded = os.path.expanduser(path)
        if os.path.exists(expanded):
            try:
                return ImageFont.truetype(expanded, size)
            except:
                continue
    
    # Try Space Mono (bold if requested)
    if bold:
        space_mono_paths = [
            "/System/Library/Fonts/Supplemental/SpaceMono-Bold.ttf",
            "/Library/Fonts/SpaceMono-Bold.ttf",
            "/System/Library/Fonts/Supplemental/SpaceMono-Regular.ttf",
            "/Library/Fonts/SpaceMono-Regular.ttf",
        ]
    else:
        space_mono_paths = [
            "/System/Library/Fonts/Supplemental/SpaceMono-Regular.ttf",
            "/Library/Fonts/SpaceMono-Regular.ttf",
            "/System/Library/Fonts/Supplemental/SpaceMono-Bold.ttf",
            "/Library/Fonts/SpaceMono-Bold.ttf",
        ]
    
    for path in space_mono_paths:
        expanded = os.path.expanduser(path)
        if os.path.exists(expanded):
            try:
                return ImageFont.truetype(expanded, size)
            except:
                continue
    
    # Try other monospace fonts
    monospace_paths = [
        "/System/Library/Fonts/Supplemental/Courier New Bold.ttf",
        "/System/Library/Fonts/Supplemental/Courier New.ttf",
        "/System/Library/Fonts/Supplemental/Menlo.ttc",
        "/System/Library/Fonts/Monaco.dfont",
    ]
    
    for path in monospace_paths:
        if os.path.exists(path):
            try:
                return ImageFont.truetype(path, size)
            except:
                continue
    
    # Fallback to default font (will be basic but functional)
    return ImageFont.load_default()

def add_text_with_shadow(draw, text, position, font, fill_color, shadow_color, shadow_offset=(2, 2)):
    """
    @brief Draw text with shadow effect
    @param draw ImageDraw object
    @param text Text to draw
    @param position (x, y) position for text
    @param font Font object
    @param fill_color Text color (RGB tuple)
    @param shadow_color Shadow color (RGB tuple)
    @param shadow_offset (x, y) offset for shadow
    @note Draws shadow first, then text on top
    """
    x, y = position
    shadow_x, shadow_y = shadow_offset
    
    # Draw shadow
    draw.text((x + shadow_x, y + shadow_y), text, font=font, fill=shadow_color)
    # Draw main text
    draw.text((x, y), text, font=font, fill=fill_color)

def add_title_and_logo(input_path, output_path, plugin_name, config):
    """
    @brief Add title text and logo to square image
    @param input_path Path to input square image
    @param output_path Path to save output image
    @param plugin_name Plugin name (for config lookup)
    @param config Plugin configuration dict
    @returns True if successful, False otherwise
    """
    try:
        # Open the square background image
        img = Image.open(input_path).convert('RGBA')
        width, height = img.size
        
        # Apply background zoom if configured (crop center, scale up for zoomed-in effect)
        zoom = config.get("background_zoom", 1.0)
        if zoom > 1.0:
            scale = zoom
            new_w, new_h = int(width * scale), int(height * scale)
            zoomed = img.resize((new_w, new_h), Image.Resampling.LANCZOS)
            left = (new_w - width) // 2
            top = (new_h - height) // 2
            img = zoomed.crop((left, top, left + width, top + height))
        
        # Add full-image darkening mask to make background darker
        dark_overlay = Image.new('RGBA', (width, height), (0, 0, 0, 130))  # Dark overlay with ~51% opacity
        img = Image.alpha_composite(img, dark_overlay)
        
        # Load logo
        if not os.path.exists(LOGO_PATH):
            print(f"✗ Logo not found: {LOGO_PATH}")
            return False
        
        logo = Image.open(LOGO_PATH).convert('RGBA')
        
        # Resize logo to appropriate size (about 120px width, maintain aspect ratio)
        logo_width = int(width * 0.12)  # 12% of image width
        logo_aspect = logo.height / logo.width
        logo_height = int(logo_width * logo_aspect)
        logo = logo.resize((logo_width, logo_height), Image.Resampling.LANCZOS)
        
        # Create a drawing context
        draw = ImageDraw.Draw(img)
        
        # Position logo in top left (with padding)
        logo_padding = int(width * 0.02)  # 2% padding
        logo_x = logo_padding
        logo_y = logo_padding
        
        # Calculate font size (even larger - responsive to image size)
        # Base size is 144px for 1200px width, scale proportionally (6x larger than original)
        base_font_size = int((width / 1200) * 144)
        font = get_font(base_font_size, bold=False, font_preference=config.get("font"), font_path=config.get("font_path"))
        
        # Support split title (e.g. CYMA teal + SYNTH white for Cymasynth)
        title_parts = config.get("title_parts")
        if title_parts:
            # Compute total width and part widths for centering
            part_widths = []
            for part in title_parts:
                bbox = draw.textbbox((0, 0), part["text"], font=font)
                part_widths.append(bbox[2] - bbox[0])
            text_width = sum(part_widths)
            text = "".join(p["text"] for p in title_parts)  # For mask height
        else:
            text = config["title"]
            bbox = draw.textbbox((0, 0), text, font=font)
            text_width = bbox[2] - bbox[0]
            text_color = hex_to_rgb(config["color"])
        
        # Position title below the logo row (centered horizontally)
        title_spacing = int(height * 0.02)  # 2% spacing below logo
        title_y = logo_y + logo_height + title_spacing
        title_x = (width - text_width) // 2  # Centered horizontally
        
        # Paste logo onto image
        img.paste(logo, (logo_x, logo_y), logo)
        
        # Create contrast mask behind text
        mask_x = title_x
        mask_y = title_y
        mask_width = text_width
        mask_height = int(base_font_size * 1.1)  # 10% taller than font size
        
        # Create mask layer with semi-transparent dark background
        mask_layer = Image.new('RGBA', img.size, (0, 0, 0, 0))
        mask_draw = ImageDraw.Draw(mask_layer)
        mask_draw.rectangle(
            [(mask_x, mask_y), (mask_x + mask_width, mask_y + mask_height)],
            fill=(0, 0, 0, 180)  # Dark background with 70% opacity (180/255)
        )
        mask_layer = mask_layer.filter(ImageFilter.GaussianBlur(radius=8))
        img = Image.alpha_composite(img, mask_layer)
        
        # Draw text with glow effect
        text_layer = Image.new('RGBA', img.size, (0, 0, 0, 0))
        text_draw = ImageDraw.Draw(text_layer)
        offsets = [(0, 3), (0, -3), (3, 0), (-3, 0), (2, 2), (-2, -2), (2, -2), (-2, 2),
                   (1, 3), (-1, -3), (3, 1), (-3, -1)]
        
        if title_parts:
            # Draw each part with its own color (CYMA teal, SYNTH white)
            x_offset = title_x
            for i, part in enumerate(title_parts):
                part_text = part["text"]
                part_color = hex_to_rgb(part["color"])
                glow_color = (*part_color, 150)
                for dx, dy in offsets:
                    text_draw.text((x_offset + dx, title_y + dy), part_text, font=font, fill=glow_color)
                x_offset += part_widths[i]
            text_layer = text_layer.filter(ImageFilter.GaussianBlur(radius=3))
            text_draw = ImageDraw.Draw(text_layer)
            x_offset = title_x
            for i, part in enumerate(title_parts):
                part_text = part["text"]
                part_color = hex_to_rgb(part["color"])
                text_draw.text((x_offset, title_y), part_text, font=font, fill=(*part_color, 255))
                x_offset += part_widths[i]
        else:
            # Single-color title (original behavior)
            shadow_rgb = tuple(max(0, c - 50) for c in text_color)
            shadow_color = (*shadow_rgb, 180)
            glow_color = (*text_color, 150)
            for dx, dy in offsets:
                text_draw.text((title_x + dx, title_y + dy), text, font=font, fill=glow_color)
            text_layer = text_layer.filter(ImageFilter.GaussianBlur(radius=3))
            text_draw = ImageDraw.Draw(text_layer)
            text_draw.text((title_x, title_y), text, font=font, fill=(*text_color, 255))
        
        # Composite text layer onto main image
        img = Image.alpha_composite(img, text_layer)
        
        # Add screenshot below the title (if it exists)
        screenshot_filename = config.get("screenshot", f"{plugin_name.capitalize()}.png")
        screenshot_path = os.path.join(SCREENSHOT_DIR, screenshot_filename)
        if os.path.exists(screenshot_path):
            try:
                screenshot = Image.open(screenshot_path).convert('RGBA')
                
                # Calculate screenshot size (about 80% of image width, maintain aspect ratio)
                screenshot_width = int(width * 0.8)
                screenshot_aspect = screenshot.height / screenshot.width
                screenshot_height = int(screenshot_width * screenshot_aspect)
                
                # Ensure screenshot doesn't exceed available space
                max_height = height - title_y - base_font_size - int(height * 0.08)  # Leave 8% margin at bottom
                if screenshot_height > max_height:
                    screenshot_height = max_height
                    screenshot_width = int(screenshot_height / screenshot_aspect)
                
                screenshot = screenshot.resize((screenshot_width, screenshot_height), Image.Resampling.LANCZOS)
                
                # Position screenshot below title (centered horizontally)
                screenshot_y = title_y + base_font_size + int(height * 0.03) + config.get("screenshot_offset_y", 0)  # 3% spacing + optional offset
                screenshot_x = (width - screenshot_width) // 2
                
                # Create dark mask behind screenshot (same style as title mask)
                mask_x = screenshot_x
                mask_y = screenshot_y
                mask_width = screenshot_width
                mask_height = screenshot_height
                
                # Create mask layer with semi-transparent dark background
                screenshot_mask_layer = Image.new('RGBA', img.size, (0, 0, 0, 0))
                screenshot_mask_draw = ImageDraw.Draw(screenshot_mask_layer)
                
                # Draw the mask (dark semi-transparent background) - same size as screenshot
                screenshot_mask_draw.rectangle(
                    [(mask_x, mask_y), (mask_x + mask_width, mask_y + mask_height)],
                    fill=(0, 0, 0, 180)  # Dark background with 70% opacity (180/255)
                )
                
                # Apply blur to mask edges for smoother appearance
                screenshot_mask_layer = screenshot_mask_layer.filter(ImageFilter.GaussianBlur(radius=8))
                
                # Composite mask onto main image first
                img = Image.alpha_composite(img, screenshot_mask_layer)
                
                # Paste the original screenshot on top
                img.paste(screenshot, (screenshot_x, screenshot_y), screenshot)
                
            except Exception as e:
                print(f"  ⚠ Could not add screenshot: {e}")
        
        # Save the image
        img.save(output_path, 'PNG', quality=95)
        print(f"✓ Created: {os.path.basename(output_path)}")
        return True
        
    except Exception as e:
        print(f"✗ Error processing {input_path}: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """
    @brief Main function to process all square images
    @returns None
    """
    print("Adding titles and logos to square background images...")
    print(f"Source: {SQUARE_DIR}")
    print(f"Logo: {LOGO_PATH}")
    print(f"Output: {OUTPUT_DIR}\n")
    
    if not os.path.exists(LOGO_PATH):
        print(f"✗ Logo file not found: {LOGO_PATH}")
        return
    
    success_count = 0
    total_count = 0
    
    for plugin_key, config in PLUGIN_CONFIG.items():
        total_count += 1
        input_name = f"{plugin_key}-background-square.png"
        output_name = f"{plugin_key}-background-square-with-title.png"
        input_path = os.path.join(SQUARE_DIR, input_name)
        output_path = os.path.join(OUTPUT_DIR, output_name)
        
        if os.path.exists(input_path):
            if add_title_and_logo(input_path, output_path, plugin_key, config):
                success_count += 1
        else:
            print(f"✗ Input image not found: {input_path}")
    
    print(f"\n{'='*60}")
    print(f"Completed: {success_count}/{total_count} images processed")
    print(f"Images with titles and logos saved to: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
