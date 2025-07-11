#pragma once

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstdint>

namespace clap_jules {
namespace graphics {

// Simple X11 renderer for displaying pixel buffers
class X11Renderer {
private:
    Display* display_;
    Window window_;
    GC gc_;
    XImage* ximage_;
    int width_, height_;
    bool initialized_;
    
public:
    X11Renderer();
    ~X11Renderer();
    
    bool initialize(unsigned long parent_window, int width, int height);
    void cleanup();
    
    bool presentPixelBuffer(const uint32_t* pixels, int width, int height);
    void resize(int width, int height);
    
    bool isInitialized() const { return initialized_; }
};

} // namespace graphics
} // namespace clap_jules

#endif // __linux__