# CLAP Plugin GUI Implementation

This document describes the CLAP plugin GUI implementation that creates a window with a color-changing button.

## Implementation Details

The plugin implements the CLAP GUI extension (`clap.gui`) and provides:

1. **Floating Window Support**: Uses GLFW to create a floating window
2. **Color-Changing Button**: A button that toggles between red and blue when clicked
3. **OpenGL Rendering**: Simple 2D rendering using OpenGL
4. **Mouse Input Handling**: Click detection for the button

## Features

### Window Management
- Creates a 400x300 pixel window
- Floating window (not embedded in host)
- Proper initialization and cleanup
- Window title can be set by the host

### Interactive Button
- Located in the center of the window (100x50 pixels)
- Starts with red color
- Toggles to blue when clicked
- Toggles back to red when clicked again
- White border for visual clarity

### Technical Implementation
- Uses GLFW for window creation and event handling
- OpenGL for rendering the button graphics
- Proper CLAP GUI extension interface implementation
- Thread-safe rendering through main thread callbacks

## Usage Example

```cpp
// Get the GUI extension
const clap_plugin_gui_t* gui = plugin->get_extension(plugin, CLAP_EXT_GUI);

// Check if floating GUI is supported
if (gui->is_api_supported(plugin, "", true)) {
    // Create the GUI
    if (gui->create(plugin, "", true)) {
        // Set window title
        gui->suggest_title(plugin, "My CLAP Plugin");
        
        // Show the window
        gui->show(plugin);
        
        // In your main loop, call the plugin's main thread callback
        // to handle rendering and events
        plugin->on_main_thread(plugin);
        
        // Hide and cleanup when done
        gui->hide(plugin);
        gui->destroy(plugin);
    }
}
```

## Building and Testing

The GUI functionality is automatically included when GLFW is available:

```bash
# Build with GLFW support
cmake . -B build
cmake --build build

# Run tests to verify GUI functionality
./build/my_plugin_tests

# Run standalone GUI test (requires desktop environment)
./build/test_gui
```

## Compatibility

- **Desktop Environments**: Full functionality with window display and mouse interaction
- **Headless Environments**: Graceful fallback - GUI extension is available but window creation fails safely
- **Host Integration**: Compatible with any CLAP host that supports the GUI extension
- **Cross-Platform**: Works on Linux, macOS, and Windows (with appropriate OpenGL support)

## Error Handling

The implementation includes proper error handling for:
- GLFW initialization failure
- Window creation failure
- OpenGL context creation issues
- Mouse input processing errors

All failures are logged and handled gracefully without crashing the plugin.

## Testing

The implementation includes comprehensive tests:
- GUI extension availability verification
- API support checking
- Window size reporting
- Resize capabilities testing
- Plugin state initialization verification

All tests pass in both desktop and headless environments.