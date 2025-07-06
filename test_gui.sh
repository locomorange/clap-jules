#!/bin/bash
set -e

echo "=== Testing GUI Rendering Functionality ==="

# Set up virtual display for testing
export DISPLAY=:99
Xvfb :99 -screen 0 1024x768x24 &
XVFB_PID=$!
sleep 2

echo "Virtual display started with PID $XVFB_PID"

# Create simple test program that demonstrates GUI functionality
cd /home/runner/work/clap-jules/clap-jules

cat > /tmp/test_gui_simple.cpp << 'EOF'
#include "brisk/brisk.h"
#include <memory>
#include <chrono>
#include <thread>

int main() {
    printf("Testing basic Brisk GUI functionality...\n");
    
    try {
        // Initialize brisk
        brisk::Application::Initialize();
        printf("✓ Brisk application initialized\n");
        
        // Create window
        auto window = std::make_unique<brisk::Window>(nullptr);
        printf("✓ Window created\n");
        
        // Set window size and make visible
        window->SetSize(400, 300);
        window->SetVisible(true);
        printf("✓ Window made visible (400x300)\n");
        
        // Create a knob
        auto knob = std::make_unique<brisk::Knob>(20.0, 20000.0, 1000.0);
        knob->SetPosition(200, 150);
        knob->SetRadius(40);
        printf("✓ Knob created at center (1000 Hz)\n");
        
        // Draw some content
        printf("Drawing GUI content...\n");
        window->BeginDraw();
        window->DrawText(10, 25, "CLAP Plugin GUI Test");
        window->DrawText(10, 50, "Testing Frequency Knob Rendering");
        window->DrawText(150, 80, "Cutoff: 1000.0 Hz");
        
        // Draw the knob
        knob->Draw(window.get());
        
        window->EndDraw();
        printf("✓ GUI content drawn successfully\n");
        
        // Update knob value and redraw
        knob->SetValue(2500.0);
        printf("✓ Knob value updated to 2500 Hz\n");
        
        window->BeginDraw();
        window->DrawText(150, 80, "Cutoff: 2500.0 Hz");
        knob->Draw(window.get());
        window->EndDraw();
        printf("✓ GUI updated with new knob value\n");
        
        // Give some time for rendering
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        // Hide the window
        window->SetVisible(false);
        printf("✓ Window hidden\n");
        
        brisk::Application::Shutdown();
        printf("✓ Brisk test completed successfully\n");
        return 0;
        
    } catch (const std::exception& e) {
        printf("✗ Exception during Brisk test: %s\n", e.what());
        return 1;
    }
}
EOF

# Compile the simplified test
echo "Compiling simplified GUI test..."
g++ -std=c++17 -I./libs/brisk/include \
    /tmp/test_gui_simple.cpp \
    build/libbrisk.a \
    -lX11 -lcairo -lpthread \
    -o /tmp/test_gui_simple

# Run the test
echo "Running simplified GUI test..."
/tmp/test_gui_simple

# Take a screenshot to verify display is working
if command -v import >/dev/null 2>&1; then
    echo "Taking screenshot..."
    import -window root /tmp/gui_test_screenshot.png 2>/dev/null || echo "Screenshot failed (expected if no windows visible)"
fi

# Cleanup
kill $XVFB_PID 2>/dev/null || true
wait $XVFB_PID 2>/dev/null || true

echo "✓ GUI test completed"