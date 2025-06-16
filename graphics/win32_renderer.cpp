#include "win32_renderer.h"

#ifdef _WIN32
#include <iostream>
#include <cstring>

namespace clap_jules {
namespace graphics {

Win32Renderer::Win32Renderer() 
    : hwnd_(nullptr), hdc_(nullptr), mem_dc_(nullptr), bitmap_(nullptr),
      pixel_buffer_(nullptr), width_(0), height_(0), initialized_(false) {
    memset(&bitmap_info_, 0, sizeof(bitmap_info_));
}

Win32Renderer::~Win32Renderer() {
    cleanup();
}

bool Win32Renderer::initialize(HWND parent_window, int width, int height) {
    if (initialized_) {
        cleanup();
    }
    
    width_ = width;
    height_ = height;
    
    // Create a child window within the parent
    hwnd_ = CreateWindowEx(
        0,                          // Extended window style
        L"STATIC",                  // Window class name
        L"CLAP Plugin Window",      // Window title
        WS_CHILD | WS_VISIBLE,      // Window style
        0, 0, width, height,        // Position and size
        parent_window,              // Parent window
        nullptr,                    // Menu
        GetModuleHandle(nullptr),   // Instance handle
        nullptr                     // Additional application data
    );
    
    if (!hwnd_) {
        std::cerr << "Win32Renderer: Failed to create child window\n";
        return false;
    }
    
    // Get device context for the window
    hdc_ = GetDC(hwnd_);
    if (!hdc_) {
        std::cerr << "Win32Renderer: Failed to get device context\n";
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
        return false;
    }
    
    // Create memory device context
    mem_dc_ = CreateCompatibleDC(hdc_);
    if (!mem_dc_) {
        std::cerr << "Win32Renderer: Failed to create memory device context\n";
        ReleaseDC(hwnd_, hdc_);
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
        hdc_ = nullptr;
        return false;
    }
    
    // Set up bitmap info structure
    bitmap_info_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info_.bmiHeader.biWidth = width;
    bitmap_info_.bmiHeader.biHeight = -height; // Negative for top-down bitmap
    bitmap_info_.bmiHeader.biPlanes = 1;
    bitmap_info_.bmiHeader.biBitCount = 32;
    bitmap_info_.bmiHeader.biCompression = BI_RGB;
    bitmap_info_.bmiHeader.biSizeImage = 0;
    bitmap_info_.bmiHeader.biXPelsPerMeter = 0;
    bitmap_info_.bmiHeader.biYPelsPerMeter = 0;
    bitmap_info_.bmiHeader.biClrUsed = 0;
    bitmap_info_.bmiHeader.biClrImportant = 0;
    
    // Create DIB section
    bitmap_ = CreateDIBSection(mem_dc_, &bitmap_info_, DIB_RGB_COLORS, &pixel_buffer_, nullptr, 0);
    if (!bitmap_) {
        std::cerr << "Win32Renderer: Failed to create DIB section\n";
        DeleteDC(mem_dc_);
        ReleaseDC(hwnd_, hdc_);
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
        hdc_ = nullptr;
        mem_dc_ = nullptr;
        return false;
    }
    
    // Select bitmap into memory DC
    SelectObject(mem_dc_, bitmap_);
    
    initialized_ = true;
    std::cout << "Win32Renderer: Successfully initialized " << width << "x" << height << " window\n";
    return true;
}

void Win32Renderer::cleanup() {
    if (bitmap_) {
        DeleteObject(bitmap_);
        bitmap_ = nullptr;
    }
    
    if (mem_dc_) {
        DeleteDC(mem_dc_);
        mem_dc_ = nullptr;
    }
    
    if (hdc_ && hwnd_) {
        ReleaseDC(hwnd_, hdc_);
        hdc_ = nullptr;
    }
    
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    
    pixel_buffer_ = nullptr;
    initialized_ = false;
}

bool Win32Renderer::presentPixelBuffer(const uint32_t* pixels, int width, int height) {
    if (!initialized_ || !pixels) {
        return false;
    }
    
    // If size changed, reinitialize
    if (width != width_ || height != height_) {
        if (!initialize(GetParent(hwnd_), width, height)) {
            return false;
        }
    }
    
    // Copy pixel data to DIB section
    // Convert from RGBA to BGRA (Win32 expects BGRA)
    uint32_t* dest = static_cast<uint32_t*>(pixel_buffer_);
    for (int i = 0; i < width * height; ++i) {
        uint32_t rgba = pixels[i];
        uint32_t r = (rgba >> 16) & 0xFF;
        uint32_t g = (rgba >> 8) & 0xFF;
        uint32_t b = rgba & 0xFF;
        uint32_t a = (rgba >> 24) & 0xFF;
        dest[i] = (a << 24) | (r << 16) | (g << 8) | b; // ARGB format for Win32
    }
    
    // Blit from memory DC to window DC
    if (!BitBlt(hdc_, 0, 0, width_, height_, mem_dc_, 0, 0, SRCCOPY)) {
        std::cerr << "Win32Renderer: BitBlt failed\n";
        return false;
    }
    
    return true;
}

void Win32Renderer::resize(int width, int height) {
    if (width == width_ && height == height_) {
        return;
    }
    
    // Reinitialize with new size
    if (initialized_) {
        HWND parent = GetParent(hwnd_);
        cleanup();
        initialize(parent, width, height);
    }
}

} // namespace graphics
} // namespace clap_jules

#endif // _WIN32