#pragma once

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

#include <windows.h>
#include <functional>
#include <memory>

namespace clap_jules {
namespace graphics {

class Win32Renderer {
public:
    Win32Renderer();
    ~Win32Renderer();
    
    bool initialize(HWND parent_hwnd, int width, int height);
    void resize(int width, int height);
    bool presentPixelBuffer(const uint32_t* pixels, int width, int height);
    void invalidate();
    bool isInitialized() const { return initialized_; }
    
    void setRedrawCallback(std::function<void()> callback);
    
private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    HWND parent_hwnd_;
    HWND child_hwnd_;
    HDC hdc_;
    HBITMAP bitmap_;
    void* bitmap_data_;
    int width_, height_;
    bool initialized_;
    std::function<void()> redraw_callback_;
    
    bool createChildWindow();
    void destroyChildWindow();
    bool createBitmap(int width, int height);
    void destroyBitmap();
};

} // namespace graphics
} // namespace clap_jules

#endif // WIN32