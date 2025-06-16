#include "win32_renderer.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <iostream>
#include <cstring>
#include <memory>

namespace clap_jules {
namespace graphics {

Win32Renderer::Win32Renderer() 
    : hwnd_(nullptr), hdc_(nullptr), mem_dc_(nullptr), bitmap_(nullptr),
      pixel_buffer_(nullptr), width_(0), height_(0), initialized_(false) {
    std::memset(&bitmap_info_, 0, sizeof(bitmap_info_));
}

Win32Renderer::~Win32Renderer() {
    cleanup();
}

bool Win32Renderer::initialize(HWND parent_window, int width, int height) {
    if (initialized_) {
        cleanup();
    }
    
    if (!parent_window || width <= 0 || height <= 0) {
        std::cerr << "Win32Renderer: Invalid parameters for initialization\n";
        return false;
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
        DWORD error = GetLastError();
        std::cerr << "Win32Renderer: Failed to create child window (error: " << error << ")\n";
        return false;
    }
    
    // Get device context for the window
    hdc_ = GetDC(hwnd_);
    if (!hdc_) {
        DWORD error = GetLastError();
        std::cerr << "Win32Renderer: Failed to get device context (error: " << error << ")\n";
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
        return false;
    }
    
    // Create memory device context
    mem_dc_ = CreateCompatibleDC(hdc_);
    if (!mem_dc_) {
        DWORD error = GetLastError();
        std::cerr << "Win32Renderer: Failed to create memory device context (error: " << error << ")\n";
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
        DWORD error = GetLastError();
        std::cerr << "Win32Renderer: Failed to create DIB section (error: " << error << ")\n";
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
    
    if (width <= 0 || height <= 0) {
        std::cerr << "Win32Renderer: Invalid dimensions: " << width << "x" << height << "\n";
        return false;
    }
    
    // If size changed, reinitialize
    if (width != width_ || height != height_) {
        HWND parent = hwnd_ ? GetParent(hwnd_) : nullptr;
        if (!parent) {
            std::cerr << "Win32Renderer: No parent window available for resize\n";
            return false;
        }
        if (!initialize(parent, width, height)) {
            return false;
        }
    }
    
    // Copy pixel data to DIB section
    // Convert from RGBA to BGRA (Win32 expects BGRA)
    uint32_t* dest = static_cast<uint32_t*>(pixel_buffer_);
    if (!dest) {
        std::cerr << "Win32Renderer: Invalid pixel buffer pointer\n";
        return false;
    }
    
    for (int i = 0; i < width * height; ++i) {
        uint32_t rgba = pixels[i];
        uint32_t r = (rgba >> 0) & 0xFF;
        uint32_t g = (rgba >> 8) & 0xFF;
        uint32_t b = (rgba >> 16) & 0xFF;
        uint32_t a = (rgba >> 24) & 0xFF;
        dest[i] = (a << 24) | (r << 16) | (g << 8) | b; // ARGB format for Win32
    }
    
    // Blit from memory DC to window DC
    if (!BitBlt(hdc_, 0, 0, width_, height_, mem_dc_, 0, 0, SRCCOPY)) {
        DWORD error = GetLastError();
        std::cerr << "Win32Renderer: BitBlt failed (error: " << error << ")\n";
        return false;
    }
    
    return true;
}

void Win32Renderer::resize(int width, int height) {
    if (width <= 0 || height <= 0) {
        std::cerr << "Win32Renderer: Invalid resize dimensions: " << width << "x" << height << "\n";
        return;
    }
    
    if (width == width_ && height == height_) {
        return;
    }
    
    // Reinitialize with new size
    if (initialized_) {
        HWND parent = hwnd_ ? GetParent(hwnd_) : nullptr;
        cleanup();
        if (parent) {
            initialize(parent, width, height);
        }
    }
}

} // namespace graphics
} // namespace clap_jules

#endif // _WIN32 || __WIN32__ || WIN32