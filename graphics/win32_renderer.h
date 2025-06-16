#pragma once

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

// Prevent Windows.h from defining min/max macros
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Include standard headers before Windows headers to avoid conflicts
#include <cstdint>
#include <memory>
#include <functional>

// Now include Windows headers
#include <windows.h>

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
    std::function<void()> redraw_callback_;
    
    // Timer for animations
    static const UINT_PTR TIMER_ID = 1;
    
    static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    
public:
    Win32Renderer();
    ~Win32Renderer();
    
    bool initialize(HWND parent_window, int width, int height);
    void cleanup();
    
    bool presentPixelBuffer(const uint32_t* pixels, int width, int height);
    void resize(int width, int height);
    
    bool isInitialized() const { return initialized_; }
    
    // Set callback for when window needs redraw
    void setRedrawCallback(std::function<void()> callback) { redraw_callback_ = callback; }
    
    // Force window to repaint
    void invalidate();
    
    // Get the child window handle
    HWND getChildWindow() const { return hwnd_; }

} // namespace graphics
} // namespace clap_jules

#endif // _WIN32 || __WIN32__ || WIN32