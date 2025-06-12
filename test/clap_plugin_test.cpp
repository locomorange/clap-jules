#include <iostream>
#include <dlfcn.h>
#include <clap/clap.h>

// A minimal CLAP host implementation for testing
class MinimalHost {
public:
    static void request_restart(const clap_host_t *host) {
        std::cout << "Host: Restart requested" << std::endl;
    }
    
    static void request_process(const clap_host_t *host) {
        std::cout << "Host: Process requested" << std::endl;
    }
    
    static void request_callback(const clap_host_t *host) {
        std::cout << "Host: Callback requested" << std::endl;
    }
    
    static const void* get_extension(const clap_host_t *host, const char *extension_id) {
        std::cout << "Host: Extension requested: " << extension_id << std::endl;
        return nullptr; // No extensions supported in this minimal host
    }
    
    static const clap_host_t host;
};

const clap_host_t MinimalHost::host = {
    CLAP_VERSION,
    nullptr, // host_data
    "clap-plugin-test", // name
    "CLAP Testing", // vendor
    "https://github.com/free-audio/clap", // url
    "1.0.0", // version
    MinimalHost::get_extension,
    MinimalHost::request_restart,
    MinimalHost::request_process,
    MinimalHost::request_callback
};

bool test_plugin(const char* plugin_path) {
    std::cout << "Testing CLAP plugin: " << plugin_path << std::endl;
    
    // Load the plugin library
    void* lib = dlopen(plugin_path, RTLD_LAZY);
    if (!lib) {
        std::cerr << "Error: Cannot load plugin library: " << dlerror() << std::endl;
        return false;
    }
    
    // Get the CLAP entry point
    const clap_plugin_entry_t* entry = (const clap_plugin_entry_t*)dlsym(lib, "clap_entry");
    if (!entry) {
        std::cerr << "Error: Cannot find clap_entry symbol: " << dlerror() << std::endl;
        dlclose(lib);
        return false;
    }
    
    std::cout << "✓ Plugin library loaded successfully" << std::endl;
    std::cout << "✓ CLAP entry point found" << std::endl;
    
    // Initialize the plugin
    if (!entry->init || !entry->init(plugin_path)) {
        std::cerr << "Error: Plugin initialization failed" << std::endl;
        dlclose(lib);
        return false;
    }
    
    std::cout << "✓ Plugin initialized successfully" << std::endl;
    
    // Get the plugin factory
    const clap_plugin_factory_t* factory = 
        (const clap_plugin_factory_t*)entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
    if (!factory) {
        std::cerr << "Error: Cannot get plugin factory" << std::endl;
        entry->deinit();
        dlclose(lib);
        return false;
    }
    
    std::cout << "✓ Plugin factory obtained" << std::endl;
    
    // Get plugin count
    uint32_t plugin_count = factory->get_plugin_count(factory);
    std::cout << "Plugin count: " << plugin_count << std::endl;
    
    if (plugin_count == 0) {
        std::cerr << "Error: No plugins found in factory" << std::endl;
        entry->deinit();
        dlclose(lib);
        return false;
    }
    
    // Get plugin descriptor
    const clap_plugin_descriptor_t* desc = factory->get_plugin_descriptor(factory, 0);
    if (!desc) {
        std::cerr << "Error: Cannot get plugin descriptor" << std::endl;
        entry->deinit();
        dlclose(lib);
        return false;
    }
    
    std::cout << "Plugin info:" << std::endl;
    std::cout << "  ID: " << desc->id << std::endl;
    std::cout << "  Name: " << desc->name << std::endl;
    std::cout << "  Vendor: " << desc->vendor << std::endl;
    std::cout << "  Version: " << desc->version << std::endl;
    
    // Create plugin instance
    const clap_plugin_t* plugin = factory->create_plugin(factory, &MinimalHost::host, desc->id);
    if (!plugin) {
        std::cerr << "Error: Cannot create plugin instance" << std::endl;
        entry->deinit();
        dlclose(lib);
        return false;
    }
    
    std::cout << "✓ Plugin instance created successfully" << std::endl;
    
    // Initialize plugin instance
    if (!plugin->init(plugin)) {
        std::cerr << "Error: Plugin instance initialization failed" << std::endl;
        plugin->destroy(plugin);
        entry->deinit();
        dlclose(lib);
        return false;
    }
    
    std::cout << "✓ Plugin instance initialized successfully" << std::endl;
    
    // Test basic plugin operations
    // Activate the plugin
    double sample_rate = 44100.0;
    uint32_t min_frames = 64;
    uint32_t max_frames = 1024;
    
    if (!plugin->activate(plugin, sample_rate, min_frames, max_frames)) {
        std::cerr << "Error: Plugin activation failed" << std::endl;
        plugin->destroy(plugin);
        entry->deinit();
        dlclose(lib);
        return false;
    }
    
    std::cout << "✓ Plugin activated successfully (SR: " << sample_rate << ", Frames: " << min_frames << "-" << max_frames << ")" << std::endl;
    
    // Start processing
    if (!plugin->start_processing(plugin)) {
        std::cerr << "Error: Plugin start processing failed" << std::endl;
        plugin->deactivate(plugin);
        plugin->destroy(plugin);
        entry->deinit();
        dlclose(lib);
        return false;
    }
    
    std::cout << "✓ Plugin processing started successfully" << std::endl;
    
    // Stop processing
    plugin->stop_processing(plugin);
    std::cout << "✓ Plugin processing stopped successfully" << std::endl;
    
    // Deactivate plugin
    plugin->deactivate(plugin);
    std::cout << "✓ Plugin deactivated successfully" << std::endl;
    
    // Cleanup
    plugin->destroy(plugin);
    std::cout << "✓ Plugin instance destroyed successfully" << std::endl;
    
    entry->deinit();
    std::cout << "✓ Plugin deinitialized successfully" << std::endl;
    
    dlclose(lib);
    std::cout << "✓ Plugin library unloaded successfully" << std::endl;
    
    std::cout << "\n🎉 All tests passed! Plugin is working correctly." << std::endl;
    
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <plugin_path>" << std::endl;
        return 1;
    }
    
    std::cout << "CLAP Plugin Test Utility" << std::endl;
    std::cout << "========================" << std::endl;
    
    if (test_plugin(argv[1])) {
        std::cout << "Test result: SUCCESS" << std::endl;
        return 0;
    } else {
        std::cout << "Test result: FAILED" << std::endl;
        return 1;
    }
}