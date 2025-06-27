#include "native/native_window.hpp"
#include <iostream>

namespace native {

NativeWindow::NativeWindow(int width, int height, const std::string& title)
    : width_(width), height_(height), title_(title), visible_(false) {
#ifdef _WIN32
    hwnd_ = nullptr;
    hdc_ = nullptr;
#elif __APPLE__
    window_ = nullptr;
    view_ = nullptr;
#else
    display_ = nullptr;
    window_ = 0;
    gc_ = nullptr;
    screen_ = 0;
#endif
    createWindow();
}

NativeWindow::~NativeWindow() {
    destroyWindow();
}

void NativeWindow::createWindow() {
#ifdef _WIN32
    // Windows implementation
    WNDCLASS wc = {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"CLAPPluginWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    
    RegisterClass(&wc);
    
    hwnd_ = CreateWindowEx(
        0,
        L"CLAPPluginWindow",
        L"CLAP Plugin",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width_, height_,
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );
    
    if (hwnd_) {
        hdc_ = GetDC(hwnd_);
    }
    
#elif __APPLE__
    // macOS implementation - simplified for now
    std::cout << "macOS native window creation not implemented yet" << std::endl;
    
#else
    // Linux X11 implementation
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
        std::cerr << "Cannot open X11 display" << std::endl;
        return;
    }
    
    screen_ = DefaultScreen(display_);
    Window root = RootWindow(display_, screen_);
    
    window_ = XCreateSimpleWindow(
        display_, root,
        100, 100,           // x, y
        width_, height_,    // width, height
        1,                  // border width
        BlackPixel(display_, screen_),  // border color
        WhitePixel(display_, screen_)   // background color
    );
    
    XSelectInput(display_, window_, ExposureMask | KeyPressMask);
    XStoreName(display_, window_, title_.c_str());
    
    gc_ = XCreateGC(display_, window_, 0, nullptr);
    XSetForeground(display_, gc_, BlackPixel(display_, screen_));
#endif
}

void NativeWindow::destroyWindow() {
#ifdef _WIN32
    if (hdc_) {
        ReleaseDC(hwnd_, hdc_);
        hdc_ = nullptr;
    }
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
#elif __APPLE__
    // macOS cleanup
#else
    if (gc_) {
        XFreeGC(display_, gc_);
        gc_ = nullptr;
    }
    if (window_) {
        XDestroyWindow(display_, window_);
        window_ = 0;
    }
    if (display_) {
        XCloseDisplay(display_);
        display_ = nullptr;
    }
#endif
}

void NativeWindow::show() {
#ifdef _WIN32
    if (hwnd_) {
        ShowWindow(hwnd_, SW_SHOW);
        UpdateWindow(hwnd_);
        visible_ = true;
    }
#elif __APPLE__
    // macOS show
    visible_ = true;
#else
    if (display_ && window_) {
        XMapWindow(display_, window_);
        XFlush(display_);
        visible_ = true;
    }
#endif
}

void NativeWindow::hide() {
#ifdef _WIN32
    if (hwnd_) {
        ShowWindow(hwnd_, SW_HIDE);
        visible_ = false;
    }
#elif __APPLE__
    // macOS hide
    visible_ = false;
#else
    if (display_ && window_) {
        XUnmapWindow(display_, window_);
        XFlush(display_);
        visible_ = false;
    }
#endif
}

bool NativeWindow::isVisible() const {
    return visible_;
}

void NativeWindow::setSize(int width, int height) {
    width_ = width;
    height_ = height;
    
#ifdef _WIN32
    if (hwnd_) {
        SetWindowPos(hwnd_, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
    }
#elif __APPLE__
    // macOS resize
#else
    if (display_ && window_) {
        XResizeWindow(display_, window_, width, height);
        XFlush(display_);
    }
#endif
}

void NativeWindow::setPosition(int x, int y) {
#ifdef _WIN32
    if (hwnd_) {
        SetWindowPos(hwnd_, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
#elif __APPLE__
    // macOS move
#else
    if (display_ && window_) {
        XMoveWindow(display_, window_, x, y);
        XFlush(display_);
    }
#endif
}

void* NativeWindow::getNativeHandle() const {
#ifdef _WIN32
    return hwnd_;
#elif __APPLE__
    return window_;
#else
    return reinterpret_cast<void*>(window_);
#endif
}

void NativeWindow::setParent(void* parent) {
#ifdef _WIN32
    if (hwnd_ && parent) {
        SetParent(hwnd_, static_cast<HWND>(parent));
    }
#elif __APPLE__
    // macOS parenting
#else
    if (display_ && window_ && parent) {
        Window parentWindow = reinterpret_cast<Window>(parent);
        XReparentWindow(display_, window_, parentWindow, 0, 0);
        XFlush(display_);
    }
#endif
}

void NativeWindow::drawText(const std::string& text, int x, int y) {
#ifdef _WIN32
    if (hdc_) {
        TextOutA(hdc_, x, y, text.c_str(), text.length());
    }
#elif __APPLE__
    // macOS text drawing
#else
    if (display_ && window_ && gc_) {
        XDrawString(display_, window_, gc_, x, y, text.c_str(), text.length());
    }
#endif
}

void NativeWindow::drawRect(int x, int y, int width, int height) {
#ifdef _WIN32
    if (hdc_) {
        Rectangle(hdc_, x, y, x + width, y + height);
    }
#elif __APPLE__
    // macOS rectangle drawing
#else
    if (display_ && window_ && gc_) {
        XDrawRectangle(display_, window_, gc_, x, y, width, height);
    }
#endif
}

void NativeWindow::clear() {
#ifdef _WIN32
    if (hdc_) {
        RECT rect;
        GetClientRect(hwnd_, &rect);
        FillRect(hdc_, &rect, (HBRUSH)(COLOR_WINDOW + 1));
    }
#elif __APPLE__
    // macOS clear
#else
    if (display_ && window_) {
        XClearWindow(display_, window_);
    }
#endif
}

void NativeWindow::present() {
#ifdef _WIN32
    // Windows automatically presents
#elif __APPLE__
    // macOS present
#else
    if (display_) {
        XFlush(display_);
    }
#endif
}

}