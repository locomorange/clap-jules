#include "x11_renderer.h"

#if defined(__linux__) && defined(HAVE_X11)

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <algorithm>

namespace clap_jules {
namespace graphics {

X11Renderer::X11Renderer() 
    : display_(nullptr), parent_window_(0), child_window_(0), gc_(nullptr), 
      image_(nullptr), image_data_(nullptr), width_(0), height_(0), 
      initialized_(false) {}

X11Renderer::~X11Renderer() {
    destroyImage();
    destroyChildWindow();
}

bool X11Renderer::initialize(Window parent_window, int width, int height) {
    if (initialized_) {
        return false;
    }
    
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
        printf("X11Renderer: Failed to open display\n");
        return false;
    }
    
    parent_window_ = parent_window;
    width_ = width;
    height_ = height;
    
    if (!createChildWindow()) {
        printf("X11Renderer: Failed to create child window\n");
        XCloseDisplay(display_);
        return false;
    }
    
    if (!createImage(width, height)) {
        printf("X11Renderer: Failed to create image\n");
        destroyChildWindow();
        XCloseDisplay(display_);
        return false;
    }
    
    initialized_ = true;
    printf("X11Renderer: Initialized successfully (%dx%d)\n", width, height);
    return true;
}

void X11Renderer::resize(int width, int height) {
    if (!initialized_ || (width == width_ && height == height_)) {
        return;
    }
    
    width_ = width;
    height_ = height;
    
    // Resize child window
    if (child_window_) {
        XResizeWindow(display_, child_window_, width, height);
    }
    
    // Recreate image with new size
    destroyImage();
    createImage(width, height);
    
    printf("X11Renderer: Resized to %dx%d\n", width, height);
}

bool X11Renderer::presentPixelBuffer(const uint32_t* pixels, int width, int height) {
    if (!initialized_ || !image_ || !pixels) {
        return false;
    }
    
    // Copy pixel data to image
    size_t bytes_to_copy = std::min(width * height * 4, width_ * height_ * 4);
    memcpy(image_data_, pixels, bytes_to_copy);
    
    // Draw image to window
    XPutImage(display_, child_window_, gc_, image_, 0, 0, 0, 0, width_, height_);
    XFlush(display_);
    
    return true;
}

bool X11Renderer::createChildWindow() {
    int screen = DefaultScreen(display_);
    
    child_window_ = XCreateSimpleWindow(
        display_, parent_window_,
        0, 0, width_, height_, 0,
        BlackPixel(display_, screen),
        WhitePixel(display_, screen)
    );
    
    if (!child_window_) {
        printf("X11Renderer: Failed to create child window\n");
        return false;
    }
    
    gc_ = XCreateGC(display_, child_window_, 0, nullptr);
    if (!gc_) {
        printf("X11Renderer: Failed to create graphics context\n");
        XDestroyWindow(display_, child_window_);
        child_window_ = 0;
        return false;
    }
    
    XMapWindow(display_, child_window_);
    XFlush(display_);
    
    printf("X11Renderer: Child window created successfully\n");
    return true;
}

void X11Renderer::destroyChildWindow() {
    if (gc_) {
        XFreeGC(display_, gc_);
        gc_ = nullptr;
    }
    
    if (child_window_) {
        XDestroyWindow(display_, child_window_);
        child_window_ = 0;
    }
    
    if (display_) {
        XCloseDisplay(display_);
        display_ = nullptr;
    }
}

bool X11Renderer::createImage(int width, int height) {
    if (!display_ || !child_window_) {
        return false;
    }
    
    int screen = DefaultScreen(display_);
    Visual* visual = DefaultVisual(display_, screen);
    int depth = DefaultDepth(display_, screen);
    
    size_t data_size = width * height * 4;
    image_data_ = (char*)malloc(data_size);
    if (!image_data_) {
        printf("X11Renderer: Failed to allocate image data\n");
        return false;
    }
    
    image_ = XCreateImage(
        display_, visual, depth, ZPixmap, 0, image_data_,
        width, height, 32, 0
    );
    
    if (!image_) {
        printf("X11Renderer: Failed to create XImage\n");
        free(image_data_);
        image_data_ = nullptr;
        return false;
    }
    
    printf("X11Renderer: Image created (%dx%d)\n", width, height);
    return true;
}

void X11Renderer::destroyImage() {
    if (image_) {
        XDestroyImage(image_); // This also frees image_data_
        image_ = nullptr;
        image_data_ = nullptr;
    }
}

} // namespace graphics
} // namespace clap_jules

#endif // __linux__ && HAVE_X11