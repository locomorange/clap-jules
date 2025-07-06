#include "brisk/brisk.h"
#include <cstring>

namespace brisk {

// Static member definition
bool Application::initialized_ = false;

#ifdef __linux__

Window::Window(void* parent_handle) 
    : parent_handle_(parent_handle), visible_(false), width_(400), height_(300),
      display_(nullptr), window_(0), surface_(nullptr), cairo_ctx_(nullptr),
      cairo_initialized_(false) {
    InitializePlatform();
}

Window::~Window() {
    CleanupPlatform();
}

void Window::InitializePlatform() {
    printf("Brisk Window: Initializing Linux X11/Cairo window\n");
    
    // Open connection to X server
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
        printf("Brisk Window: Failed to open X display\n");
        return;
    }
    
    int screen = DefaultScreen(display_);
    ::Window parent = parent_handle_ ? *(::Window*)parent_handle_ : RootWindow(display_, screen);
    
    // Create window
    window_ = XCreateSimpleWindow(
        display_,
        parent,
        0, 0,           // x, y
        width_, height_, // width, height
        1,              // border_width
        BlackPixel(display_, screen),  // border
        WhitePixel(display_, screen)   // background
    );
    
    if (window_) {
        // Set window properties
        XStoreName(display_, window_, "CLAP Plugin GUI");
        
        // Select events
        XSelectInput(display_, window_, ExposureMask | KeyPressMask | ButtonPressMask);
        
        // Create Cairo surface for drawing
        surface_ = cairo_xlib_surface_create(display_, window_, 
                                           DefaultVisual(display_, screen),
                                           width_, height_);
        
        if (surface_) {
            cairo_ctx_ = cairo_create(surface_);
            if (cairo_ctx_) {
                cairo_initialized_ = true;
                printf("Brisk Window: Cairo context created successfully\n");
            }
        }
        
        printf("Brisk Window: X11 window created (ID: %lu)\n", window_);
    } else {
        printf("Brisk Window: Failed to create X11 window\n");
    }
}

void Window::CleanupPlatform() {
    if (cairo_ctx_) {
        cairo_destroy(cairo_ctx_);
        cairo_ctx_ = nullptr;
    }
    
    if (surface_) {
        cairo_surface_destroy(surface_);
        surface_ = nullptr;
    }
    
    if (window_ && display_) {
        XDestroyWindow(display_, window_);
        window_ = 0;
    }
    
    if (display_) {
        XCloseDisplay(display_);
        display_ = nullptr;
    }
    
    cairo_initialized_ = false;
}

void Window::SetVisible(bool visible) {
    visible_ = visible;
    
    if (display_ && window_) {
        if (visible) {
            XMapWindow(display_, window_);
            XFlush(display_);
            printf("Brisk Window: Window mapped (visible)\n");
        } else {
            XUnmapWindow(display_, window_);
            XFlush(display_);
            printf("Brisk Window: Window unmapped (hidden)\n");
        }
    }
}

void* Window::GetHandle() const {
    return window_ ? (void*)&window_ : parent_handle_;
}

void Window::SetSize(int width, int height) {
    width_ = width;
    height_ = height;
    
    if (display_ && window_) {
        XResizeWindow(display_, window_, width, height);
        
        // Recreate Cairo surface with new size
        if (surface_) {
            cairo_xlib_surface_set_size(surface_, width, height);
        }
        
        XFlush(display_);
        printf("Brisk Window: Resized to %dx%d\n", width, height);
    }
}

void Window::BeginDraw() {
    if (cairo_ctx_) {
        // Clear the surface
        cairo_set_source_rgb(cairo_ctx_, 0.2, 0.2, 0.2); // Dark gray background
        cairo_paint(cairo_ctx_);
    }
}

void Window::EndDraw() {
    if (display_ && cairo_ctx_) {
        cairo_surface_flush(surface_);
        XFlush(display_);
    }
}

void Window::DrawKnob(int x, int y, int radius, double value, double min_val, double max_val) {
    if (!cairo_ctx_) return;
    
    // Normalize value to 0-1 range
    double normalized = (value - min_val) / (max_val - min_val);
    normalized = std::max(0.0, std::min(1.0, normalized));
    
    // Angle range: -135° to +135° (270° total range)
    double angle = -2.356 + (normalized * 4.712); // -135° to +135° in radians
    
    // Draw knob body (circle)
    cairo_set_source_rgb(cairo_ctx_, 0.6, 0.6, 0.6); // Gray knob
    cairo_arc(cairo_ctx_, x, y, radius, 0, 2 * M_PI);
    cairo_fill(cairo_ctx_);
    
    // Draw knob outline
    cairo_set_source_rgb(cairo_ctx_, 0.3, 0.3, 0.3); // Darker outline
    cairo_set_line_width(cairo_ctx_, 2.0);
    cairo_arc(cairo_ctx_, x, y, radius, 0, 2 * M_PI);
    cairo_stroke(cairo_ctx_);
    
    // Draw value indicator (line from center to edge)
    cairo_set_source_rgb(cairo_ctx_, 1.0, 0.4, 0.0); // Orange indicator
    cairo_set_line_width(cairo_ctx_, 3.0);
    cairo_move_to(cairo_ctx_, x, y);
    cairo_line_to(cairo_ctx_, 
                  x + (radius * 0.8) * cos(angle),
                  y + (radius * 0.8) * sin(angle));
    cairo_stroke(cairo_ctx_);
    
    // Draw center dot
    cairo_set_source_rgb(cairo_ctx_, 0.1, 0.1, 0.1); // Dark center
    cairo_arc(cairo_ctx_, x, y, 3, 0, 2 * M_PI);
    cairo_fill(cairo_ctx_);
    
    printf("Brisk Window: Drew knob at (%d,%d) radius=%d value=%.1f (%.2f%%)\n", 
           x, y, radius, value, normalized * 100.0);
}

void Window::DrawText(int x, int y, const char* text) {
    if (!cairo_ctx_ || !text) return;
    
    cairo_set_source_rgb(cairo_ctx_, 1.0, 1.0, 1.0); // White text
    cairo_select_font_face(cairo_ctx_, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cairo_ctx_, 12.0);
    cairo_move_to(cairo_ctx_, x, y);
    cairo_show_text(cairo_ctx_, text);
}

void Window::Render() {
    if (!cairo_initialized_) {
        printf("Brisk Window: Cannot render - Cairo not initialized\n");
        return;
    }
    
    BeginDraw();
    
    // Draw title
    DrawText(10, 25, "CLAP Low-Pass Filter");
    
    // Default knob will be drawn by the Knob class
    
    EndDraw();
    printf("Brisk Window: Render completed\n");
}

#else // Windows/macOS placeholder

Window::Window(void* parent_handle) 
    : parent_handle_(parent_handle), visible_(false), width_(400), height_(300) {
    printf("Brisk Window: Platform not implemented for this OS\n");
}

Window::~Window() {}

void Window::SetVisible(bool visible) {
    visible_ = visible;
    printf("Brisk Window: SetVisible(%s) - not implemented\n", visible ? "true" : "false");
}

void* Window::GetHandle() const {
    return parent_handle_;
}

void Window::SetSize(int width, int height) {
    width_ = width;
    height_ = height;
}

void Window::BeginDraw() {}
void Window::EndDraw() {}

void Window::DrawKnob(int x, int y, int radius, double value, double min_val, double max_val) {
    printf("Brisk Window: DrawKnob - not implemented for this platform\n");
}

void Window::DrawText(int x, int y, const char* text) {
    printf("Brisk Window: DrawText - not implemented for this platform\n");
}

void Window::Render() {
    printf("Brisk Window: Render - not implemented for this platform\n");
}

void Window::InitializePlatform() {}
void Window::CleanupPlatform() {}

#endif

} // namespace brisk