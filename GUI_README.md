# Professional Audio Plugin GUI - Soothe2 Style

This implementation provides a modern, professional audio plugin GUI similar to popular plugins like Soothe2, built using the VSTGUI framework for the CLAP plugin standard.

## Features

### Visual Design
- **Professional Dark Theme**: Modern dark color scheme with soft blue accents
- **Clean Layout**: Two-panel design with logical control grouping
- **Professional Typography**: Consistent Arial font family with proper hierarchy
- **Interactive Elements**: Hover states, selection feedback, and smooth animations

### Left Control Panel (250x600px)
- **Brand Header**: "Soothe Pro - Dynamic EQ & Resonance Control"
- **Preset Management**: Dropdown with 5 predefined settings
- **Main Controls**:
  - Large cutoff knob (80px diameter)
  - Secondary knobs for resonance and drive (60px diameter)
  - Vertical mix slider
  - Output gain control
- **Bypass Button**: Professional styling with status indication
- **Status Display**: CPU usage and latency monitoring

### Right Panel - EQ Visualization (650x600px)
- **Interactive Frequency Graph**: Real-time visualization of EQ response
- **Logarithmic Frequency Grid**: 20Hz to 20kHz with proper scaling
- **Gain Range**: ±12dB with clear reference lines
- **Draggable EQ Nodes**: Three interactive control points (200Hz, 1kHz, 5kHz)
- **Real-time Response Curve**: Smooth frequency response calculation
- **Visual Feedback**: Selected nodes show frequency and gain values

## Color Scheme

```css
Background:  #191a1e  /* Professional dark base */
Panels:      #23242a  /* Subtle panel contrast */
Accent:      #78b4ff  /* Soft blue highlights */
Text:        #dcdce1  /* High contrast readability */
Grid:        #3c3c46  /* Subtle reference lines */
```

## Technical Implementation

### Architecture
- **VSTGUI Framework**: Cross-platform GUI rendering
- **Custom Views**: EQVisualizationView for frequency response
- **Modular Design**: Easy to customize and extend
- **Memory Efficient**: Proper resource management

### Key Classes
- `MyPluginEditor`: Main GUI controller
- `EQVisualizationView`: Interactive frequency response display
- `EQNode`: Data structure for EQ control points

### File Structure
```
my_plugin_gui.h         # GUI interface definitions
my_plugin_gui.cpp       # GUI implementation
my_plugin.h            # Plugin parameter definitions
my_plugin.cpp          # Main plugin implementation
```

## Customization

### Colors
Modify the static color constants in `MyPluginEditor`:
```cpp
static const CColor kBackgroundColor(25, 25, 30, 255);
static const CColor kAccentColor(120, 180, 255, 255);
// etc.
```

### Layout
Adjust panel sizes and positions in:
- `createLeftPanel()` - Control panel layout
- `createRightPanel()` - EQ visualization area

### Controls
Add new controls by:
1. Declaring in the header file
2. Creating in the appropriate panel method
3. Styling with `styleControl()`
4. Adding parameter binding

## Building

The GUI is automatically included when building with VSTGUI support:

```bash
# Configure with GUI support
cmake . -B build -DCLAP_INCLUDE_GUI=ON

# Build the plugin
cmake --build build --config Release
```

## Integration

The GUI integrates with the CLAP plugin through:
- Parameter synchronization
- Host window embedding
- Cross-platform window management
- Event handling for user interactions

## Future Enhancements

Potential improvements:
- Parameter automation display
- Spectrum analyzer overlay
- Additional EQ filter types
- Preset management system
- Undo/redo functionality
- Accessibility features

## Dependencies

- VSTGUI 4.x
- CLAP SDK
- Platform-specific window system libraries (X11, Cocoa, Win32)

## Compatibility

- **Platforms**: Linux, macOS, Windows
- **Plugin Format**: CLAP
- **DAWs**: All CLAP-compatible hosts
- **GUI Framework**: VSTGUI 4.x

This implementation demonstrates modern audio plugin GUI design principles and provides a solid foundation for professional audio software development.