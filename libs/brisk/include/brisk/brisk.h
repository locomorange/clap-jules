#pragma once

// Simple UI framework for CLAP plugin GUI
// Implements basic drawing primitives for frequency knob

#include <functional>
#include <algorithm>
#include <cmath>
#include <cstdio>

#ifdef __linux__
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <cairo/cairo.h>
    #include <cairo/cairo-xlib.h>
#endif

namespace brisk {

class Application {
public:
    static void Initialize() {
        printf("Brisk: Application initialized\n");
        initialized_ = true;
    }
    
    static void Shutdown() {
        printf("Brisk: Application shutdown\n");
        initialized_ = false;
    }
    
    static bool IsInitialized() { return initialized_; }
    
private:
    static bool initialized_;
};

class Window {
public:
    Window(void* parent_handle);
    ~Window();
    
    void SetVisible(bool visible);
    void* GetHandle() const;
    void Render();
    void SetSize(int width, int height);
    
    // Drawing context access
    void BeginDraw();
    void EndDraw();
    void DrawKnob(int x, int y, int radius, double value, double min_val, double max_val);
    void DrawText(int x, int y, const char* text);
    
private:
    void* parent_handle_;
    bool visible_;
    int width_, height_;
    
#ifdef __linux__
    Display* display_;
    ::Window window_;
    cairo_surface_t* surface_;
    cairo_t* cairo_ctx_;
    bool cairo_initialized_;
#endif
    
    void InitializePlatform();
    void CleanupPlatform();
};

class Knob {
public:
    Knob(double min_val, double max_val, double initial_val)
        : min_value_(min_val), max_value_(max_val), current_value_(initial_val),
          x_(0), y_(0), radius_(30), needs_redraw_(true) {}
    
    void SetValue(double value) {
        double old_value = current_value_;
        current_value_ = std::max(min_value_, std::min(max_value_, value));
        if (old_value != current_value_) {
            needs_redraw_ = true;
        }
    }
    
    double GetValue() const {
        return current_value_;
    }
    
    void SetCallback(std::function<void(double)> callback) {
        callback_ = callback;
    }
    
    void SetPosition(int x, int y) {
        x_ = x;
        y_ = y;
        needs_redraw_ = true;
    }
    
    void SetRadius(int radius) {
        radius_ = radius;
        needs_redraw_ = true;
    }
    
    void Draw(Window* window) {
        if (needs_redraw_) {
            window->DrawKnob(x_, y_, radius_, current_value_, min_value_, max_value_);
            needs_redraw_ = false;
        }
    }
    
    bool NeedsRedraw() const { return needs_redraw_; }
    
private:
    double min_value_;
    double max_value_;
    double current_value_;
    std::function<void(double)> callback_;
    int x_, y_, radius_;
    bool needs_redraw_;
};

} // namespace brisk
