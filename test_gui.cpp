#include "my_plugin.h"
#include <iostream>
#include <thread>
#include <chrono>

// Simple test host that demonstrates the GUI functionality
int main() {
    std::cout << "Testing CLAP Plugin GUI..." << std::endl;
    
    // Create plugin factory
    const clap_plugin_factory* factory = get_plugin_factory();
    
    // Create a minimal host
    struct test_host {
        clap_host_t host;
    };
    
    test_host host_instance;
    host_instance.host.clap_version = CLAP_VERSION;
    host_instance.host.host_data = &host_instance;
    host_instance.host.name = "Test Host";
    host_instance.host.vendor = "Test";
    host_instance.host.url = "http://test.com";
    host_instance.host.version = "1.0.0";
    host_instance.host.get_extension = [](const clap_host_t* host, const char* extension_id) -> const void* {
        return nullptr;
    };
    host_instance.host.request_restart = [](const clap_host_t* host) {};
    host_instance.host.request_process = [](const clap_host_t* host) {};
    host_instance.host.request_callback = [](const clap_host_t* host) {};
    
    // Create plugin instance
    const clap_plugin_t* plugin = factory->create_plugin(factory, &host_instance.host, "com.example.myplugin");
    if (!plugin) {
        std::cerr << "Failed to create plugin" << std::endl;
        return 1;
    }
    
    // Initialize plugin
    if (!plugin->init(plugin)) {
        std::cerr << "Failed to initialize plugin" << std::endl;
        return 1;
    }
    
    std::cout << "Plugin initialized successfully" << std::endl;
    
    // Get GUI extension
    const clap_plugin_gui_t* gui = (const clap_plugin_gui_t*)plugin->get_extension(plugin, CLAP_EXT_GUI);
    if (!gui) {
        std::cerr << "GUI extension not available" << std::endl;
        plugin->destroy(plugin);
        return 1;
    }
    
    std::cout << "GUI extension available" << std::endl;
    
    // Test GUI API support - test with X11 embedded
    if (!gui->is_api_supported(plugin, CLAP_WINDOW_API_X11, false)) {
        std::cout << "X11 embedded GUI not supported, trying other APIs..." << std::endl;
        // Try Win32
        if (!gui->is_api_supported(plugin, CLAP_WINDOW_API_WIN32, false)) {
            std::cout << "Win32 embedded GUI not supported, trying Wayland floating..." << std::endl;
            // Try Wayland floating as fallback
            if (!gui->is_api_supported(plugin, CLAP_WINDOW_API_WAYLAND, true)) {
                std::cerr << "No supported GUI API found" << std::endl;
                plugin->destroy(plugin);
                return 1;
            } else {
                std::cout << "Wayland floating GUI supported" << std::endl;
            }
        } else {
            std::cout << "Win32 embedded GUI supported" << std::endl;
        }
    } else {
        std::cout << "X11 embedded GUI supported" << std::endl;
    }
    
    // Create GUI - use the preferred API
    const char* preferred_api;
    bool is_floating;
    gui->get_preferred_api(plugin, &preferred_api, &is_floating);
    
    if (!gui->create(plugin, preferred_api, is_floating)) {
        std::cerr << "Failed to create GUI" << std::endl;
        plugin->destroy(plugin);
        return 1;
    }
    
    std::cout << "GUI created successfully" << std::endl;
    
    // Get GUI size
    uint32_t width, height;
    if (gui->get_size(plugin, &width, &height)) {
        std::cout << "GUI size: " << width << "x" << height << std::endl;
    }
    
    // Set title
    gui->suggest_title(plugin, "CLAP Plugin Test Window");
    std::cout << "GUI title set" << std::endl;
    
    // Show GUI
    if (!gui->show(plugin)) {
        std::cerr << "Failed to show GUI" << std::endl;
        gui->destroy(plugin);
        plugin->destroy(plugin);
        return 1;
    }
    
    std::cout << "GUI shown - Test window should be visible!" << std::endl;
    std::cout << "Click the button to change its color from red to blue and back." << std::endl;
    std::cout << "The program will run for 30 seconds..." << std::endl;
    
    // Run for 30 seconds, calling on_main_thread to handle rendering
    auto start_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::seconds(30);
    
    while (std::chrono::steady_clock::now() - start_time < duration) {
        plugin->on_main_thread(plugin);
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
    
    std::cout << "Test completed. Cleaning up..." << std::endl;
    
    // Hide and destroy GUI
    gui->hide(plugin);
    gui->destroy(plugin);
    
    // Destroy plugin
    plugin->destroy(plugin);
    
    std::cout << "Test completed successfully!" << std::endl;
    return 0;
}