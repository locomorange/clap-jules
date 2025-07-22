#pragma once

#include <clap/ext/gui.h>
#include <cstring>
#include <algorithm>

namespace brisk_clap {

// Cross-platform GUI API utilities
class CrossPlatformGUISupport {
public:
    // Check if an API is supported on the current platform
    static bool isApiSupportedOnPlatform(const char* api, bool is_floating = false) {
        if (!api) return false;
        
        // Windows support
        #ifdef _WIN32
        if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0) return true;
        #endif
        
        // macOS support
        #ifdef __APPLE__
        if (strcmp(api, CLAP_WINDOW_API_COCOA) == 0) return true;
        #endif
        
        // Linux support
        #ifdef __linux__
        if (strcmp(api, CLAP_WINDOW_API_X11) == 0) return true;
        // Wayland only supports floating windows
        if (strcmp(api, CLAP_WINDOW_API_WAYLAND) == 0 && is_floating) return true;
        #endif
        
        return false;
    }
    
    // Get the preferred API for the current platform
    static const char* getPreferredApi() {
        #ifdef _WIN32
        return CLAP_WINDOW_API_WIN32;
        #elif defined(__APPLE__)
        return CLAP_WINDOW_API_COCOA;
        #elif defined(__linux__)
        return CLAP_WINDOW_API_X11; // Prefer X11 over Wayland for embedded windows
        #else
        return nullptr; // Unsupported platform
        #endif
    }
    
    // Get all supported APIs on the current platform
    static void getSupportedApis(const char** apis, size_t* count, size_t max_count) {
        size_t idx = 0;
        
        #ifdef _WIN32
        if (idx < max_count) apis[idx++] = CLAP_WINDOW_API_WIN32;
        #endif
        
        #ifdef __APPLE__
        if (idx < max_count) apis[idx++] = CLAP_WINDOW_API_COCOA;
        #endif
        
        #ifdef __linux__
        if (idx < max_count) apis[idx++] = CLAP_WINDOW_API_X11;
        if (idx < max_count) apis[idx++] = CLAP_WINDOW_API_WAYLAND;
        #endif
        
        *count = idx;
    }
    
    // Check if floating windows are preferred for the given API
    static bool prefersFloatingWindows(const char* api) {
        if (!api) return false;
        
        // Wayland requires floating windows for now
        if (strcmp(api, CLAP_WINDOW_API_WAYLAND) == 0) return true;
        
        // Other APIs prefer embedded windows
        return false;
    }
    
    // Get platform-specific window handle from clap_window_t
    static void* extractNativeHandle(const clap_window_t* window) {
        if (!window || !window->api) return nullptr;
        
        if (strcmp(window->api, CLAP_WINDOW_API_WIN32) == 0) {
            return window->win32;
        }
        else if (strcmp(window->api, CLAP_WINDOW_API_COCOA) == 0) {
            return window->cocoa;
        }
        else if (strcmp(window->api, CLAP_WINDOW_API_X11) == 0) {
            return reinterpret_cast<void*>(static_cast<uintptr_t>(window->x11));
        }
        else if (strcmp(window->api, CLAP_WINDOW_API_WAYLAND) == 0) {
            return window->ptr;
        }
        
        return window->ptr; // Fallback to generic pointer
    }
    
    // Platform-specific scaling information
    static bool usesLogicalPixels(const char* api) {
        if (!api) return false;
        
        // macOS Cocoa uses logical pixels
        if (strcmp(api, CLAP_WINDOW_API_COCOA) == 0) return true;
        
        // Other platforms use physical pixels
        return false;
    }
    
    // Validate and adjust size constraints based on platform
    static bool validateSize(const char* api, uint32_t* width, uint32_t* height) {
        if (!api || !width || !height) return false;
        
        uint32_t minWidth = 200, minHeight = 150;
        uint32_t maxWidth = 1200, maxHeight = 800;
        
        // Platform-specific constraints
        #ifdef _WIN32
        if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0) {
            minWidth = 150; minHeight = 100;
            maxWidth = 1920; maxHeight = 1080;
        }
        #endif
        
        #ifdef __APPLE__
        if (strcmp(api, CLAP_WINDOW_API_COCOA) == 0) {
            minWidth = 200; minHeight = 150;
            maxWidth = 1440; maxHeight = 900;
        }
        #endif
        
        #ifdef __linux__
        if (strcmp(api, CLAP_WINDOW_API_X11) == 0 || strcmp(api, CLAP_WINDOW_API_WAYLAND) == 0) {
            minWidth = 150; minHeight = 100;
            maxWidth = 1920; maxHeight = 1080;
        }
        #endif
        
        // Apply constraints
        bool changed = false;
        if (*width < minWidth) { *width = minWidth; changed = true; }
        if (*height < minHeight) { *height = minHeight; changed = true; }
        if (*width > maxWidth) { *width = maxWidth; changed = true; }
        if (*height > maxHeight) { *height = maxHeight; changed = true; }
        
        return !changed; // Return true if no changes were needed
    }
    
    // Get default scale factor for the platform
    static double getDefaultScaleFactor(const char* api) {
        if (usesLogicalPixels(api)) {
            return 1.0; // Cocoa handles scaling internally
        }
        
        // For physical pixel systems, try to detect system DPI
        #ifdef _WIN32
        // Windows DPI detection could be implemented here
        return 1.0;
        #elif defined(__linux__)
        // Linux DPI detection could be implemented here
        return 1.0;
        #else
        return 1.0;
        #endif
    }
};

} // namespace brisk_clap
