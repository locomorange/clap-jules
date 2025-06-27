#include "brisk/widgets/Widgets.hpp"
#include "native/native_window.hpp"
#include <sstream>
#include <iomanip>

namespace brisk {

void Label::render(native::NativeWindow* window, int x, int y) {
    if (!window || text_.empty()) {
        return;
    }
    
    // Adjust x position based on alignment (simple implementation)
    int renderX = x;
    if (align_ == TextAlign::Center) {
        renderX = x + 50; // Simple center offset
    } else if (align_ == TextAlign::Right) {
        renderX = x + 100; // Simple right offset
    }
    
    window->drawText(text_, renderX, y + 15); // +15 for baseline
}

void Button::render(native::NativeWindow* window, int x, int y) {
    if (!window) {
        return;
    }
    
    // Draw button rectangle
    window->drawRect(x, y, 100, 25);
    
    // Draw button text
    if (!text_.empty()) {
        window->drawText(text_, x + 10, y + 17); // Center text in button
    }
}

void Slider::render(native::NativeWindow* window, int x, int y) {
    if (!window) {
        return;
    }
    
    // Draw slider track
    window->drawRect(x, y + 10, 150, 5);
    
    // Calculate slider position
    float normalizedValue = (value_ - min_) / (max_ - min_);
    int sliderPos = x + static_cast<int>(normalizedValue * 140); // 140 = 150 - 10 (thumb width)
    
    // Draw slider thumb
    window->drawRect(sliderPos, y + 5, 10, 15);
    
    // Draw value text
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value_;
    window->drawText(oss.str(), x + 160, y + 15);
}

}