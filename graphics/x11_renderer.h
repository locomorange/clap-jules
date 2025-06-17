#pragma once

#if defined(__linux__) && defined(HAVE_X11)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <functional>

namespace clap_jules {
namespace graphics {

class X11Renderer {
public:
    X11Renderer();
    ~X11Renderer();
    
    bool initialize(Window parent_window, int width, int height);
    void resize(int width, int height);
    bool presentPixelBuffer(const uint32_t* pixels, int width, int height);
    bool isInitialized() const { return initialized_; }
    
private:
    Display* display_;
    Window parent_window_;
    Window child_window_;
    GC gc_;
    XImage* image_;
    char* image_data_;
    int width_, height_;
    bool initialized_;
    
    bool createChildWindow();
    void destroyChildWindow();
    bool createImage(int width, int height);
    void destroyImage();
};

} // namespace graphics
} // namespace clap_jules

#endif // __linux__ && HAVE_X11