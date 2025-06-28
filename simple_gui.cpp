#include "simple_gui.hpp"
#include <stdio.h>
#include <string.h>

#ifdef __linux__
#include <unistd.h>
#include <sys/select.h>
#elif defined(_WIN32)
#include <tchar.h>
#endif

SimpleGUI::SimpleGUI() {
    printf("SimpleGUI: Constructor\n");
}

SimpleGUI::~SimpleGUI() {
    printf("SimpleGUI: Destructor\n");
    if (m_is_created) {
        destroy();
    }
}

bool SimpleGUI::create() {
    printf("SimpleGUI: Creating window\n");
    
    if (m_is_created) {
        printf("SimpleGUI: Already created\n");
        return true;
    }
    
#ifdef __linux__
    // Open connection to X server
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        printf("SimpleGUI: Error - Cannot open X display\n");
        return false;
    }
    
    m_screen = DefaultScreen(m_display);
    
    // Create window
    m_window = XCreateSimpleWindow(
        m_display,
        RootWindow(m_display, m_screen),
        100, 100,  // x, y position
        m_width, m_height,
        1,  // border width
        BlackPixel(m_display, m_screen),  // border
        WhitePixel(m_display, m_screen)   // background
    );
    
    if (!m_window) {
        printf("SimpleGUI: Error - Cannot create X window\n");
        XCloseDisplay(m_display);
        m_display = nullptr;
        return false;
    }
    
    // Set window properties
    XStoreName(m_display, m_window, "CLAP Plugin GUI");
    
    // Select input events
    XSelectInput(m_display, m_window, 
                 ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);
    
    // Set up window manager protocols
    Atom wm_protocols = XInternAtom(m_display, "WM_PROTOCOLS", False);
    Atom wm_delete_window = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(m_display, m_window, &wm_delete_window, 1);
    
    printf("SimpleGUI: X11 window created successfully\n");
    m_is_created = true;
    return true;
    
#elif defined(_WIN32)
    // Windows implementation
    m_hinstance = GetModuleHandle(nullptr);
    
    // Register window class
    static bool class_registered = false;
    if (!class_registered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = DefWindowProc;
        wc.hInstance = m_hinstance;
        wc.lpszClassName = _T("ClapPluginGUI");
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        
        if (!RegisterClass(&wc)) {
            printf("SimpleGUI: Error - Failed to register window class\n");
            return false;
        }
        class_registered = true;
    }
    
    // Create window
    m_hwnd = CreateWindow(
        _T("ClapPluginGUI"),
        _T("CLAP Plugin GUI"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        m_width, m_height,
        nullptr, nullptr,
        m_hinstance, nullptr
    );
    
    if (!m_hwnd) {
        printf("SimpleGUI: Error - Cannot create Windows window\n");
        return false;
    }
    
    printf("SimpleGUI: Windows window created successfully\n");
    m_is_created = true;
    return true;
    
#else
    printf("SimpleGUI: Platform not supported yet\n");
    return false;
#endif
}

void SimpleGUI::destroy() {
    printf("SimpleGUI: Destroying window\n");
    
    if (!m_is_created) {
        return;
    }
    
#ifdef __linux__
    if (m_is_visible) {
        hide();
    }
    
    if (m_window) {
        XDestroyWindow(m_display, m_window);
        m_window = 0;
    }
    
    if (m_display) {
        XCloseDisplay(m_display);
        m_display = nullptr;
    }
#elif defined(_WIN32)
    if (m_is_visible) {
        hide();
    }
    
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
#endif
    
    m_is_created = false;
    printf("SimpleGUI: Window destroyed\n");
}

bool SimpleGUI::show() {
    printf("SimpleGUI: Showing window\n");
    
    if (!m_is_created) {
        printf("SimpleGUI: Window not created\n");
        return false;
    }
    
#ifdef __linux__
    XMapWindow(m_display, m_window);
    XFlush(m_display);
    m_is_visible = true;
    
    // Draw initial content
    draw_content();
    
    printf("SimpleGUI: Window shown\n");
    return true;
#elif defined(_WIN32)
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    m_is_visible = true;
    
    printf("SimpleGUI: Windows window shown\n");
    return true;
#else
    return false;
#endif
}

bool SimpleGUI::hide() {
    printf("SimpleGUI: Hiding window\n");
    
#ifdef __linux__
    if (m_window && m_is_visible) {
        XUnmapWindow(m_display, m_window);
        XFlush(m_display);
        m_is_visible = false;
    }
    return true;
#elif defined(_WIN32)
    if (m_hwnd && m_is_visible) {
        ShowWindow(m_hwnd, SW_HIDE);
        m_is_visible = false;
    }
    return true;
#else
    return false;
#endif
}

bool SimpleGUI::set_size(uint32_t width, uint32_t height) {
    printf("SimpleGUI: Setting size to %ux%u\n", width, height);
    
    m_width = width;
    m_height = height;
    
#ifdef __linux__
    if (m_window && m_is_created) {
        XResizeWindow(m_display, m_window, width, height);
        XFlush(m_display);
    }
#endif
    
    return true;
}

bool SimpleGUI::get_size(uint32_t* width, uint32_t* height) {
    if (width) *width = m_width;
    if (height) *height = m_height;
    return true;
}

void SimpleGUI::set_title(const char* title) {
    printf("SimpleGUI: Setting title to '%s'\n", title ? title : "null");
    
#ifdef __linux__
    if (m_window && title) {
        XStoreName(m_display, m_window, title);
        XFlush(m_display);
    }
#endif
}

#ifdef __linux__
bool SimpleGUI::set_parent_x11(unsigned long parent_window) {
    printf("SimpleGUI: Setting X11 parent window to 0x%lx\n", parent_window);
    
    if (!m_is_created) {
        printf("SimpleGUI: Window not created\n");
        return false;
    }
    
    // Reparent window to host window
    XReparentWindow(m_display, m_window, parent_window, 0, 0);
    XFlush(m_display);
    
    return true;
}
#endif

void SimpleGUI::draw_content() {
#ifdef __linux__
    if (!m_window || !m_display) return;
    
    // Get graphics context
    GC gc = DefaultGC(m_display, m_screen);
    
    // Clear window
    XClearWindow(m_display, m_window);
    
    // Draw simple content
    XDrawString(m_display, m_window, gc, 50, 50, "CLAP Plugin GUI", 15);
    XDrawString(m_display, m_window, gc, 50, 80, "Hello from Plugin!", 18);
    
    // Draw a simple rectangle
    XDrawRectangle(m_display, m_window, gc, 50, 100, 200, 100);
    XDrawString(m_display, m_window, gc, 70, 130, "Volume Control", 14);
    
    // Draw a simple "slider"
    XFillRectangle(m_display, m_window, gc, 70, 150, 100, 10);
    XFillRectangle(m_display, m_window, gc, 120, 145, 10, 20); // slider thumb
    
    XFlush(m_display);
#endif
}

void SimpleGUI::handle_events() {
#ifdef __linux__
    if (!m_display) return;
    
    XEvent event;
    while (XPending(m_display)) {
        XNextEvent(m_display, &event);
        
        switch (event.type) {
            case Expose:
                if (event.xexpose.count == 0) {
                    draw_content();
                }
                break;
                
            case ConfigureNotify:
                m_width = event.xconfigure.width;
                m_height = event.xconfigure.height;
                printf("SimpleGUI: Window resized to %ux%u\n", m_width, m_height);
                break;
                
            case ButtonPress:
                printf("SimpleGUI: Mouse button pressed at (%d, %d)\n",
                       event.xbutton.x, event.xbutton.y);
                break;
                
            case KeyPress:
                printf("SimpleGUI: Key pressed\n");
                break;
        }
    }
#endif
}