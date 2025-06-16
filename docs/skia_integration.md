# Skia Integration Example

This document provides examples of how to use Skia graphics library within the clap-jules plugin.

## Basic Skia Usage

When Skia is enabled (`CLAP_JULES_WITH_SKIA` is defined), you can use Skia's powerful 2D graphics API:

```cpp
#ifdef CLAP_JULES_WITH_SKIA
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"

// Create a raster surface
SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);

if (surface) {
    SkCanvas* canvas = surface->getCanvas();
    
    // Draw shapes
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    
    canvas->drawRect(SkRect::MakeXYWH(10, 10, 100, 100), paint);
    
    // Draw circles
    paint.setColor(SK_ColorBLUE);
    canvas->drawCircle(60, 60, 30, paint);
    
    // Save to image file or use in plugin UI
    // ...
}
#endif
```

## Integration Points

### Plugin Initialization
The plugin demonstrates Skia integration in the `my_plugin_init()` function:
- Creates a test surface
- Performs basic drawing operations
- Validates Skia functionality

### Potential Use Cases
- Real-time audio visualization
- Plugin parameter UI rendering
- Spectrum analyzers
- Waveform displays
- Custom control graphics

## Build Configuration

### Without Skia (Default)
```bash
cmake . -B build
cmake --build build
```

### With Skia
```bash
./scripts/setup_skia.sh  # First time only
cmake . -B build -DENABLE_SKIA=ON
cmake --build build
```

## Performance Considerations

- Skia operations should be performed on appropriate threads
- Consider caching surfaces for real-time audio applications
- Use GPU acceleration when available for better performance
- Profile graphics operations to ensure they don't impact audio processing

## Troubleshooting

### Common Issues

1. **Skia not found**: Run `./scripts/setup_skia.sh` first
2. **Build errors**: Ensure depot_tools is in PATH
3. **Linking errors**: Check that Skia was built successfully
4. **Runtime errors**: Verify Skia surface creation succeeds

### Debug Information
The plugin outputs Skia status messages during initialization:
- "Skia integration enabled" - Skia is available
- "Skia surface creation test successful" - Basic functionality works
- "Skia integration not enabled" - Built without Skia support