#pragma once

#include <cstdint>

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

// Simple cross-platform GUI window for CLAP plugin
class SimpleGUI {
private:
#ifdef __linux__
    Display* m_display = nullptr;
    Window m_window = 0;
    int m_screen = 0;
#elif defined(_WIN32)
    HWND m_hwnd = nullptr;
    HINSTANCE m_hinstance = nullptr;
#endif
    
    bool m_is_visible = false;
    bool m_is_created = false;

public:
    SimpleGUI();
    ~SimpleGUI();
    
    bool create();
    void destroy();
    bool show();
    bool hide();
    bool set_size(uint32_t width, uint32_t height);
    bool get_size(uint32_t* width, uint32_t* height);
    void set_title(const char* title);
    void draw_content(); // Made public for Windows callback
    
    // Platform-specific methods
#ifdef __linux__
    bool set_parent_x11(unsigned long parent_window);
#elif defined(_WIN32)
    bool set_parent_win32(HWND parent_window);
#endif

    // Made public for Windows callback access
    uint32_t m_width = 400;
    uint32_t m_height = 300;

private:
    void handle_events();
};