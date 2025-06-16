#pragma once

#ifdef _WIN32
#include <windows.h>
#include <cstdint>

namespace clap_jules {
namespace graphics {

// Simple Win32 renderer for displaying pixel buffers
class Win32Renderer {
private:
    HWND hwnd_;
    HDC hdc_;
    HDC mem_dc_;
    HBITMAP bitmap_;
    BITMAPINFO bitmap_info_;
    void* pixel_buffer_;
    int width_, height_;
    bool initialized_;
    
public:
    Win32Renderer();
    ~Win32Renderer();
    
    bool initialize(HWND parent_window, int width, int height);
    void cleanup();
    
    bool presentPixelBuffer(const uint32_t* pixels, int width, int height);
    void resize(int width, int height);
    
    bool isInitialized() const { return initialized_; }
};

} // namespace graphics
} // namespace clap_jules

#endif // _WIN32