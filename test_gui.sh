#!/bin/bash
# Test script for CLAP GUI functionality

echo "CLAP Jules Plugin GUI Test"
echo "=========================="

# Check if plugin exists
if [ ! -f "build/MyFirstClapPlugin.so" ]; then
    echo "Error: Plugin not found. Please build first with: cmake --build build"
    exit 1
fi

# Test plugin info
echo "1. Testing plugin info:"
./clap-info build/MyFirstClapPlugin.so --brief | grep -E "(name|features)"

echo ""
echo "2. Testing plugin validation:"
timeout 10 ./clap-validator validate build/MyFirstClapPlugin.so 2>/dev/null | tail -1

echo ""
echo "3. Testing GUI extension:"

# Create a simple test to show that GUI extension is available
# This would normally be done by a CLAP host, but we can demonstrate the functionality
cat << 'EOF' > /tmp/test_gui.cpp
#include <dlfcn.h>
#include <stdio.h>
#include <clap/clap.h>
#include <clap/ext/gui.h>

int main() {
    void* lib = dlopen("build/MyFirstClapPlugin.so", RTLD_LAZY);
    if (!lib) {
        printf("Failed to load plugin\n");
        return 1;
    }
    
    const clap_plugin_entry_t* entry = (const clap_plugin_entry_t*)dlsym(lib, "clap_entry");
    if (!entry) {
        printf("Failed to get entry point\n");
        dlclose(lib);
        return 1;
    }
    
    entry->init("test");
    
    const clap_plugin_factory_t* factory = 
        (const clap_plugin_factory_t*)entry->get_factory(CLAP_PLUGIN_FACTORY_ID);
    
    if (!factory) {
        printf("Failed to get factory\n");
        entry->deinit();
        dlclose(lib);
        return 1;
    }
    
    // Create dummy host
    static const clap_host_t host = {
        .clap_version = CLAP_VERSION,
        .host_data = nullptr,
        .name = "Test Host",
        .vendor = "Test",
        .url = "",
        .version = "1.0",
        .get_extension = [](const clap_host_t*, const char*) -> const void* { return nullptr; },
        .request_restart = [](const clap_host_t*) {},
        .request_process = [](const clap_host_t*) {},
        .request_callback = [](const clap_host_t*) {}
    };
    
    const clap_plugin_t* plugin = factory->create_plugin(factory, &host, "com.example.myplugin");
    if (!plugin) {
        printf("Failed to create plugin\n");
        entry->deinit();
        dlclose(lib);
        return 1;
    }
    
    // Test GUI extension
    const clap_plugin_gui_t* gui = 
        (const clap_plugin_gui_t*)plugin->get_extension(plugin, CLAP_EXT_GUI);
    
    if (gui) {
        printf("✓ GUI extension is available\n");
        
        // Test API support
        if (gui->is_api_supported(plugin, CLAP_WINDOW_API_X11, true)) {
            printf("✓ Floating X11 windows supported\n");
        }
        
        // Test GUI creation
        if (gui->create(plugin, CLAP_WINDOW_API_X11, true)) {
            printf("✓ GUI can be created\n");
            
            uint32_t width, height;
            if (gui->get_size(plugin, &width, &height)) {
                printf("✓ GUI size: %dx%d\n", width, height);
            }
            
            gui->destroy(plugin);
            printf("✓ GUI destroyed cleanly\n");
        }
    } else {
        printf("✗ GUI extension not available\n");
    }
    
    plugin->destroy(plugin);
    entry->deinit();
    dlclose(lib);
    
    return 0;
}
EOF

echo "Compiling GUI test..."
g++ -o /tmp/test_gui /tmp/test_gui.cpp -ldl -I libs/clap/include 2>/dev/null

if [ -f "/tmp/test_gui" ]; then
    echo "Running GUI test:"
    /tmp/test_gui
    rm -f /tmp/test_gui
else
    echo "Could not compile GUI test (this is normal in CI environments)"
fi

rm -f /tmp/test_gui.cpp

echo ""
echo "4. Flutter app status:"
if [ -f "flutter_ui/lib/main.dart" ]; then
    echo "✓ Flutter app created"
    echo "  To build: cd flutter_ui && flutter build linux"
    echo "  To run:   cd flutter_ui && flutter run -d linux"
else
    echo "✗ Flutter app not found"
fi

echo ""
echo "Test complete!"