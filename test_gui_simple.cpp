#include <clap/clap.h>
#include <clap/ext/gui.h>
#include <dlfcn.h>
#include <iostream>
#include <cstring>

static const void* host_get_extension(const clap_host_t *host, const char *ext) { return nullptr; }
static void host_request_restart(const clap_host_t *host) {}
static void host_request_process(const clap_host_t *host) {}
static void host_request_callback(const clap_host_t *host) {}

static clap_host_t host = {
    CLAP_VERSION, nullptr, "test-host", "Test Host", "1.0", 
    "https://example.com", host_get_extension, host_request_restart, host_request_process, host_request_callback
};

int main() {
    void *lib = dlopen("build/MyFirstClapPlugin.so", RTLD_NOW);
    if (!lib) return 1;
    
    auto entry = (const clap_plugin_entry_t*)dlsym(lib, "clap_entry");
    if (!entry || !entry->init("test")) return 1;
    
    auto factory = (const clap_plugin_factory_t*)entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
    if (!factory) return 1;
    
    auto desc = factory->get_plugin_descriptor(factory, 0);
    auto plugin = factory->create_plugin(factory, &host, desc->id);
    if (!plugin || !plugin->init(plugin)) return 1;
    
    auto gui = (const clap_plugin_gui_t*)plugin->get_extension(plugin, CLAP_EXT_GUI);
    if (!gui) {
        std::cout << "No GUI extension" << std::endl;
        plugin->destroy(plugin);
        entry->deinit();
        dlclose(lib);
        return 0;
    }
    
    // Test create/destroy cycle
    if (gui->create(plugin, CLAP_WINDOW_API_X11, false)) {
        std::cout << "GUI created successfully" << std::endl;
        gui->destroy(plugin);
        std::cout << "GUI destroyed successfully" << std::endl;
    }
    
    plugin->destroy(plugin);
    entry->deinit();
    dlclose(lib);
    std::cout << "GUI test completed without crashes" << std::endl;
    return 0;
}
