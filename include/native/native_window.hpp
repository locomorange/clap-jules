#pragma once

#include <string>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <Cocoa/Cocoa.h>
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
    NSWindow* window_;
    NSView* view_;
#else
    Display* display_;
    Window window_;
    GC gc_;
    int screen_;
#endif
};

}