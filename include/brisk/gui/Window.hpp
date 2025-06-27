#pragma once

#include "brisk/core/BasicTypes.hpp"
#include <memory>
#include <functional>
#include <string>

namespace brisk {

class Widget {
public:
    virtual ~Widget() = default;
};

class Layout : public Widget {
public:
    virtual void addWidget(std::shared_ptr<Widget> widget) {}
};

class VBoxLayout : public Layout {
public:
    void addWidget(std::shared_ptr<Widget> widget) override {}
};

class HBoxLayout : public Layout {
public:
    void addWidget(std::shared_ptr<Widget> widget) override {}
};

class Window {
public:
    Window(WindowStyle style, Size size, const std::string& title) {}
    virtual ~Window() = default;
    
    void setVisible(bool visible) {}
    bool visible() const { return false; }
    void setSize(Size size) {}
    void setPosition(Point pos) {}
    void* nativeHandle() const { return nullptr; }
    void setLayout(std::shared_ptr<Layout> layout) {}
};

}