#!/usr/bin/env python3
"""
@fileoverview Add glowy border to square product images
@module Glowy Border Generator

Adds a glowing border effect to the square product images with titles and logos.
"""

import os
from PIL import Image, ImageDraw, ImageFilter

# Define directories
SQUARE_DIR = "_Shared/Assets/backgrounds/square"
OUTPUT_DIR = "_Shared/Assets/backgrounds/square"

# Plugin configuration for border colors
PLUGIN_BORDER_COLORS = {
    "apogee": "#00d4ff",  # Orbit Cyan
    "eclipse": "#fbbf24",  # Corona Gold
    "ion": "#c084fc",      # Ion Core Purple
    "kepler": "#3b82f6",   # Celestial Blue
    "lagrange": "#0094ff",  # Stellar Blue
    "perihelion": "#ffa500", # Solar Gold
    "retrograde": "#6b2fb5", # Nebula Purple
    "tidal": "#1e40af"      # Deep Ocean Blue
}

def hex_to_rgb(hex_color):
    """Convert hex color to RGB tuple"""
    hex_color = hex_color.lstrip('#')
    return tuple(int(hex_color[i:i+2], 16) for i in (0, 2, 4))

def add_glowy_border(input_path, output_path, border_color, border_width=8, glow_intensity=20):
    """
    @brief Add glowy border to square image
    @param input_path Path to input image
    @param output_path Path to save output image
    @param border_color Hex color for border glow
    @param border_width Width of border in pixels
    @param glow_intensity Intensity of glow effect
    """
    try:
        # Open the image
        img = Image.open(input_path).convert('RGBA')
        width, height = img.size
        
        # Create a new image with padding for border
        border_padding = border_width + glow_intensity
        new_width = width + (border_padding * 2)
        new_height = height + (border_padding * 2)
        
        # Create new image with transparent background
        bordered_img = Image.new('RGBA', (new_width, new_height), (0, 0, 0, 0))
        
        # Paste original image centered
        bordered_img.paste(img, (border_padding, border_padding), img)
        
        # Create border layers for glow effect
        draw = ImageDraw.Draw(bordered_img)
        border_rgb = hex_to_rgb(border_color)
        
        # Draw multiple border layers for glow effect
        for i in range(glow_intensity, 0, -2):
            alpha = int(255 * (0.3 + (glow_intensity - i) / glow_intensity * 0.7))
            border_size = border_width + i
            
            # Draw border rectangle
            draw.rectangle(
                [(border_padding - border_size, border_padding - border_size),
                 (new_width - border_padding + border_size, new_height - border_padding + border_size)],
                outline=(*border_rgb, alpha),
                width=2
            )
        
        # Draw main border (brightest)
        draw.rectangle(
            [(border_padding - border_width, border_padding - border_width),
             (new_width - border_padding + border_width, new_height - border_padding + border_width)],
            outline=(*border_rgb, 255),
            width=border_width
        )
        
        # Apply blur to outer glow layers (create soft glow)
        # We'll do this by creating a separate glow layer
        glow_layer = Image.new('RGBA', (new_width, new_height), (0, 0, 0, 0))
        glow_draw = ImageDraw.Draw(glow_layer)
        
        # Draw glow effect
        for i in range(glow_intensity, 0, -1):
            alpha = int(100 * (i / glow_intensity))
            size = border_width + i * 2
            
            glow_draw.rectangle(
                [(border_padding - size, border_padding - size),
                 (new_width - border_padding + size, new_height - border_padding + size)],
                outline=(*border_rgb, alpha),
                width=1
            )
        
        # Blur the glow layer
        glow_layer = glow_layer.filter(ImageFilter.GaussianBlur(radius=glow_intensity * 0.5))
        
        # Composite glow layer
        bordered_img = Image.alpha_composite(bordered_img, glow_layer)
        
        # Draw sharp inner border
        draw = ImageDraw.Draw(bordered_img)
        draw.rectangle(
            [(border_padding - border_width, border_padding - border_width),
             (new_width - border_padding + border_width, new_height - border_padding + border_width)],
            outline=(*border_rgb, 255),
            width=border_width
        )
        
        # Save the image
        bordered_img.save(output_path, 'PNG', quality=95)
        print(f"✓ Added glowy border: {os.path.basename(output_path)}")
        return True
        
    except Exception as e:
        print(f"✗ Error processing {input_path}: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Main function to process all square images with titles"""
    print("Adding glowy borders to square product images...")
    print(f"Source: {SQUARE_DIR}")
    print(f"Output: {OUTPUT_DIR}\n")
    
    success_count = 0
    total_count = 0
    
    for plugin_key, border_color in PLUGIN_BORDER_COLORS.items():
        total_count += 1
        input_name = f"{plugin_key}-background-square-with-title.png"
        output_name = f"{plugin_key}-background-square-with-title.png"  # Overwrite
        input_path = os.path.join(SQUARE_DIR, input_name)
        output_path = os.path.join(OUTPUT_DIR, output_name)
        
        if os.path.exists(input_path):
            if add_glowy_border(input_path, output_path, border_color, border_width=10, glow_intensity=25):
                success_count += 1
        else:
            print(f"✗ Input image not found: {input_path}")
    
    print(f"\n{'='*60}")
    print(f"Completed: {success_count}/{total_count} images processed")
    print(f"Images with glowy borders saved to: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
