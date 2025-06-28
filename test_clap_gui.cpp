#include <iostream>
#include <clap/clap.h>
#include <clap/ext/gui.h>

#ifdef _WIN32
#include <windows.h>
#define dlopen(name, flags) LoadLibraryA(name)
#define dlsym(handle, name) GetProcAddress((HMODULE)handle, name)
#define dlclose(handle) FreeLibrary((HMODULE)handle)
#define dlerror() "Windows DLL error"
#else
#include <dlfcn.h>
#endif

int main() {
    std::cout << "Testing CLAP plugin GUI extension..." << std::endl;
    
    // Load the plugin library
#ifdef _WIN32
    void* lib = dlopen("./Release/MyFirstClapPlugin.clap", 0);
#else
    void* lib = dlopen("./MyFirstClapPlugin.so", RTLD_LAZY);
#endif
    if (!lib) {
        std::cerr << "Failed to load plugin: " << dlerror() << std::endl;
        return 1;
    }
    
    // Get the entry point
    const clap_plugin_entry_t* entry = (const clap_plugin_entry_t*)dlsym(lib, "clap_entry");
    if (!entry) {
        std::cerr << "Failed to find clap_entry symbol: " << dlerror() << std::endl;
        dlclose(lib);
        return 1;
    }
    
    std::cout << "Plugin entry found, initializing..." << std::endl;
    
    // Initialize plugin
    if (!entry->init("test_path")) {
        std::cerr << "Failed to initialize plugin" << std::endl;
        dlclose(lib);
        return 1;
    }
    
    // Get plugin factory
    const clap_plugin_factory_t* factory = 
        (const clap_plugin_factory_t*)entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
    if (!factory) {
        std::cerr << "Failed to get plugin factory" << std::endl;
        entry->deinit();
        dlclose(lib);
        return 1;
    }
    
    std::cout << "Plugin factory found, plugin count: " << factory->get_plugin_count(factory) << std::endl;
    
    // Get plugin descriptor
    const clap_plugin_descriptor_t* desc = factory->get_plugin_descriptor(factory, 0);
    if (!desc) {
        std::cerr << "Failed to get plugin descriptor" << std::endl;
        entry->deinit();
        dlclose(lib);
        return 1;
    }
    
    std::cout << "Plugin: " << desc->name << " (ID: " << desc->id << ")" << std::endl;
    
    // Create dummy host
    clap_host_t host = {
        CLAP_VERSION,
        nullptr,        // host_data
        "TestHost",     // name
        "Test",         // vendor
        "https://test", // url
        "1.0.0",        // version
        nullptr,        // get_extension
        nullptr,        // request_restart
        nullptr,        // request_process
        nullptr,        // request_callback
    };
    
    // Create plugin instance
    const clap_plugin_t* plugin = factory->create_plugin(factory, &host, desc->id);
    if (!plugin) {
        std::cerr << "Failed to create plugin instance" << std::endl;
        entry->deinit();
        dlclose(lib);
        return 1;
    }
    
    std::cout << "Plugin instance created, checking GUI extension..." << std::endl;
    
    // Check GUI extension
    const clap_plugin_gui_t* gui_ext = 
        (const clap_plugin_gui_t*)plugin->get_extension(plugin, CLAP_EXT_GUI);
    
    if (!gui_ext) {
        std::cerr << "Plugin does not support GUI extension" << std::endl;
    } else {
        std::cout << "✓ GUI extension found!" << std::endl;
        
        // Test GUI capabilities
        bool supports_floating = gui_ext->is_api_supported(plugin, nullptr, true);
        std::cout << "Supports floating windows: " << (supports_floating ? "YES" : "NO") << std::endl;
        
        bool supports_x11_embedded = gui_ext->is_api_supported(plugin, CLAP_WINDOW_API_X11, false);
        std::cout << "Supports X11 embedded: " << (supports_x11_embedded ? "YES" : "NO") << std::endl;
        
        // Test GUI creation
        if (gui_ext->create(plugin, nullptr, true)) {
            std::cout << "✓ GUI created successfully!" << std::endl;
            
            uint32_t width, height;
            if (gui_ext->get_size(plugin, &width, &height)) {
                std::cout << "GUI size: " << width << "x" << height << std::endl;
            }
            
            // Clean up GUI
            gui_ext->destroy(plugin);
            std::cout << "GUI destroyed" << std::endl;
        } else {
            std::cerr << "Failed to create GUI" << std::endl;
        }
    }
    
    // Clean up
    plugin->destroy(plugin);
    entry->deinit();
    dlclose(lib);
    
    std::cout << "Test completed successfully!" << std::endl;
    return 0;
}