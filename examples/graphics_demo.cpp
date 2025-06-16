#include "graphics/skia_graphics.h"
#include <iostream>
#include <memory>
#include <chrono>

using namespace clap_jules::graphics;

int main() {
    std::cout << "CLAP-Jules Graphics Demo\n";
    std::cout << "========================\n";
    
    // Display backend information
    std::cout << "Graphics Backend: " << getGraphicsBackendInfo() << "\n";
    std::cout << "Skia Available: " << (isSkiaAvailable() ? "Yes" : "No") << "\n\n";
    
    // Create a graphics context
    auto graphics = createGraphicsContext(320, 240);
    
    if (!graphics) {
        std::cerr << "Failed to create graphics context\n";
        return 1;
    }
    
    std::cout << "Created 320x240 graphics context\n";
    
    // Demonstrate basic drawing operations
    std::cout << "Performing drawing operations...\n";
    
    // Clear background
    graphics->clear(Color(30, 30, 40)); // Dark blue-gray background
    
    // Draw some shapes
    graphics->drawRect(Rect(50, 50, 100, 60), Color(255, 100, 100)); // Red rectangle
    graphics->drawCircle(Point(200, 120), 40, Color(100, 255, 100)); // Green circle
    graphics->drawLine(Point(20, 200), Point(300, 200), Color(100, 100, 255), 3.0f); // Blue line
    
    // Draw text
    graphics->drawText("CLAP-Jules + Skia", Point(60, 30), Color(255, 255, 255), 18.0f);
    graphics->drawText("2D Graphics Demo", Point(70, 180), Color(200, 200, 200), 14.0f);
    
    // Demonstrate state management
    graphics->save();
    graphics->translate(160, 120); // Move to center
    graphics->rotate(0.5f); // Rotate 45 degrees
    graphics->drawRect(Rect(-25, -25, 50, 50), Color(255, 255, 100, 128)); // Semi-transparent yellow square
    graphics->restore();
    
    std::cout << "Drawing operations completed successfully!\n";
    
    // Performance test
    std::cout << "\nPerforming performance test (1000 circles)...\n";
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        float x = (i * 7) % 320;
        float y = (i * 11) % 240;
        float radius = 1 + (i % 5);
        Color color(i % 255, (i * 2) % 255, (i * 3) % 255, 100);
        graphics->drawCircle(Point(x, y), radius, color);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Performance test completed in " << duration.count() << " microseconds\n";
    std::cout << "Average: " << (duration.count() / 1000.0) << " microseconds per circle\n";
    
    std::cout << "\nGraphics demo completed successfully!\n";
    return 0;
}