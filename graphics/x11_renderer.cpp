#include "x11_renderer.h"

#ifdef __linux__
#include <iostream>
#include <cstring>

namespace clap_jules {
namespace graphics {

X11Renderer::X11Renderer() 
    : display_(nullptr), window_(0), gc_(nullptr), ximage_(nullptr), 
      width_(0), height_(0), initialized_(false) {
}

X11Renderer::~X11Renderer() {
    cleanup();
}

bool X11Renderer::initialize(unsigned long parent_window, int width, int height) {
    if (initialized_) {
        cleanup();
    }
    
    width_ = width;
    height_ = height;
    
    // Open connection to X server
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
        std::cerr << "X11Renderer: Failed to open X display\n";
        return false;
    }
    
    // Get default screen and create a child window
    int screen = DefaultScreen(display_);
    window_ = XCreateSimpleWindow(display_, parent_window, 0, 0, width, height,
                                  0, BlackPixel(display_, screen), WhitePixel(display_, screen));
    
    if (!window_) {
        std::cerr << "X11Renderer: Failed to create window\n";
        XCloseDisplay(display_);
        display_ = nullptr;
        return false;
    }
    
    // Create graphics context
    gc_ = XCreateGC(display_, window_, 0, nullptr);
    if (!gc_) {
        std::cerr << "X11Renderer: Failed to create graphics context\n";
        XDestroyWindow(display_, window_);
        XCloseDisplay(display_);
        display_ = nullptr;
        return false;
    }
    
    // Map the window to make it visible
    XMapWindow(display_, window_);
    XFlush(display_);
    
    initialized_ = true;
    std::cout << "X11Renderer: Initialized " << width << "x" << height << " window\n";
    return true;
}

void X11Renderer::cleanup() {
    if (!initialized_) return;
    
    if (ximage_) {
        XDestroyImage(ximage_);
        ximage_ = nullptr;
    }
    
    if (gc_) {
        XFreeGC(display_, gc_);
        gc_ = nullptr;
    }
    
    if (window_) {
        XDestroyWindow(display_, window_);
        window_ = 0;
    }
    
    if (display_) {
        XCloseDisplay(display_);
        display_ = nullptr;
    }
    
    initialized_ = false;
}

bool X11Renderer::presentPixelBuffer(const uint32_t* pixels, int width, int height) {
    if (!initialized_ || !pixels) {
        return false;
    }
    
    // Recreate XImage if size changed
    if (!ximage_ || width != width_ || height != height_) {
        if (ximage_) {
            XDestroyImage(ximage_);
        }
        
        width_ = width;
        height_ = height;
        
        // Create XImage for the pixel buffer
        int screen = DefaultScreen(display_);
        int depth = DefaultDepth(display_, screen);
        
        // Allocate memory for the image data (XImage will own this memory)
        char* image_data = new char[width * height * 4];
        
        ximage_ = XCreateImage(display_, DefaultVisual(display_, screen),
                              depth, ZPixmap, 0, image_data,
                              width, height, 32, width * 4);
        
        if (!ximage_) {
            std::cerr << "X11Renderer: Failed to create XImage\n";
            delete[] image_data;
            return false;
        }
        
        // Resize window to match new dimensions
        XResizeWindow(display_, window_, width, height);
    }
    
    // Copy pixel data to XImage
    uint32_t* image_pixels = (uint32_t*)ximage_->data;
    std::memcpy(image_pixels, pixels, width * height * sizeof(uint32_t));
    
    // Present the image to the window
    XPutImage(display_, window_, gc_, ximage_, 0, 0, 0, 0, width, height);
    XFlush(display_);
    
    return true;
}

void X11Renderer::resize(int width, int height) {
    if (!initialized_) return;
    
    width_ = width;
    height_ = height;
    
    XResizeWindow(display_, window_, width, height);
    
    // XImage will be recreated on next present call
    if (ximage_) {
        XDestroyImage(ximage_);
        ximage_ = nullptr;
    }
}

} // namespace graphics
} // namespace clap_jules

#endif // __linux__