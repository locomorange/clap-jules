#include <iostream>
#include <dlfcn.h>
#include <clap/clap.h>
#include <clap/ext/gui.h>

int main() {
    std::cout << "Testing CLAP GUI functionality..." << std::endl;
    
    // Load the plugin
    void* lib = dlopen("./build/MyFirstClapPlugin.so", RTLD_LAZY);
    if (!lib) {
        std::cerr << "Failed to load plugin: " << dlerror() << std::endl;
        return 1;
    }
    
    // Get the entry point
    clap_plugin_entry_t* entry = (clap_plugin_entry_t*)dlsym(lib, "clap_entry");
    if (!entry) {
        std::cerr << "Failed to find clap_entry" << std::endl;
        dlclose(lib);
        return 1;
    }
    
    // Initialize the plugin
    if (!entry->init("./build/MyFirstClapPlugin.so")) {
        std::cerr << "Failed to initialize plugin" << std::endl;
        dlclose(lib);
        return 1;
    }
    
    // Get the factory
    const clap_plugin_factory_t* factory = 
        (const clap_plugin_factory_t*)entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
    if (!factory) {
        std::cerr << "Failed to get plugin factory" << std::endl;
        entry->deinit();
        dlclose(lib);
        return 1;
    }
    
    // Create a dummy host
    clap_host_t host = {};
    host.clap_version = CLAP_VERSION;
    host.name = "GUI Test Host";
    host.vendor = "Test";
    host.version = "1.0.0";
    
    // Create plugin instance
    const clap_plugin_t* plugin = factory->create_plugin(factory, &host, "com.example.myplugin");
    if (!plugin) {
        std::cerr << "Failed to create plugin instance" << std::endl;
        entry->deinit();
        dlclose(lib);
        return 1;
    }
    
    // Initialize plugin
    if (!plugin->init(plugin)) {
        std::cerr << "Failed to initialize plugin" << std::endl;
        plugin->destroy(plugin);
        entry->deinit();
        dlclose(lib);
        return 1;
    }
    
    // Get GUI extension
    const clap_plugin_gui_t* gui = 
        (const clap_plugin_gui_t*)plugin->get_extension(plugin, CLAP_EXT_GUI);
    if (!gui) {
        std::cerr << "Plugin does not support GUI extension" << std::endl;
        plugin->destroy(plugin);
        entry->deinit();
        dlclose(lib);
        return 1;
    }
    
    std::cout << "GUI extension found!" << std::endl;
    
    // Test GUI functions
    std::cout << "Testing GUI API support..." << std::endl;
    bool x11_supported = gui->is_api_supported(plugin, CLAP_WINDOW_API_X11, false);
    std::cout << "X11 embedded support: " << (x11_supported ? "YES" : "NO") << std::endl;
    
    bool x11_floating_supported = gui->is_api_supported(plugin, CLAP_WINDOW_API_X11, true);
    std::cout << "X11 floating support: " << (x11_floating_supported ? "YES" : "NO") << std::endl;
    
    // Test preferred API
    const char* preferred_api = nullptr;
    bool is_floating = false;
    if (gui->get_preferred_api(plugin, &preferred_api, &is_floating)) {
        std::cout << "Preferred API: " << preferred_api << " (floating: " << (is_floating ? "YES" : "NO") << ")" << std::endl;
    }
    
    // Test GUI creation
    std::cout << "Creating GUI..." << std::endl;
    if (gui->create(plugin, CLAP_WINDOW_API_X11, false)) {
        std::cout << "GUI created successfully!" << std::endl;
        
        // Test size
        uint32_t width, height;
        if (gui->get_size(plugin, &width, &height)) {
            std::cout << "GUI size: " << width << "x" << height << std::endl;
        }
        
        // Test resize
        std::cout << "Testing resize..." << std::endl;
        if (gui->set_size(plugin, 400, 300)) {
            std::cout << "Resize successful!" << std::endl;
        }
        
        // Test show/hide
        std::cout << "Testing show/hide..." << std::endl;
        if (gui->show(plugin)) {
            std::cout << "GUI shown!" << std::endl;
        }
        
        if (gui->hide(plugin)) {
            std::cout << "GUI hidden!" << std::endl;
        }
        
        // Destroy GUI
        gui->destroy(plugin);
        std::cout << "GUI destroyed!" << std::endl;
    } else {
        std::cerr << "Failed to create GUI" << std::endl;
    }
    
    // Cleanup
    plugin->destroy(plugin);
    entry->deinit();
    dlclose(lib);
    
    std::cout << "GUI test completed successfully!" << std::endl;
    return 0;
}