#!/usr/bin/env python3
"""
@fileoverview Create square versions of plugin background images
@module Square Background Generator

Crops each background image to a square by using the inner square frame
(centered crop using the smaller dimension).
"""

import os
from PIL import Image

# Define the source and output directories
SOURCE_DIR = "_Shared/Assets/backgrounds"
OUTPUT_DIR = "_Shared/Assets/backgrounds/square"

# List of all plugin background images
BACKGROUND_IMAGES = [
    "apogee-background.png",
    "cymasynth-background.png",
    "eclipse-background.png",
    "ion-background.png",
    "kepler-background.png",
    "lagrange-background.png",
    "perihelion-background.png",
    "retrograde-background.png",
    "tidal-background.png",
]

def create_square_image(input_path, output_path):
    """
    @brief Crop an image to a square using the inner square frame
    @param input_path Path to the source image
    @param output_path Path where the square image will be saved
    @note Uses the smaller dimension (width or height) to create a centered square crop
    """
    try:
        # Open the image
        img = Image.open(input_path)
        width, height = img.size
        
        # Determine the size of the square (use the smaller dimension)
        size = min(width, height)
        
        # Calculate the crop box (centered)
        left = (width - size) // 2
        top = (height - size) // 2
        right = left + size
        bottom = top + size
        
        # Crop to square
        square_img = img.crop((left, top, right, bottom))
        
        # Save the square image
        square_img.save(output_path, quality=95)
        print(f"✓ Created square version: {os.path.basename(output_path)} ({size}x{size})")
        
        return True
    except Exception as e:
        print(f"✗ Error processing {input_path}: {e}")
        return False

def main():
    """
    @brief Main function to process all background images
    @returns None
    """
    # Create output directory if it doesn't exist
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    print("Creating square versions of background images...")
    print(f"Source: {SOURCE_DIR}")
    print(f"Output: {OUTPUT_DIR}\n")
    
    success_count = 0
    total_count = len(BACKGROUND_IMAGES)
    
    for image_name in BACKGROUND_IMAGES:
        input_path = os.path.join(SOURCE_DIR, image_name)
        output_name = image_name.replace(".png", "-square.png")
        output_path = os.path.join(OUTPUT_DIR, output_name)
        
        if os.path.exists(input_path):
            if create_square_image(input_path, output_path):
                success_count += 1
        else:
            print(f"✗ Source image not found: {input_path}")
    
    print(f"\n{'='*60}")
    print(f"Completed: {success_count}/{total_count} images processed")
    print(f"Square images saved to: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
