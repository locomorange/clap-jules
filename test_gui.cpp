#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <clap/clap.h>
#include <clap/ext/gui.h>

#ifdef __linux__
#include <X11/Xlib.h>
#endif

// Simple test host implementation
class TestHost {
public:
    TestHost() {
        // Initialize host structure
        host.clap_version = CLAP_VERSION;
        host.host_data = this;
        host.name = "Test GUI Host";
        host.vendor = "Test";
        host.url = "https://example.com";
        host.version = "1.0.0";
        host.get_extension = get_extension_static;
        host.request_restart = request_restart_static;
        host.request_process = request_process_static;
        host.request_callback = request_callback_static;
    }
    
    clap_host_t host;
    
private:
    static const void* get_extension_static(const clap_host_t* host, const char* extension_id) {
        TestHost* self = static_cast<TestHost*>(host->host_data);
        return self->get_extension(extension_id);
    }
    
    static void request_restart_static(const clap_host_t* host) {
        printf("Host: Restart requested\n");
    }
    
    static void request_process_static(const clap_host_t* host) {
        printf("Host: Process requested\n");
    }
    
    static void request_callback_static(const clap_host_t* host) {
        printf("Host: Callback requested\n");
    }
    
    const void* get_extension(const char* extension_id) {
        printf("Host: Extension requested: %s\n", extension_id);
        return nullptr; // No extensions for this simple test
    }
};

int main() {
    printf("CLAP GUI Test Program\n");
    printf("=====================\n\n");
    
    // Load the plugin library
    const char* plugin_path = "./build/MyFirstClapPlugin.so";
    void* lib = dlopen(plugin_path, RTLD_LAZY);
    if (!lib) {
        printf("Error: Could not load plugin: %s\n", dlerror());
        return 1;
    }
    
    // Get the plugin entry point
    const clap_plugin_entry_t* entry = (const clap_plugin_entry_t*)dlsym(lib, "clap_entry");
    if (!entry) {
        printf("Error: Could not find clap_entry symbol\n");
        dlclose(lib);
        return 1;
    }
    
    // Initialize the plugin library
    if (!entry->init(plugin_path)) {
        printf("Error: Plugin initialization failed\n");
        dlclose(lib);
        return 1;
    }
    
    // Get the plugin factory
    const clap_plugin_factory_t* factory = 
        (const clap_plugin_factory_t*)entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
    if (!factory) {
        printf("Error: Could not get plugin factory\n");
        entry->deinit();
        dlclose(lib);
        return 1;
    }
    
    // Create test host
    TestHost test_host;
    
    // Get first plugin descriptor
    const clap_plugin_descriptor_t* desc = factory->get_plugin_descriptor(factory, 0);
    if (!desc) {
        printf("Error: No plugin descriptor available\n");
        entry->deinit();
        dlclose(lib);
        return 1;
    }
    
    printf("Plugin found: %s\n", desc->name);
    printf("Vendor: %s\n", desc->vendor);
    printf("Version: %s\n", desc->version);
    printf("Description: %s\n\n", desc->description);
    
    // Create plugin instance
    const clap_plugin_t* plugin = factory->create_plugin(factory, &test_host.host, desc->id);
    if (!plugin) {
        printf("Error: Could not create plugin instance\n");
        entry->deinit();
        dlclose(lib);
        return 1;
    }
    
    // Initialize plugin
    if (!plugin->init(plugin)) {
        printf("Error: Plugin init failed\n");
        plugin->destroy(plugin);
        entry->deinit();
        dlclose(lib);
        return 1;
    }
    
    printf("Plugin initialized successfully\n");
    
    // Test GUI extension
    const clap_plugin_gui_t* gui = 
        (const clap_plugin_gui_t*)plugin->get_extension(plugin, CLAP_EXT_GUI);
    
    if (!gui) {
        printf("Error: Plugin does not support GUI extension\n");
    } else {
        printf("GUI extension found!\n\n");
        
        // Test API support
        const char* api;
        bool is_floating;
        
        if (gui->get_preferred_api(plugin, &api, &is_floating)) {
            printf("Preferred API: %s (floating: %s)\n", api, is_floating ? "yes" : "no");
            
            if (gui->is_api_supported(plugin, api, is_floating)) {
                printf("API is supported\n");
                
                // Create GUI
                if (gui->create(plugin, api, is_floating)) {
                    printf("GUI created successfully\n");
                    
                    // Get size
                    uint32_t width, height;
                    if (gui->get_size(plugin, &width, &height)) {
                        printf("GUI size: %dx%d\n", width, height);
                    }
                    
                    // Test resize capability
                    if (gui->can_resize(plugin)) {
                        printf("GUI can be resized\n");
                        
                        // Try to resize
                        uint32_t new_width = 500, new_height = 400;
                        if (gui->adjust_size(plugin, &new_width, &new_height)) {
                            printf("Adjusted size: %dx%d\n", new_width, new_height);
                            
                            if (gui->set_size(plugin, new_width, new_height)) {
                                printf("Size set successfully\n");
                            }
                        }
                    }
                    
#ifdef __linux__
                    // For Linux, we can test creating a simple window
                    Display* display = XOpenDisplay(NULL);
                    if (display) {
                        printf("X11 display opened\n");
                        
                        int screen = DefaultScreen(display);
                        Window root = RootWindow(display, screen);
                        
                        // Create a simple parent window
                        Window parent_window = XCreateSimpleWindow(
                            display, root,
                            100, 100, 600, 500, 1,
                            BlackPixel(display, screen),
                            WhitePixel(display, screen)
                        );
                        
                        if (parent_window) {
                            printf("Created parent X11 window\n");
                            
                            // Set up CLAP window structure
                            clap_window_t clap_window;
                            clap_window.api = CLAP_WINDOW_API_X11;
                            clap_window.x11 = parent_window;
                            
                            if (gui->set_parent(plugin, &clap_window)) {
                                printf("Parent window set successfully\n");
                                
                                // Map the parent window
                                XMapWindow(display, parent_window);
                                XFlush(display);
                                
                                // Show the plugin GUI
                                if (gui->show(plugin)) {
                                    printf("GUI shown successfully!\n");
                                    printf("Plugin GUI is now visible. Press Enter to continue...\n");
                                    getchar();
                                    
                                    // Hide the GUI
                                    gui->hide(plugin);
                                    printf("GUI hidden\n");
                                } else {
                                    printf("Failed to show GUI\n");
                                }
                            } else {
                                printf("Failed to set parent window\n");
                            }
                            
                            XDestroyWindow(display, parent_window);
                        }
                        
                        XCloseDisplay(display);
                    }
#endif
                    
                    // Destroy GUI
                    gui->destroy(plugin);
                    printf("GUI destroyed\n");
                } else {
                    printf("Failed to create GUI\n");
                }
            } else {
                printf("API not supported\n");
            }
        } else {
            printf("No preferred API available\n");
        }
    }
    
    // Clean up
    plugin->destroy(plugin);
    entry->deinit();
    dlclose(lib);
    
    printf("\nTest completed successfully!\n");
    return 0;
}