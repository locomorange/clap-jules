# Real-time Spectrum Analyzer CLAP Plugin

This project implements a real-time spectrum analyzer as a CLAP (CLever Audio Plugin) with advanced visualization capabilities.

## Features

### Real-time FFT Processing
- **FFT Size**: 512 points for optimal balance between frequency resolution and real-time performance
- **Sample Rate Support**: Adaptive to host sample rate (typically 44.1kHz/48kHz)
- **Audio Input**: Captures left channel audio input from DAW
- **Real-time Processing**: Processes audio in the `my_plugin_process` callback with minimal latency

### Multiple Visualization Modes
The spectrum analyzer provides four distinct drawing modes that automatically cycle every 5 seconds:

1. **Lines Mode**: Connected line graph showing frequency spectrum as smooth curves
2. **Dots Mode**: Individual dots representing each frequency bin for precise visualization
3. **Bins Mode**: Vertical bar graph representation similar to traditional spectrum analyzers  
4. **Fill Mode**: Filled area under the spectrum curve for smooth visual presentation

### Visual Features
- **Frequency Grid**: Vertical lines showing frequency divisions (0Hz to Nyquist)
- **Amplitude Grid**: Horizontal lines for amplitude reference
- **Frequency Labels**: Automatic Hz/kHz labeling on the frequency axis
- **dB Scaling**: Logarithmic amplitude scaling for better dynamic range visualization
- **Visual Smoothing**: Exponential smoothing for stable, flicker-free display
- **Real-time Updates**: ~60 FPS rendering for smooth animation

## Technical Implementation

### Core Components
- `SimpleFFT`: Custom Cooley-Tukey FFT implementation optimized for real-time use
- `SpectrumAnalyzer`: Main analysis class handling audio buffering and spectrum computation
- CLAP Plugin Integration: Full integration with CLAP specification including GUI extension

### Performance Optimizations
- Power-of-2 FFT sizes for optimal performance
- Circular audio buffering for continuous processing
- Efficient magnitude calculation from complex FFT output
- Smooth visual transitions to reduce computational load

### Audio Processing Flow
1. Audio input captured in `my_plugin_process` callback
2. Audio samples stored in circular buffer
3. When buffer is full, FFT is performed on the data
4. Magnitude spectrum is calculated and smoothed
5. Visual rendering updates the spectrum display
6. Audio is passed through for monitoring (optional)

## Building and Testing

```bash
# Build the project
cmake . -B build
cmake --build build --config Release

# Run tests (16 total tests)
ctest --test-dir build --output-on-failure

# Validate with CLAP tools
./clap-validator validate build/MyFirstClapPlugin.so
./clap-info build/MyFirstClapPlugin.so --brief
```

## Plugin Information
- **ID**: `com.example.spectrum-analyzer`
- **Name**: Real-time Spectrum Analyzer  
- **Vendor**: CLAP Jules
- **Version**: 1.0.0
- **Features**: `analyzer`, `audio_effect`
- **Extensions**: GUI support for real-time visualization

## Usage in DAW
1. Load the plugin on an audio track or bus
2. The plugin will automatically start analyzing incoming audio
3. The GUI shows the real-time spectrum with automatic mode cycling
4. Drawing modes cycle every 5 seconds: Lines → Dots → Bins → Fill
5. Audio passes through unmodified for monitoring

## Test Coverage
The implementation includes comprehensive tests covering:
- FFT functionality and accuracy
- Spectrum analyzer initialization and processing
- Drawing mode switching
- Frequency calculation correctness
- Audio data handling and buffering
- Memory management and resource cleanup

All tests pass (16/16) with full CLAP validation compliance (7/7 validator tests passed).