#pragma once

#include "brisk/core/BasicTypes.hpp"
#include <memory>
#include <functional>
#include <string>
#include <vector>

// Forward declaration
namespace native {
    class NativeWindow;
}

namespace brisk {

class Widget {
public:
    virtual ~Widget() = default;
    virtual void render(native::NativeWindow* window, int x, int y) {}
};

class Layout : public Widget {
public:
    virtual void addWidget(std::shared_ptr<Widget> widget) {
        widgets_.push_back(widget);
    }
    
    void render(native::NativeWindow* window, int x, int y) override {
        int currentY = y;
        for (auto& widget : widgets_) {
            if (widget) {
                widget->render(window, x, currentY);
                currentY += 30; // Simple spacing
            }
        }
    }
    
protected:
    std::vector<std::shared_ptr<Widget>> widgets_;
};

class VBoxLayout : public Layout {
public:
    void render(native::NativeWindow* window, int x, int y) override {
        int currentY = y;
        for (auto& widget : widgets_) {
            if (widget) {
                widget->render(window, x, currentY);
                currentY += 35; // Vertical spacing
            }
        }
    }
};

class HBoxLayout : public Layout {
public:
    void render(native::NativeWindow* window, int x, int y) override {
        int currentX = x;
        for (auto& widget : widgets_) {
            if (widget) {
                widget->render(window, currentX, y);
                currentX += 120; // Horizontal spacing
            }
        }
    }
};

class Window {
public:
    Window(WindowStyle style, Size size, const std::string& title);
    virtual ~Window();
    
    void setVisible(bool visible);
    bool visible() const;
    void setSize(Size size);
    void setPosition(Point pos);
    void* nativeHandle() const;
    void setLayout(std::shared_ptr<Layout> layout);
    
    void render(); // Render all widgets
    
private:
    std::unique_ptr<native::NativeWindow> nativeWindow_;
    std::shared_ptr<Layout> layout_;
    Size size_;
    bool visible_;
};

}