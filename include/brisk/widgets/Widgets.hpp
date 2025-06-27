#pragma once

#include "brisk/gui/Window.hpp"
#include <string>
#include <functional>

// Forward declaration
namespace native {
    class NativeWindow;
}

namespace brisk {

class Label : public Widget {
public:
    Label(const std::string& text) : text_(text), align_(TextAlign::Left) {}
    
    void setText(const std::string& text) { text_ = text; }
    void setTextAlign(TextAlign align) { align_ = align; }
    
    void render(native::NativeWindow* window, int x, int y) override;
    
private:
    std::string text_;
    TextAlign align_;
};

class Button : public Widget {
public:
    Button(const std::string& text) : text_(text) {}
    
    void setText(const std::string& text) { text_ = text; }
    void render(native::NativeWindow* window, int x, int y) override;
    
    std::function<void()> onClick;
    
private:
    std::string text_;
};

class Slider : public Widget {
public:
    Slider(float min, float max, float value) 
        : min_(min), max_(max), value_(value) {}
    
    void setValue(float value) { 
        if (value >= min_ && value <= max_) {
            value_ = value;
            if (onValueChanged) {
                onValueChanged(value_);
            }
        }
    }
    
    float getValue() const { return value_; }
    void render(native::NativeWindow* window, int x, int y) override;
    
    std::function<void(float)> onValueChanged;
    
private:
    float min_, max_, value_;
};

}