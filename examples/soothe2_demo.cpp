#include "graphics/skia_graphics.h"
#include "graphics/soothe2_gui.h"
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

using namespace clap_jules::graphics;
using namespace clap_jules::ui;

void simulateMouseEvent(Soothe2GUI& gui, float x, float y, bool pressed) {
    MouseEvent event;
    event.x = x;
    event.y = y;
    event.pressed = pressed;
    event.dragging = pressed;
    event.button = 0; // Left button
    
    gui.handleMouse(event);
}

int main() {
    std::cout << "Soothe2 GUI Demo\n";
    std::cout << "================\n";
    
    // Display backend information
    std::cout << "Graphics Backend: " << getGraphicsBackendInfo() << "\n";
    std::cout << "Skia Available: " << (isSkiaAvailable() ? "Yes" : "No") << "\n\n";
    
    const int width = 800;
    const int height = 600;
    
    // Create a graphics context
    auto graphics = createGraphicsContext(width, height);
    
    if (!graphics) {
        std::cerr << "Failed to create graphics context\n";
        return 1;
    }
    
    std::cout << "Created " << width << "x" << height << " graphics context\n";
    
    // Create Soothe2 GUI
    Soothe2GUI soothe2(width, height);
    std::cout << "Created Soothe2 GUI\n";
    
    // Set up parameter change callback
    soothe2.setParameterChangeCallback([](const std::string& paramName, float value) {
        std::cout << "Parameter changed: " << paramName << " = " << value << "\n";
    });
    
    // Simulate some parameter changes
    std::cout << "\nDemonstrating parameter changes:\n";
    
    // Get direct access to parameters
    auto& params = soothe2.params;
    
    // Change some parameters to show the GUI updates
    params.depth = 0.7f;
    params.selectivity = 0.3f;
    params.attack = 0.5f;
    params.release = 0.8f;
    params.mix = 0.9f;
    params.sharpness = 0.4f;
    
    // Update EQ bands
    params.band1.frequency = 150.0f;
    params.band1.gain = -3.0f;
    params.band2.frequency = 1200.0f;
    params.band2.gain = 2.5f;
    params.band3.frequency = 4800.0f;
    params.band3.gain = -1.5f;
    params.band4.frequency = 15000.0f;
    params.band4.gain = 1.0f;
    
    std::cout << "Set up demo parameters\n";
    
    // Render the GUI multiple times to show animation
    const int numFrames = 5;
    for (int frame = 0; frame < numFrames; ++frame) {
        std::cout << "\nRendering frame " << (frame + 1) << "/" << numFrames << "...\n";
        
        // Update timing
        float deltaTime = 0.016f; // ~60 FPS
        soothe2.update(deltaTime);
        
        // Clear and render
        graphics->clear(Color(20, 20, 25));
        soothe2.draw(graphics.get());
        
        // Simulate some mouse interactions
        if (frame == 1) {
            std::cout << "Simulating mouse click on depth knob...\n";
            simulateMouseEvent(soothe2, 65, 120, true);  // Click down
            simulateMouseEvent(soothe2, 65, 110, true);  // Drag up
            simulateMouseEvent(soothe2, 65, 110, false); // Release
        }
        
        if (frame == 3) {
            std::cout << "Simulating mouse drag on EQ graph...\n";
            simulateMouseEvent(soothe2, 500, 200, true);  // Click on EQ area
            simulateMouseEvent(soothe2, 520, 180, true);  // Drag to new position
            simulateMouseEvent(soothe2, 520, 180, false); // Release
        }
        
        // Small delay to simulate real-time rendering
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::cout << "\nGUI rendering completed successfully!\n";
    
    // Display current parameter values
    std::cout << "\nCurrent parameter values:\n";
    std::cout << "  Depth: " << params.depth << "\n";
    std::cout << "  Selectivity: " << params.selectivity << "\n";
    std::cout << "  Attack: " << params.attack << "\n";
    std::cout << "  Release: " << params.release << "\n";
    std::cout << "  Mix: " << params.mix << "\n";
    std::cout << "  Sharpness: " << params.sharpness << "\n";
    std::cout << "  Bypass: " << (params.bypass ? "ON" : "OFF") << "\n";
    std::cout << "  Solo: " << (params.solo ? "ON" : "OFF") << "\n";
    std::cout << "  Delta: " << (params.delta ? "ON" : "OFF") << "\n";
    
    std::cout << "\nEQ Bands:\n";
    std::cout << "  Band 1: " << params.band1.frequency << " Hz, " << params.band1.gain << " dB, Q=" << params.band1.q << "\n";
    std::cout << "  Band 2: " << params.band2.frequency << " Hz, " << params.band2.gain << " dB, Q=" << params.band2.q << "\n";
    std::cout << "  Band 3: " << params.band3.frequency << " Hz, " << params.band3.gain << " dB, Q=" << params.band3.q << "\n";
    std::cout << "  Band 4: " << params.band4.frequency << " Hz, " << params.band4.gain << " dB, Q=" << params.band4.q << "\n";
    
    std::cout << "\nSoothe2 GUI demo completed!\n";
    return 0;
}