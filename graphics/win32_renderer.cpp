#include "win32_renderer.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <iostream>
#include <cstring>
#include <memory>
#include <map>

namespace clap_jules {
namespace graphics {

// Static map to associate window handles with renderer instances
static std::map<HWND, Win32Renderer*> window_renderer_map;

Win32Renderer::Win32Renderer() 
    : hwnd_(nullptr), hdc_(nullptr), mem_dc_(nullptr), bitmap_(nullptr),
      pixel_buffer_(nullptr), width_(0), height_(0), initialized_(false) {
    std::memset(&bitmap_info_, 0, sizeof(bitmap_info_));
}

Win32Renderer::~Win32Renderer() {
    cleanup();
}

LRESULT CALLBACK Win32Renderer::WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    // Find the renderer associated with this window
    auto it = window_renderer_map.find(hwnd);
    Win32Renderer* renderer = (it != window_renderer_map.end()) ? it->second : nullptr;
    
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Trigger redraw if we have a callback
            if (renderer && renderer->redraw_callback_) {
                renderer->redraw_callback_();
            }
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_TIMER:
            if (wparam == TIMER_ID) {
                // Trigger regular redraws for animation
                if (renderer && renderer->redraw_callback_) {
                    renderer->redraw_callback_();
                }
                // Force window to repaint
                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;
            }
            break;
            
        case WM_SIZE: {
            RECT rect;
            if (GetClientRect(hwnd, &rect)) {
                int new_width = rect.right - rect.left;
                int new_height = rect.bottom - rect.top;
                if (renderer && (new_width != renderer->width_ || new_height != renderer->height_)) {
                    // Trigger redraw after resize
                    if (renderer->redraw_callback_) {
                        renderer->redraw_callback_();
                    }
                }
            }
            break;
        }
        
        case WM_ERASEBKGND:
            // Prevent background erase to avoid flicker
            return 1;
            
        case WM_DESTROY:
            // Remove from map when window is destroyed
            if (renderer) {
                window_renderer_map.erase(hwnd);
            }
            break;
    }
    
    return DefWindowProc(hwnd, msg, wparam, lparam);
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
    
    // Register a simple window class for our child window
    static bool class_registered = false;
    if (!class_registered) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WindowProcedure;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = L"CLAP_PLUGIN_CHILD";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr; // No background brush to avoid flicker
        wc.style = CS_OWNDC; // Own device context
        
        if (RegisterClassExW(&wc)) {
            class_registered = true;
        } else {
            std::cerr << "Win32Renderer: Failed to register window class\n";
        }
    }
    
    // Create a child window within the parent
    hwnd_ = CreateWindowExW(
        0,                          // Extended window style
        L"CLAP_PLUGIN_CHILD",       // Window class name
        L"CLAP Plugin Window",      // Window title
        WS_CHILD | WS_VISIBLE,      // Window style - visible by default
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
    
    // Associate this renderer with the window
    window_renderer_map[hwnd_] = this;
    
    // Get device context for the window
    hdc_ = GetDC(hwnd_);
    if (!hdc_) {
        DWORD error = GetLastError();
        std::cerr << "Win32Renderer: Failed to get device context (error: " << error << ")\n";
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
        window_renderer_map.erase(hwnd_);
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
        window_renderer_map.erase(hwnd_);
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
        window_renderer_map.erase(hwnd_);
        return false;
    }
    
    // Select bitmap into memory DC
    SelectObject(mem_dc_, bitmap_);
    
    // Initialize pixel buffer with a test pattern to ensure it's not white
    if (pixel_buffer_) {
        uint32_t* pixels = static_cast<uint32_t*>(pixel_buffer_);
        uint32_t dark_gray = 0xFF404040; // Dark gray in ARGB format
        for (int i = 0; i < width * height; ++i) {
            pixels[i] = dark_gray;
        }
    }
    
    // Set up timer for regular updates (60 FPS)
    SetTimer(hwnd_, TIMER_ID, 16, nullptr); // ~60 FPS
    
    initialized_ = true;
    std::cout << "Win32Renderer: Successfully initialized " << width << "x" << height << " window\n";
    
    // Force an initial paint
    InvalidateRect(hwnd_, nullptr, FALSE);
    UpdateWindow(hwnd_);
    
    return true;
}

void Win32Renderer::cleanup() {
    // Stop timer
    if (hwnd_) {
        KillTimer(hwnd_, TIMER_ID);
    }
    
    // Remove from window map
    if (hwnd_) {
        window_renderer_map.erase(hwnd_);
    }
    
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
    
    // Force window to update
    InvalidateRect(hwnd_, nullptr, FALSE);
    
    return true;
}

void Win32Renderer::invalidate() {
    if (hwnd_) {
        InvalidateRect(hwnd_, nullptr, FALSE);
        UpdateWindow(hwnd_);
    }
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