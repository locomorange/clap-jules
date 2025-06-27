#pragma once

#include "brisk/gui/Window.hpp"
#include <string>
#include <functional>

namespace brisk {

class Label : public Widget {
public:
    Label(const std::string& text) {}
    void setText(const std::string& text) {}
    void setTextAlign(TextAlign align) {}
};

class Button : public Widget {
public:
    Button(const std::string& text) {}
    void setText(const std::string& text) {}
    std::function<void()> onClick;
};

class Slider : public Widget {
public:
    Slider(float min, float max, float value) {}
    void setValue(float value) {}
    float getValue() const { return 0.0f; }
    std::function<void(float)> onValueChanged;
};

}