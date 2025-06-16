#include "graphics_wrapper.h"
#include <iostream>
#include <fstream>

/**
 * Simple example demonstrating Skia integration through the graphics wrapper.
 * This example creates a simple image with various shapes and saves the pixel data.
 */
int main() {
    std::cout << "Skia Integration Example\n";
    std::cout << "========================\n\n";
    
    // Check if Skia is available
    if (clap_jules::GraphicsContext::isSkiaAvailable()) {
        std::cout << "✓ Skia graphics acceleration is available\n";
    } else {
        std::cout << "✓ Using software graphics fallback\n";
    }
    
    // Create a graphics context
    const int width = 200;
    const int height = 200;
    clap_jules::GraphicsContext graphics(width, height);
    
    std::cout << "Created " << width << "x" << height << " graphics context\n";
    
    // Draw a sample scene
    graphics.clear(0xFF2D2D2D);  // Dark gray background
    
    // Draw a gradient-like effect with rectangles
    for (int i = 0; i < 10; i++) {
        uint8_t intensity = 50 + i * 20;
        uint32_t color = 0xFF000000 | (intensity << 16) | (intensity << 8) | intensity;
        graphics.drawRect(20 + i * 5, 20 + i * 5, 100 - i * 10, 100 - i * 10, color);
    }
    
    // Draw some colorful shapes
    graphics.drawCircle(150, 50, 30, 0xFF4169E1);   // Royal blue circle
    graphics.drawCircle(150, 150, 25, 0xFFFFD700);  // Gold circle
    graphics.drawRect(120, 120, 60, 20, 0xFFDC143C); // Crimson rectangle
    
    // Draw some lines
    graphics.drawLine(0, 0, width-1, height-1, 0xFFFF4500);     // Orange diagonal
    graphics.drawLine(width-1, 0, 0, height-1, 0xFF32CD32);    // Lime green diagonal
    
    // Border
    graphics.drawRect(0, 0, width, 2, 0xFFFFFFFF);              // Top border
    graphics.drawRect(0, height-2, width, 2, 0xFFFFFFFF);       // Bottom border
    graphics.drawRect(0, 0, 2, height, 0xFFFFFFFF);             // Left border
    graphics.drawRect(width-2, 0, 2, height, 0xFFFFFFFF);       // Right border
    
    std::cout << "Drew sample graphics scene\n";
    
    // Get the pixel data
    const uint32_t* pixels = graphics.getPixels();
    if (pixels) {
        // Save to a simple PPM file for visualization
        std::ofstream file("example_output.ppm", std::ios::binary);
        file << "P6\n" << width << " " << height << "\n255\n";
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                uint32_t pixel = pixels[y * width + x];
                uint8_t r = (pixel >> 16) & 0xFF;
                uint8_t g = (pixel >> 8) & 0xFF;
                uint8_t b = pixel & 0xFF;
                file.write(reinterpret_cast<char*>(&r), 1);
                file.write(reinterpret_cast<char*>(&g), 1);
                file.write(reinterpret_cast<char*>(&b), 1);
            }
        }
        
        std::cout << "✓ Saved example output to 'example_output.ppm'\n";
        std::cout << "  (You can view this file with image viewers that support PPM format)\n";
    } else {
        std::cout << "✗ Failed to get pixel data\n";
        return 1;
    }
    
    std::cout << "\nExample completed successfully!\n";
    return 0;
}