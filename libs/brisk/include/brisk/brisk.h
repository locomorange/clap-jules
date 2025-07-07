#pragma once

// Simple UI framework for CLAP plugin GUI
// Implements basic drawing primitives for frequency knob

// Standard library includes
#include <functional>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

// Platform-specific definitions
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Platform-specific includes
#ifdef __linux__
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <cairo/cairo.h>
    #include <cairo/cairo-xlib.h>
#endif

namespace brisk {

class Application {
public:
    static void Initialize();
    static void Shutdown();
    static bool IsInitialized();
    
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
    Knob(double min_val, double max_val, double initial_val);
    
    void SetValue(double value);
    double GetValue() const;
    void SetCallback(std::function<void(double)> callback);
    void SetPosition(int x, int y);
    void SetRadius(int radius);
    void Draw(Window* window);
    bool NeedsRedraw() const;
    
private:
    double min_value_;
    double max_value_;
    double current_value_;
    std::function<void(double)> callback_;
    int x_, y_, radius_;
    bool needs_redraw_;
};

} // namespace brisk
