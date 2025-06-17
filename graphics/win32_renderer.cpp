#include "win32_renderer.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

#include <cstdio>

namespace clap_jules {
namespace graphics {

static const wchar_t* CHILD_WINDOW_CLASS = L"ClapJulesChild";
static bool class_registered = false;

Win32Renderer::Win32Renderer() 
    : parent_hwnd_(nullptr), child_hwnd_(nullptr), hdc_(nullptr), 
      bitmap_(nullptr), bitmap_data_(nullptr), width_(0), height_(0), 
      initialized_(false) {}

Win32Renderer::~Win32Renderer() {
    destroyBitmap();
    destroyChildWindow();
}

bool Win32Renderer::initialize(HWND parent_hwnd, int width, int height) {
    if (initialized_) {
        return false;
    }
    
    parent_hwnd_ = parent_hwnd;
    width_ = width;
    height_ = height;
    
    if (!createChildWindow()) {
        printf("Win32Renderer: Failed to create child window\n");
        return false;
    }
    
    if (!createBitmap(width, height)) {
        printf("Win32Renderer: Failed to create bitmap\n");
        destroyChildWindow();
        return false;
    }
    
    initialized_ = true;
    printf("Win32Renderer: Initialized successfully (%dx%d)\n", width, height);
    return true;
}

void Win32Renderer::resize(int width, int height) {
    if (!initialized_ || (width == width_ && height == height_)) {
        return;
    }
    
    width_ = width;
    height_ = height;
    
    // Resize child window
    if (child_hwnd_) {
        SetWindowPos(child_hwnd_, nullptr, 0, 0, width, height, 
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
    
    // Recreate bitmap with new size
    destroyBitmap();
    createBitmap(width, height);
    
    printf("Win32Renderer: Resized to %dx%d\n", width, height);
}

bool Win32Renderer::presentPixelBuffer(const uint32_t* pixels, int width, int height) {
    if (!initialized_ || !bitmap_data_ || !pixels) {
        return false;
    }
    
    // Copy pixel data to bitmap
    size_t bytes_to_copy = std::min(width * height * 4, width_ * height_ * 4);
    memcpy(bitmap_data_, pixels, bytes_to_copy);
    
    // Trigger redraw
    InvalidateRect(child_hwnd_, nullptr, FALSE);
    
    return true;
}

void Win32Renderer::invalidate() {
    if (child_hwnd_) {
        InvalidateRect(child_hwnd_, nullptr, FALSE);
    }
}

void Win32Renderer::setRedrawCallback(std::function<void()> callback) {
    redraw_callback_ = callback;
}

LRESULT CALLBACK Win32Renderer::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Win32Renderer* renderer = nullptr;
    
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        renderer = static_cast<Win32Renderer*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(renderer));
    } else {
        renderer = reinterpret_cast<Win32Renderer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    
    if (renderer) {
        switch (uMsg) {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                if (renderer->bitmap_ && renderer->hdc_) {
                    BitBlt(hdc, 0, 0, renderer->width_, renderer->height_,
                           renderer->hdc_, 0, 0, SRCCOPY);
                }
                
                EndPaint(hwnd, &ps);
                return 0;
            }
            
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MOUSEMOVE:
                // Trigger redraw callback for mouse events
                if (renderer->redraw_callback_) {
                    renderer->redraw_callback_();
                }
                return 0;
                
            case WM_SIZE:
                // Handle window resize
                return 0;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool Win32Renderer::createChildWindow() {
    // Register window class if not already done
    if (!class_registered) {
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = CHILD_WINDOW_CLASS;
        
        if (!RegisterClassEx(&wc)) {
            printf("Win32Renderer: Failed to register window class\n");
            return false;
        }
        class_registered = true;
    }
    
    // Create child window
    child_hwnd_ = CreateWindowEx(
        0,
        CHILD_WINDOW_CLASS,
        L"CLAP Plugin GUI",
        WS_CHILD | WS_VISIBLE,
        0, 0, width_, height_,
        parent_hwnd_,
        nullptr,
        GetModuleHandle(nullptr),
        this
    );
    
    if (!child_hwnd_) {
        printf("Win32Renderer: Failed to create child window (error: %lu)\n", GetLastError());
        return false;
    }
    
    ShowWindow(child_hwnd_, SW_SHOW);
    UpdateWindow(child_hwnd_);
    
    printf("Win32Renderer: Child window created successfully\n");
    return true;
}

void Win32Renderer::destroyChildWindow() {
    if (child_hwnd_) {
        DestroyWindow(child_hwnd_);
        child_hwnd_ = nullptr;
    }
}

bool Win32Renderer::createBitmap(int width, int height) {
    if (!child_hwnd_) {
        return false;
    }
    
    HDC window_hdc = GetDC(child_hwnd_);
    if (!window_hdc) {
        return false;
    }
    
    hdc_ = CreateCompatibleDC(window_hdc);
    if (!hdc_) {
        ReleaseDC(child_hwnd_, window_hdc);
        return false;
    }
    
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down bitmap
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    bitmap_ = CreateDIBSection(hdc_, &bmi, DIB_RGB_COLORS, &bitmap_data_, nullptr, 0);
    if (!bitmap_) {
        DeleteDC(hdc_);
        hdc_ = nullptr;
        ReleaseDC(child_hwnd_, window_hdc);
        return false;
    }
    
    SelectObject(hdc_, bitmap_);
    ReleaseDC(child_hwnd_, window_hdc);
    
    printf("Win32Renderer: Bitmap created (%dx%d)\n", width, height);
    return true;
}

void Win32Renderer::destroyBitmap() {
    if (bitmap_) {
        DeleteObject(bitmap_);
        bitmap_ = nullptr;
        bitmap_data_ = nullptr;
    }
    
    if (hdc_) {
        DeleteDC(hdc_);
        hdc_ = nullptr;
    }
}

} // namespace graphics
} // namespace clap_jules

#endif // WIN32