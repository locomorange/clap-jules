#pragma once

#include <string>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
// Forward declare as opaque pointers to avoid importing Objective-C headers in C++
// These will be cast to proper types in the .mm implementation file
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

namespace native {

class NativeWindow {
public:
    NativeWindow(int width, int height, const std::string& title);
    ~NativeWindow();
    
    void show();
    void hide();
    bool isVisible() const;
    void setSize(int width, int height);
    void setPosition(int x, int y);
    void* getNativeHandle() const;
    void setParent(void* parent);
    
    // Simple drawing methods
    void drawText(const std::string& text, int x, int y);
    void drawRect(int x, int y, int width, int height);
    void clear();
    void present();
    
private:
    void createWindow();
    void destroyWindow();
    
    int width_, height_;
    std::string title_;
    bool visible_;
    
#ifdef _WIN32
    HWND hwnd_;
    HDC hdc_;
#elif __APPLE__
    void* window_;  // NSWindow* - opaque pointer to avoid Objective-C in C++ header
    void* view_;    // NSView* - opaque pointer to avoid Objective-C in C++ header
#else
    Display* display_;
    Window window_;
    GC gc_;
    int screen_;
#endif
};

}