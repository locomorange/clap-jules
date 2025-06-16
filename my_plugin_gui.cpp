#include "my_plugin_gui.h"
#include "my_plugin.h"
#include <vstgui/lib/controls/cknob.h>
#include <vstgui/lib/controls/cslider.h>
#include <vstgui/lib/controls/ctextlabel.h>
#include <vstgui/lib/controls/cbuttons.h>
#include <vstgui/lib/controls/coptionmenu.h>
#include <vstgui/lib/controls/csegmentbutton.h>
#include <vstgui/lib/controls/ctextedit.h>
#include <vstgui/lib/controls/cswitch.h>
#include <vstgui/lib/controls/cvumeter.h>
#include <vstgui/lib/vstguiinit.h>
#include <vstgui/lib/cbitmap.h>
#include <vstgui/lib/cgraphicspath.h>
#include <iostream>
#include <cmath>

#ifdef __linux__
#include <vstgui/lib/platform/platform_x11.h>
#include <vstgui/lib/platform/linux/x11platform.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

// Static initialization flag
static bool vstgui_initialized = false;

// Professional color scheme inspired by Soothe2
const CColor MyPluginEditor::kBackgroundColor(25, 25, 30, 255);      // Dark background
const CColor MyPluginEditor::kPanelColor(35, 35, 42, 255);           // Panel background
const CColor MyPluginEditor::kAccentColor(120, 180, 255, 255);       // Blue accent
const CColor MyPluginEditor::kTextColor(220, 220, 225, 255);         // Light text
const CColor MyPluginEditor::kGridColor(60, 60, 70, 255);            // Grid lines

//=============================================================================
// EQVisualizationView Implementation
//=============================================================================

EQVisualizationView::EQVisualizationView(const CRect& size) 
    : CView(size), selectedNode(-1) {
    // Initialize default EQ nodes
    eqNodes.resize(3);
    eqNodes[0] = {200.0, 0.0, 1.0, CPoint(0, 0), false, false};
    eqNodes[1] = {1000.0, 0.0, 1.0, CPoint(0, 0), false, false};
    eqNodes[2] = {5000.0, 0.0, 1.0, CPoint(0, 0), false, false};
}

EQVisualizationView::~EQVisualizationView() {
}

void EQVisualizationView::draw(CDrawContext* context) {
    // Clear background
    context->setFillColor(MyPluginEditor::kBackgroundColor);
    context->drawRect(getViewSize(), kDrawFilled);
    
    drawGrid(context);
    drawFrequencyResponse(context);
    drawNodes(context);
}

void EQVisualizationView::drawGrid(CDrawContext* context) {
    context->setLineStyle(kLineSolid);
    context->setLineWidth(1.0);
    context->setFrameColor(MyPluginEditor::kGridColor);
    
    CRect bounds = getViewSize();
    bounds.makeIntegral();
    
    // Vertical grid lines (frequency divisions)
    const double freqs[] = {50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    for (double freq : freqs) {
        double x = bounds.left + (std::log10(freq / 20.0) / std::log10(20000.0 / 20.0)) * bounds.getWidth();
        if (x >= bounds.left && x <= bounds.right) {
            context->drawLine(CPoint(x, bounds.top), CPoint(x, bounds.bottom));
        }
    }
    
    // Horizontal grid lines (gain divisions)
    const double gains[] = {-12, -6, 0, 6, 12};
    for (double gain : gains) {
        double y = bounds.top + bounds.getHeight() * (1.0 - (gain + 12.0) / 24.0);
        if (y >= bounds.top && y <= bounds.bottom) {
            context->drawLine(CPoint(bounds.left, y), CPoint(bounds.right, y));
            
            // Draw gain labels
            if (gain == 0) {
                context->setLineWidth(2.0);
                context->drawLine(CPoint(bounds.left, y), CPoint(bounds.right, y));
                context->setLineWidth(1.0);
            }
        }
    }
}

void EQVisualizationView::drawFrequencyResponse(CDrawContext* context) {
    context->setLineStyle(kLineSolid);
    context->setLineWidth(2.0);
    context->setFrameColor(MyPluginEditor::kAccentColor);
    
    CRect bounds = getViewSize();
    auto path = context->createGraphicsPath();
    if (!path) return;
    
    bool firstPoint = true;
    
    // Draw smooth frequency response curve
    for (int x = 0; x < bounds.getWidth(); x += 2) {
        double freq = 20.0 * std::pow(20000.0 / 20.0, (double)x / bounds.getWidth());
        double totalGain = 0.0;
        
        // Calculate combined response from all EQ bands
        for (const auto& node : eqNodes) {
            if (node.gain != 0.0) {
                double ratio = freq / node.freq;
                double q = node.q;
                
                // Simple bell filter approximation
                double gain = node.gain / (1.0 + q * q * (ratio - 1.0/ratio) * (ratio - 1.0/ratio));
                totalGain += gain;
            }
        }
        
        // Clamp gain to reasonable range
        totalGain = std::max(-12.0, std::min(12.0, totalGain));
        
        double y = bounds.top + bounds.getHeight() * (1.0 - (totalGain + 12.0) / 24.0);
        CPoint point(bounds.left + x, y);
        
        if (firstPoint) {
            path->beginSubpath(point);
            firstPoint = false;
        } else {
            path->addLine(point);
        }
    }
    
    context->drawGraphicsPath(path, CDrawContext::kPathStroked);
    path->forget();
}

void EQVisualizationView::drawNodes(CDrawContext* context) {
    for (size_t i = 0; i < eqNodes.size(); ++i) {
        const auto& node = eqNodes[i];
        CPoint pos = frequencyToPosition(node.freq, node.gain);
        
        // Draw node circle
        CRect nodeRect(pos.x - 6, pos.y - 6, pos.x + 6, pos.y + 6);
        
        if (node.selected || selectedNode == (int)i) {
            context->setFillColor(MyPluginEditor::kAccentColor);
            context->setFrameColor(CColor(255, 255, 255, 255));
            context->setLineWidth(2.0);
        } else {
            context->setFillColor(CColor(80, 80, 90, 200));
            context->setFrameColor(MyPluginEditor::kAccentColor);
            context->setLineWidth(1.0);
        }
        
        context->drawEllipse(nodeRect, kDrawFilledAndStroked);
        
        // Draw frequency and gain labels
        if (node.selected || selectedNode == (int)i) {
            char freqText[32];
            char gainText[32];
            
            if (node.freq >= 1000) {
                snprintf(freqText, sizeof(freqText), "%.1fkHz", node.freq / 1000.0);
            } else {
                snprintf(freqText, sizeof(freqText), "%.0fHz", node.freq);
            }
            snprintf(gainText, sizeof(gainText), "%+.1fdB", node.gain);
            
            context->setFontColor(MyPluginEditor::kTextColor);
            context->setFont(kSystemFont);
            
            CRect textRect(pos.x - 30, pos.y - 25, pos.x + 30, pos.y - 10);
            context->drawString(freqText, textRect, kCenterText);
            textRect.offset(0, 15);
            context->drawString(gainText, textRect, kCenterText);
        }
    }
}

CPoint EQVisualizationView::frequencyToPosition(double freq, double gain) {
    CRect bounds = getViewSize();
    
    double x = bounds.left + (std::log10(freq / 20.0) / std::log10(20000.0 / 20.0)) * bounds.getWidth();
    double y = bounds.top + bounds.getHeight() * (1.0 - (gain + 12.0) / 24.0);
    
    return CPoint(x, y);
}

void EQVisualizationView::positionToFrequency(const CPoint& pos, double& freq, double& gain) {
    CRect bounds = getViewSize();
    
    double normalizedX = (pos.x - bounds.left) / bounds.getWidth();
    freq = 20.0 * std::pow(20000.0 / 20.0, normalizedX);
    freq = std::max(20.0, std::min(20000.0, freq));
    
    double normalizedY = (pos.y - bounds.top) / bounds.getHeight();
    gain = 12.0 - (normalizedY * 24.0);
    gain = std::max(-12.0, std::min(12.0, gain));
}

int EQVisualizationView::getNodeAtPosition(const CPoint& pos) {
    for (size_t i = 0; i < eqNodes.size(); ++i) {
        CPoint nodePos = frequencyToPosition(eqNodes[i].freq, eqNodes[i].gain);
        double distance = std::sqrt(std::pow(pos.x - nodePos.x, 2) + std::pow(pos.y - nodePos.y, 2));
        if (distance <= 10.0) {
            return (int)i;
        }
    }
    return -1;
}

CMouseEventResult EQVisualizationView::onMouseDown(CPoint& where, const CButtonState& buttons) {
    if (buttons & kLButton) {
        selectedNode = getNodeAtPosition(where);
        if (selectedNode >= 0) {
            eqNodes[selectedNode].selected = true;
            eqNodes[selectedNode].dragging = true;
            lastMousePos = where;
            invalid();
            return kMouseEventHandled;
        }
    }
    return kMouseEventNotHandled;
}

CMouseEventResult EQVisualizationView::onMouseMoved(CPoint& where, const CButtonState& buttons) {
    if (selectedNode >= 0 && eqNodes[selectedNode].dragging) {
        double freq, gain;
        positionToFrequency(where, freq, gain);
        
        eqNodes[selectedNode].freq = freq;
        eqNodes[selectedNode].gain = gain;
        
        invalid();
        return kMouseEventHandled;
    }
    return kMouseEventNotHandled;
}

CMouseEventResult EQVisualizationView::onMouseUp(CPoint& where, const CButtonState& buttons) {
    if (selectedNode >= 0) {
        eqNodes[selectedNode].dragging = false;
        // Keep selected for visual feedback
        invalid();
        return kMouseEventHandled;
    }
    return kMouseEventNotHandled;
}

void EQVisualizationView::updateEQData(double eq_freq[3], double eq_gain[3], double eq_q[3]) {
    if (eqNodes.size() >= 3) {
        for (int i = 0; i < 3; ++i) {
            eqNodes[i].freq = eq_freq[i];
            eqNodes[i].gain = eq_gain[i];
            eqNodes[i].q = eq_q[i];
        }
        invalid();
    }
}

void EQVisualizationView::setNodeSelected(int nodeIndex, bool selected) {
    if (nodeIndex >= 0 && nodeIndex < (int)eqNodes.size()) {
        // Deselect all first
        for (auto& node : eqNodes) {
            node.selected = false;
        }
        
        if (selected) {
            eqNodes[nodeIndex].selected = true;
            selectedNode = nodeIndex;
        } else {
            selectedNode = -1;
        }
        invalid();
    }
}

//=============================================================================
// SpectrumAnalyzerView Implementation
//=============================================================================

SpectrumAnalyzerView::SpectrumAnalyzerView(const CRect& size) 
    : CView(size), drawStyle(SPECTRUM_STYLE_LINES), enabled(true) {
    // Initialize with empty data
    spectrumMagnitudes.resize(SPECTRUM_FFT_SIZE / 2, 0.0f);
    spectrumFrequencies.resize(SPECTRUM_FFT_SIZE / 2, 0.0f);
    
    // Initialize frequency bins for display
    for (size_t i = 0; i < spectrumFrequencies.size(); ++i) {
        float freq = 20.0f * powf(20000.0f / 20.0f, (float)i / (float)(spectrumFrequencies.size() - 1));
        spectrumFrequencies[i] = freq;
    }
}

SpectrumAnalyzerView::~SpectrumAnalyzerView() {
}

void SpectrumAnalyzerView::draw(CDrawContext* context) {
    if (!enabled) {
        // Draw disabled state
        context->setFillColor(CColor(20, 20, 25, 255));
        context->drawRect(getViewSize(), kDrawFilled);
        
        context->setFontColor(CColor(100, 100, 110, 255));
        context->setFont(kSystemFont);
        CRect textRect = getViewSize();
        context->drawString("Spectrum Analyzer Disabled", textRect, kCenterText);
        return;
    }
    
    // Clear background
    context->setFillColor(MyPluginEditor::kBackgroundColor);
    context->drawRect(getViewSize(), kDrawFilled);
    
    drawGrid(context);
    drawSpectrum(context);
}

void SpectrumAnalyzerView::drawGrid(CDrawContext* context) {
    context->setLineStyle(kLineSolid);
    context->setLineWidth(1.0);
    context->setFrameColor(MyPluginEditor::kGridColor);
    
    CRect bounds = getViewSize();
    bounds.makeIntegral();
    
    // Vertical grid lines (frequency divisions) - logarithmic scale
    const float freqs[] = {50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    for (float freq : freqs) {
        if (freq >= 20.0f && freq <= 20000.0f) {
            float normalizedX = log10f(freq / 20.0f) / log10f(20000.0f / 20.0f);
            float x = bounds.left + normalizedX * bounds.getWidth();
            if (x >= bounds.left && x <= bounds.right) {
                context->drawLine(CPoint(x, bounds.top), CPoint(x, bounds.bottom));
            }
        }
    }
    
    // Horizontal grid lines (magnitude divisions) - dB scale
    const float dbs[] = {-60, -40, -20, -10, -6, -3, 0};
    for (float db : dbs) {
        float normalizedY = (db + 60.0f) / 60.0f; // Normalize -60dB to 0dB range
        float y = bounds.bottom - normalizedY * bounds.getHeight();
        if (y >= bounds.top && y <= bounds.bottom) {
            context->drawLine(CPoint(bounds.left, y), CPoint(bounds.right, y));
            
            // Highlight 0dB line
            if (db == 0.0f) {
                context->setLineWidth(2.0);
                context->drawLine(CPoint(bounds.left, y), CPoint(bounds.right, y));
                context->setLineWidth(1.0);
            }
        }
    }
}

void SpectrumAnalyzerView::drawSpectrum(CDrawContext* context) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    switch (drawStyle) {
        case SPECTRUM_STYLE_LINES:
            drawLines(context);
            break;
        case SPECTRUM_STYLE_DOTS:
            drawDots(context);
            break;
        case SPECTRUM_STYLE_BINS:
            drawBins(context);
            break;
        case SPECTRUM_STYLE_FILLS:
            drawFills(context);
            break;
        default:
            drawLines(context);
            break;
    }
}

void SpectrumAnalyzerView::drawLines(CDrawContext* context) {
    if (spectrumMagnitudes.empty()) return;
    
    context->setLineStyle(kLineSolid);
    context->setLineWidth(2.0);
    context->setFrameColor(MyPluginEditor::kAccentColor);
    
    auto path = context->createGraphicsPath();
    if (!path) return;
    
    bool firstPoint = true;
    
    for (size_t i = 0; i < spectrumMagnitudes.size(); ++i) {
        if (!isFrequencyInRange(spectrumFrequencies[i])) continue;
        
        CPoint point = frequencyToPosition(spectrumFrequencies[i], spectrumMagnitudes[i]);
        
        if (firstPoint) {
            path->beginSubpath(point);
            firstPoint = false;
        } else {
            path->addLine(point);
        }
    }
    
    context->drawGraphicsPath(path, CDrawContext::kPathStroked);
    path->forget();
}

void SpectrumAnalyzerView::drawDots(CDrawContext* context) {
    context->setFillColor(MyPluginEditor::kAccentColor);
    
    for (size_t i = 0; i < spectrumMagnitudes.size(); i += 4) { // Sample every 4th point for performance
        if (!isFrequencyInRange(spectrumFrequencies[i])) continue;
        
        CPoint point = frequencyToPosition(spectrumFrequencies[i], spectrumMagnitudes[i]);
        CRect dotRect(point.x - 1, point.y - 1, point.x + 1, point.y + 1);
        context->drawEllipse(dotRect, kDrawFilled);
    }
}

void SpectrumAnalyzerView::drawBins(CDrawContext* context) {
    context->setFillColor(CColor(120, 180, 255, 100)); // Semi-transparent accent color
    context->setFrameColor(MyPluginEditor::kAccentColor);
    context->setLineWidth(1.0);
    
    CRect bounds = getViewSize();
    
    for (size_t i = 0; i < spectrumMagnitudes.size(); i += 2) { // Sample every 2nd point
        if (!isFrequencyInRange(spectrumFrequencies[i])) continue;
        
        CPoint point = frequencyToPosition(spectrumFrequencies[i], spectrumMagnitudes[i]);
        
        // Calculate bin width
        float binWidth = bounds.getWidth() / (float)(spectrumMagnitudes.size() / 2);
        CRect binRect(point.x - binWidth/2, point.y, point.x + binWidth/2, bounds.bottom);
        
        context->drawRect(binRect, kDrawFilledAndStroked);
    }
}

void SpectrumAnalyzerView::drawFills(CDrawContext* context) {
    if (spectrumMagnitudes.empty()) return;
    
    context->setFillColor(CColor(120, 180, 255, 80)); // Semi-transparent accent color
    
    auto path = context->createGraphicsPath();
    if (!path) return;
    
    CRect bounds = getViewSize();
    bool firstPoint = true;
    
    // Start from bottom-left
    path->beginSubpath(CPoint(bounds.left, bounds.bottom));
    
    for (size_t i = 0; i < spectrumMagnitudes.size(); ++i) {
        if (!isFrequencyInRange(spectrumFrequencies[i])) continue;
        
        CPoint point = frequencyToPosition(spectrumFrequencies[i], spectrumMagnitudes[i]);
        path->addLine(point);
    }
    
    // Close to bottom-right
    path->addLine(CPoint(bounds.right, bounds.bottom));
    path->closeSubpath();
    
    context->drawGraphicsPath(path, CDrawContext::kPathFilled);
    path->forget();
}

CPoint SpectrumAnalyzerView::frequencyToPosition(float freq, float magnitude) {
    CRect bounds = getViewSize();
    
    // Logarithmic frequency scale (20Hz to 20kHz)
    float normalizedX = log10f(freq / 20.0f) / log10f(20000.0f / 20.0f);
    float x = bounds.left + normalizedX * bounds.getWidth();
    
    // Linear magnitude scale (0 to 1, representing -60dB to 0dB)
    float y = bounds.bottom - magnitude * bounds.getHeight();
    
    return CPoint(x, y);
}

bool SpectrumAnalyzerView::isFrequencyInRange(float freq) {
    return freq >= 20.0f && freq <= 20000.0f;
}

CMouseEventResult SpectrumAnalyzerView::onMouseDown(CPoint& where, const CButtonState& buttons) {
    // Could implement interaction here (e.g., style switching)
    return kMouseEventNotHandled;
}

void SpectrumAnalyzerView::updateSpectrumData(const std::vector<float>& magnitudes, const std::vector<float>& frequencies) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    if (magnitudes.size() == spectrumMagnitudes.size() && 
        frequencies.size() == spectrumFrequencies.size()) {
        spectrumMagnitudes = magnitudes;
        spectrumFrequencies = frequencies;
        invalid(); // Trigger redraw
    }
}

void SpectrumAnalyzerView::setDrawStyle(SpectrumDrawStyle style) {
    if (drawStyle != style) {
        drawStyle = style;
        invalid(); // Trigger redraw
    }
}

void SpectrumAnalyzerView::setEnabled(bool enable) {
    if (enabled != enable) {
        enabled = enable;
        invalid(); // Trigger redraw
    }
}

//=============================================================================
// MyPluginEditor Implementation
//=============================================================================

MyPluginEditor::MyPluginEditor(const clap_host_t* host, my_plugin_t* plugin)
    : frame(nullptr)
    , isCreated(false) 
    , isVisible(false)
    , currentWidth(900)  // Wider for professional layout
    , currentHeight(600) // Taller for better proportions
    , host(host)
    , pluginInstance(plugin)
    , leftPanel(nullptr)
    , rightPanel(nullptr)
    , eqPanel(nullptr)
    , spectrumPanel(nullptr)
    , eqView(nullptr)
    , spectrumView(nullptr)
    , cutoffKnob(nullptr)
    , resonanceKnob(nullptr)
    , driveKnob(nullptr)
    , outputKnob(nullptr)
    , mixSlider(nullptr)
    , bypassButton(nullptr)
    , presetMenu(nullptr)
    , brandLabel(nullptr)
    , statusLabel(nullptr)
    , spectrumStyleMenu(nullptr)
    , spectrumToggleButton(nullptr)
{
    // Initialize parameters with default values
    for (int i = 0; i < PARAM_COUNT; ++i) {
        currentParams[i] = 0.0;
    }
    currentParams[PARAM_CUTOFF] = 0.5;     // 1000Hz default
    currentParams[PARAM_RESONANCE] = 0.1;  // Low resonance
    currentParams[PARAM_MIX] = 1.0;        // Full wet
    currentParams[PARAM_SPECTRUM_ENABLED] = 1.0;  // Spectrum enabled
    currentParams[PARAM_SPECTRUM_STYLE] = 0.0;    // Lines style
}

MyPluginEditor::~MyPluginEditor() {
    destroy();
}

bool MyPluginEditor::isApiSupported(const char* api, bool isFloating) {
    // Support platform-specific APIs
#ifdef __linux__
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) {
        // For now, let's allow X11 and try to set up timer/fd support later
        // The host extensions should be available by the time we need them
        return true;
    }
#elif defined(__APPLE__)
    if (strcmp(api, CLAP_WINDOW_API_COCOA) == 0) {
        return true;
    }
#elif defined(_WIN32)
    if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0) {
        return true;
    }
#endif
    return false;
}

bool MyPluginEditor::getPreferredApi(const char** api, bool* isFloating) {
#ifdef __linux__
    *api = CLAP_WINDOW_API_X11;
#elif defined(__APPLE__)
    *api = CLAP_WINDOW_API_COCOA;
#elif defined(_WIN32)
    *api = CLAP_WINDOW_API_WIN32;
#else
    *api = CLAP_WINDOW_API_X11; // fallback
#endif
    *isFloating = false;
    return true;
}

bool MyPluginEditor::create(const char* api, bool isFloating) {
    if (isCreated) {
        std::cout << "MyPlugin GUI: Already created, returning true" << std::endl;
        return true;
    }
    
    std::cout << "MyPlugin GUI: Creating GUI with API: " << (api ? api : "null") 
              << ", isFloating: " << (isFloating ? "true" : "false") << std::endl;
    
    // Check if the API is supported for this platform
    if (!isApiSupported(api, isFloating)) {
        std::cout << "MyPlugin GUI: API not supported: " << (api ? api : "null") << std::endl;
        return false;
    }
    
    // Initialize VSTGUI if not already done
    if (!vstgui_initialized) {
        std::cout << "MyPlugin GUI: Initializing VSTGUI..." << std::endl;
        try {
#ifdef __linux__
            VSTGUI::init(nullptr);
            std::cout << "MyPlugin GUI: VSTGUI initialized for Linux" << std::endl;
#elif defined(__APPLE__)
            VSTGUI::init(CFBundleGetMainBundle());
            std::cout << "MyPlugin GUI: VSTGUI initialized for macOS" << std::endl;
#elif defined(_WIN32)
            HMODULE hModule = GetModuleHandle(nullptr);
            std::cout << "MyPlugin GUI: Module handle: " << hModule << std::endl;
            VSTGUI::init(hModule);
            std::cout << "MyPlugin GUI: VSTGUI initialized for Windows" << std::endl;
#else
            VSTGUI::init(nullptr);
            std::cout << "MyPlugin GUI: VSTGUI initialized for unknown platform" << std::endl;
#endif
            vstgui_initialized = true;
            std::cout << "MyPlugin GUI: VSTGUI initialization complete" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "MyPlugin GUI: Error initializing VSTGUI: " << e.what() << std::endl;
            return false;
        }
    } else {
        std::cout << "MyPlugin GUI: VSTGUI already initialized" << std::endl;
    }
    
    try {
        std::cout << "MyPlugin GUI: Creating CFrame..." << std::endl;
        // Create VSTGUI frame with professional size
        CRect rect(0, 0, currentWidth, currentHeight);
        frame = new CFrame(rect, nullptr);
        
        if (frame) {
            std::cout << "MyPlugin GUI: CFrame created successfully" << std::endl;
            // Set professional dark background
            frame->setBackgroundColor(kBackgroundColor);
            std::cout << "MyPlugin GUI: Background color set" << std::endl;
            createControls();
            std::cout << "MyPlugin GUI: Controls created" << std::endl;
            isCreated = true;
            std::cout << "MyPlugin GUI: Professional GUI created successfully (" 
                      << currentWidth << "x" << currentHeight << ")" << std::endl;
            return true;
        } else {
            std::cout << "MyPlugin GUI: Failed to create CFrame (returned nullptr)" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << "MyPlugin GUI: Error creating GUI: " << e.what() << std::endl;
    }
    
    return false;
}

void MyPluginEditor::destroy() {
    if (frame) {
        try {
            // VSTGUI CFrame doesn't have isOpen() method, so just close unconditionally
            std::cout << "MyPlugin GUI: Closing frame" << std::endl;
            frame->close();
            
            std::cout << "MyPlugin GUI: Releasing frame" << std::endl;
            frame->forget();
            frame = nullptr;
        } catch (const std::exception& e) {
            std::cout << "MyPlugin GUI: Exception during destroy: " << e.what() << std::endl;
            frame = nullptr; // Ensure we don't leak
        } catch (...) {
            std::cout << "MyPlugin GUI: Unknown exception during destroy" << std::endl;
            frame = nullptr; // Ensure we don't leak
        }
    }
    isCreated = false;
    isVisible = false;
}

bool MyPluginEditor::setScale(double scale) {
    // For simplicity, we'll ignore scaling for now
    return true;
}

bool MyPluginEditor::getSize(uint32_t* width, uint32_t* height) {
    if (!isCreated) {
        return false;
    }
    
    *width = currentWidth;
    *height = currentHeight;
    return true;
}

bool MyPluginEditor::canResize() {
    return false; // Fixed size for simplicity
}

bool MyPluginEditor::getResizeHints(clap_gui_resize_hints_t* hints) {
    return false; // Not resizable
}

bool MyPluginEditor::adjustSize(uint32_t* width, uint32_t* height) {
    // Fixed size, no adjustment needed
    return false;
}

bool MyPluginEditor::setSize(uint32_t width, uint32_t height) {
    if (!isCreated) {
        return false;
    }
    
    // For now, keep fixed size
    return false;
}

bool MyPluginEditor::setParent(const clap_window_t* window) {
    if (!isCreated || !frame || !window) {
        return false;
    }
    
    try {
        std::cout << "MyPlugin GUI: Setting parent window" << std::endl;
        
#ifdef __linux__
        // Embed into X11 window
        if (window->api && strcmp(window->api, CLAP_WINDOW_API_X11) == 0) {
            std::cout << "MyPlugin GUI: Request to embed in X11 parent (ID: " << window->x11 << ")" << std::endl;
            
            // Add safety check for valid window ID
            if (window->x11 == 0) {
                std::cout << "MyPlugin GUI: Invalid window ID (0)" << std::endl;
                return false;
            }
            
            // Try to open the frame and embed it in the host window
            try {
                bool result = frame->open((void*)(window->x11));
                if (result) {
                    std::cout << "MyPlugin GUI: Frame opened and embedded successfully in X11 window (ID: " << window->x11 << ")" << std::endl;
                } else {
                    std::cout << "MyPlugin GUI: Failed to open frame (VSTGUI returned false)" << std::endl;
                }
                return result;
            } catch (const std::exception& e) {
                std::cout << "MyPlugin GUI: Exception opening frame: " << e.what() << std::endl;
                return false;
            } catch (...) {
                std::cout << "MyPlugin GUI: Unknown exception opening frame" << std::endl;
                return false;
            }
        }
#elif defined(__APPLE__)
        if (window->api && strcmp(window->api, CLAP_WINDOW_API_COCOA) == 0) {
            return frame->open(window->cocoa);
        }
#elif defined(_WIN32)
        if (window->api && strcmp(window->api, CLAP_WINDOW_API_WIN32) == 0) {
            std::cout << "MyPlugin GUI: Request to embed in Windows parent (HWND: " << window->win32 << ")" << std::endl;
            
            // Add safety check for valid window handle
            if (window->win32 == NULL) {
                std::cout << "MyPlugin GUI: Invalid window handle (NULL)" << std::endl;
                return false;
            }
            
            // Verify the window handle is valid using Windows API
            if (!IsWindow((HWND)window->win32)) {
                std::cout << "MyPlugin GUI: Invalid window handle - not a valid window (HWND: " << window->win32 << ")" << std::endl;
                return false;
            }
            
            // Get additional window information for debugging
            RECT windowRect;
            if (GetWindowRect((HWND)window->win32, &windowRect)) {
                std::cout << "MyPlugin GUI: Host window rect: " << windowRect.left << "," << windowRect.top 
                          << " to " << windowRect.right << "," << windowRect.bottom << std::endl;
            } else {
                std::cout << "MyPlugin GUI: Warning: Could not get host window rect" << std::endl;
            }
            
            // Try to open the frame and embed it in the host window
            try {
                bool result = frame->open(window->win32);
                if (result) {
                    std::cout << "MyPlugin GUI: Frame opened and embedded successfully in Windows window (HWND: " << window->win32 << ")" << std::endl;
                } else {
                    std::cout << "MyPlugin GUI: Failed to open frame (VSTGUI returned false)" << std::endl;
                }
                return result;
            } catch (const std::exception& e) {
                std::cout << "MyPlugin GUI: Exception opening frame: " << e.what() << std::endl;
                return false;
            } catch (...) {
                std::cout << "MyPlugin GUI: Unknown exception opening frame" << std::endl;
                return false;
            }
        }
#endif
        std::cout << "MyPlugin GUI: Unsupported window API: " << (window->api ? window->api : "null") << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "MyPlugin GUI: Error setting parent: " << e.what() << std::endl;
    }
    
    return false;
}

bool MyPluginEditor::setTransient(const clap_window_t* window) {
    // Not used for embedded windows
    return false;
}

void MyPluginEditor::suggestTitle(const char* title) {
    // Not used for embedded windows
}

bool MyPluginEditor::show() {
    std::cout << "MyPlugin GUI: show() called, isCreated=" << isCreated << ", frame=" << (frame ? "valid" : "null") << std::endl;
    
    if (!isCreated || !frame) {
        std::cout << "MyPlugin GUI: Cannot show - not created or frame is null" << std::endl;
        return false;
    }
    
    try {
        std::cout << "MyPlugin GUI: Setting frame visible..." << std::endl;
        frame->setVisible(true);
        isVisible = true;
        std::cout << "MyPlugin GUI: Frame set to visible, isVisible=" << isVisible << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cout << "MyPlugin GUI: Error showing GUI: " << e.what() << std::endl;
    }
    
    return false;
}

bool MyPluginEditor::hide() {
    if (!isCreated || !frame) {
        return false;
    }
    
    try {
        frame->setVisible(false);
        isVisible = false;
        std::cout << "MyPlugin GUI: Hidden" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cout << "MyPlugin GUI: Error hiding GUI: " << e.what() << std::endl;
    }
    
    return false;
}

void MyPluginEditor::createControls() {
    if (!frame) {
        return;
    }
    
    // Create main layout containers
    createLeftPanel();
    createRightPanel();
    
    std::cout << "MyPlugin GUI: Professional control layout with spectrum analyzer created" << std::endl;
}

void MyPluginEditor::createBrandHeader() {
    if (!leftPanel) return;
    
    // Brand/logo area
    CRect brandRect(10, 10, 240, 50);
    brandLabel = new CTextLabel(brandRect, "Soothe Pro");
    brandLabel->setFontColor(kTextColor);
    brandLabel->setBackColor(CColor(0, 0, 0, 0)); // Transparent
    brandLabel->setFont(new CFontDesc("Arial", 18, kNormalFace));
    brandLabel->setHoriAlign(kCenterText);
    brandLabel->setStyle(brandLabel->getStyle() | CTextLabel::kNoFrame);
    leftPanel->addView(brandLabel);
    
    // Version/subtitle
    CRect subtitleRect(10, 45, 240, 65);
    auto subtitleLabel = new CTextLabel(subtitleRect, "Dynamic EQ & Resonance Control");
    subtitleLabel->setFontColor(CColor(160, 160, 170, 255));
    subtitleLabel->setBackColor(CColor(0, 0, 0, 0));
    subtitleLabel->setFont(new CFontDesc("Arial", 10, kNormalFace));
    subtitleLabel->setHoriAlign(kCenterText);
    subtitleLabel->setStyle(subtitleLabel->getStyle() | CTextLabel::kNoFrame);
    leftPanel->addView(subtitleLabel);
}

void MyPluginEditor::createLeftPanel() {
    if (!frame) return;
    
    // Left control panel (similar to Soothe2's left sidebar)
    CRect leftRect(0, 0, 250, currentHeight);
    leftPanel = new CViewContainer(leftRect);
    leftPanel->setBackgroundColor(kPanelColor);
    frame->addView(leftPanel);
    
    createBrandHeader();
    
    int yPos = 80;
    const int spacing = 20;
    const int controlHeight = 15;
    const int knobSize = 60;
    
    // Preset selection
    CRect presetRect(15, yPos, 235, yPos + 25);
    presetMenu = new COptionMenu(presetRect, nullptr, 0);
    presetMenu->addEntry("Default");
    presetMenu->addEntry("Warm Analog");
    presetMenu->addEntry("Modern Digital");
    presetMenu->addEntry("Vintage Character");
    presetMenu->addEntry("Custom Settings");
    presetMenu->setCurrent(0);
    styleControl(presetMenu);
    leftPanel->addView(presetMenu);
    yPos += 40;
    
    // Main controls section
    CRect sectionRect(10, yPos, 240, yPos + 20);
    auto sectionLabel = new CTextLabel(sectionRect, "MAIN CONTROLS");
    sectionLabel->setFontColor(kAccentColor);
    sectionLabel->setBackColor(CColor(0, 0, 0, 0));
    sectionLabel->setFont(new CFontDesc("Arial", 12, kBoldFace));
    sectionLabel->setHoriAlign(kLeftText);
    sectionLabel->setStyle(sectionLabel->getStyle() | CTextLabel::kNoFrame);
    leftPanel->addView(sectionLabel);
    yPos += 35;
    
    // Large main knob (Cutoff)
    CRect cutoffKnobRect(95, yPos, 95 + 80, yPos + 80);
    cutoffKnob = new CKnob(cutoffKnobRect, nullptr, 0 /* PARAM_CUTOFF */, nullptr, nullptr);
    cutoffKnob->setDefaultValue(0.5);
    cutoffKnob->setValue(0.5);
    styleControl(cutoffKnob);
    leftPanel->addView(cutoffKnob);
    
    CRect cutoffLabelRect(80, yPos + 85, 160, yPos + 100);
    auto cutoffLabel = new CTextLabel(cutoffLabelRect, "CUTOFF");
    cutoffLabel->setFontColor(kTextColor);
    cutoffLabel->setBackColor(CColor(0, 0, 0, 0));
    cutoffLabel->setFont(new CFontDesc("Arial", 11, kBoldFace));
    cutoffLabel->setHoriAlign(kCenterText);
    cutoffLabel->setStyle(cutoffLabel->getStyle() | CTextLabel::kNoFrame);
    leftPanel->addView(cutoffLabel);
    yPos += 120;
    
    // Secondary knobs row
    int knobX = 30;
    const int knobSpacing = 70;
    
    // Resonance knob
    CRect resKnobRect(knobX, yPos, knobX + knobSize, yPos + knobSize);
    resonanceKnob = new CKnob(resKnobRect, nullptr, 1 /* PARAM_RESONANCE */, nullptr, nullptr);
    resonanceKnob->setDefaultValue(0.3);
    resonanceKnob->setValue(0.3);
    styleControl(resonanceKnob);
    leftPanel->addView(resonanceKnob);
    
    CRect resLabelRect(knobX - 10, yPos + knobSize + 5, knobX + knobSize + 10, yPos + knobSize + 20);
    auto resLabel = new CTextLabel(resLabelRect, "RESO");
    resLabel->setFontColor(kTextColor);
    resLabel->setBackColor(CColor(0, 0, 0, 0));
    resLabel->setFont(new CFontDesc("Arial", 9, kNormalFace));
    resLabel->setHoriAlign(kCenterText);
    resLabel->setStyle(resLabel->getStyle() | CTextLabel::kNoFrame);
    leftPanel->addView(resLabel);
    
    // Drive knob
    knobX += knobSpacing;
    CRect driveKnobRect(knobX, yPos, knobX + knobSize, yPos + knobSize);
    driveKnob = new CKnob(driveKnobRect, nullptr, 2 /* PARAM_DRIVE */, nullptr, nullptr);
    driveKnob->setDefaultValue(0.0);
    driveKnob->setValue(0.0);
    styleControl(driveKnob);
    leftPanel->addView(driveKnob);
    
    CRect driveLabelRect(knobX - 10, yPos + knobSize + 5, knobX + knobSize + 10, yPos + knobSize + 20);
    auto driveLabel = new CTextLabel(driveLabelRect, "DRIVE");
    driveLabel->setFontColor(kTextColor);
    driveLabel->setBackColor(CColor(0, 0, 0, 0));
    driveLabel->setFont(new CFontDesc("Arial", 9, kNormalFace));
    driveLabel->setHoriAlign(kCenterText);
    driveLabel->setStyle(driveLabel->getStyle() | CTextLabel::kNoFrame);
    leftPanel->addView(driveLabel);
    
    yPos += knobSize + 40;
    
    // Output section
    CRect outputSectionRect(10, yPos, 240, yPos + 20);
    auto outputSectionLabel = new CTextLabel(outputSectionRect, "OUTPUT");
    outputSectionLabel->setFontColor(kAccentColor);
    outputSectionLabel->setBackColor(CColor(0, 0, 0, 0));
    outputSectionLabel->setFont(new CFontDesc("Arial", 12, kBoldFace));
    outputSectionLabel->setHoriAlign(kLeftText);
    outputSectionLabel->setStyle(outputSectionLabel->getStyle() | CTextLabel::kNoFrame);
    leftPanel->addView(outputSectionLabel);
    yPos += 30;
    
    // Mix slider (vertical)
    CRect mixSliderRect(50, yPos, 70, yPos + 80);
    mixSlider = new CVerticalSlider(mixSliderRect, nullptr, 4 /* PARAM_MIX */, 0, 100, nullptr, nullptr);
    mixSlider->setValue(100.0);
    styleControl(mixSlider);
    leftPanel->addView(mixSlider);
    
    CRect mixLabelRect(40, yPos + 85, 80, yPos + 100);
    auto mixLabel = new CTextLabel(mixLabelRect, "MIX");
    mixLabel->setFontColor(kTextColor);
    mixLabel->setBackColor(CColor(0, 0, 0, 0));
    mixLabel->setFont(new CFontDesc("Arial", 9, kNormalFace));
    mixLabel->setHoriAlign(kCenterText);
    mixLabel->setStyle(mixLabel->getStyle() | CTextLabel::kNoFrame);
    leftPanel->addView(mixLabel);
    
    // Output gain knob
    CRect outputKnobRect(120, yPos + 10, 120 + knobSize, yPos + 10 + knobSize);
    outputKnob = new CKnob(outputKnobRect, nullptr, 3 /* PARAM_OUTPUT */, nullptr, nullptr);
    outputKnob->setDefaultValue(0.5);
    outputKnob->setValue(0.5);
    styleControl(outputKnob);
    leftPanel->addView(outputKnob);
    
    CRect outputLabelRect(110, yPos + knobSize + 15, 190, yPos + knobSize + 30);
    auto outputLabel = new CTextLabel(outputLabelRect, "OUTPUT");
    outputLabel->setFontColor(kTextColor);
    outputLabel->setBackColor(CColor(0, 0, 0, 0));
    outputLabel->setFont(new CFontDesc("Arial", 9, kNormalFace));
    outputLabel->setHoriAlign(kCenterText);
    outputLabel->setStyle(outputLabel->getStyle() | CTextLabel::kNoFrame);
    leftPanel->addView(outputLabel);
    
    yPos += 120;
    
    // Bypass button
    CRect bypassRect(75, yPos, 175, yPos + 30);
    bypassButton = new CTextButton(bypassRect, nullptr, 5 /* PARAM_BYPASS */, "BYPASS");
    styleControl(bypassButton);
    leftPanel->addView(bypassButton);
    
    yPos += 50;
    
    // Status display
    CRect statusRect(15, currentHeight - 60, 235, currentHeight - 20);
    statusLabel = new CTextLabel(statusRect, "Status: Active\\nCPU: 12% | Latency: 0ms");
    statusLabel->setFontColor(CColor(140, 140, 150, 255));
    statusLabel->setBackColor(CColor(0, 0, 0, 0));
    statusLabel->setFont(new CFontDesc("Arial", 8, kNormalFace));
    statusLabel->setHoriAlign(kCenterText);
    statusLabel->setStyle(statusLabel->getStyle() | CTextLabel::kNoFrame);
    leftPanel->addView(statusLabel);
}

void MyPluginEditor::createRightPanel() {
    if (!frame) return;
    
    // Right panel for both EQ and spectrum visualization
    CRect rightRect(250, 0, currentWidth, currentHeight);
    rightPanel = new CViewContainer(rightRect);
    rightPanel->setBackgroundColor(kBackgroundColor);
    frame->addView(rightPanel);
    
    // Create EQ and spectrum sub-panels
    createEQPanel();
    createSpectrumPanel();
}

void MyPluginEditor::createEQPanel() {
    if (!rightPanel) return;
    
    // EQ panel takes the top half of the right panel
    CRect eqPanelRect(0, 0, rightPanel->getViewSize().getWidth(), rightPanel->getViewSize().getHeight() / 2);
    eqPanel = new CViewContainer(eqPanelRect);
    eqPanel->setBackgroundColor(kBackgroundColor);
    rightPanel->addView(eqPanel);
    
    // EQ section header
    CRect eqHeaderRect(20, 10, eqPanelRect.getWidth() - 20, 35);
    auto eqHeaderLabel = new CTextLabel(eqHeaderRect, "DYNAMIC EQ VISUALIZATION");
    eqHeaderLabel->setFontColor(kAccentColor);
    eqHeaderLabel->setBackColor(CColor(0, 0, 0, 0));
    eqHeaderLabel->setFont(new CFontDesc("Arial", 12, kBoldFace));
    eqHeaderLabel->setHoriAlign(kLeftText);
    eqHeaderLabel->setStyle(eqHeaderLabel->getStyle() | CTextLabel::kNoFrame);
    eqPanel->addView(eqHeaderLabel);
    
    // Main EQ visualization area
    CRect eqRect(20, 40, eqPanelRect.getWidth() - 20, eqPanelRect.getHeight() - 10);
    eqView = new EQVisualizationView(eqRect);
    eqPanel->addView(eqView);
    
    // Update EQ view with current parameters
    double eq_freq[3] = {200.0, 1000.0, 5000.0};
    double eq_gain[3] = {0.0, 0.0, 0.0};
    double eq_q[3] = {1.0, 1.0, 1.0};
    eqView->updateEQData(eq_freq, eq_gain, eq_q);
}

void MyPluginEditor::createSpectrumPanel() {
    if (!rightPanel) return;
    
    // Spectrum panel takes the bottom half of the right panel
    CRect spectrumPanelRect(0, rightPanel->getViewSize().getHeight() / 2, 
                           rightPanel->getViewSize().getWidth(), rightPanel->getViewSize().getHeight());
    spectrumPanel = new CViewContainer(spectrumPanelRect);
    spectrumPanel->setBackgroundColor(kBackgroundColor);
    rightPanel->addView(spectrumPanel);
    
    // Spectrum analyzer header with controls
    CRect headerRect(20, 10, spectrumPanelRect.getWidth() - 20, 35);
    auto spectrumHeaderLabel = new CTextLabel(headerRect, "SPECTRUM ANALYZER");
    spectrumHeaderLabel->setFontColor(kAccentColor);
    spectrumHeaderLabel->setBackColor(CColor(0, 0, 0, 0));
    spectrumHeaderLabel->setFont(new CFontDesc("Arial", 12, kBoldFace));
    spectrumHeaderLabel->setHoriAlign(kLeftText);  
    spectrumHeaderLabel->setStyle(spectrumHeaderLabel->getStyle() | CTextLabel::kNoFrame);
    spectrumPanel->addView(spectrumHeaderLabel);
    
    // Style selector
    CRect styleMenuRect(spectrumPanelRect.getWidth() - 150, 12, spectrumPanelRect.getWidth() - 80, 32);
    spectrumStyleMenu = new COptionMenu(styleMenuRect, nullptr, PARAM_SPECTRUM_STYLE);
    spectrumStyleMenu->addEntry("Lines");
    spectrumStyleMenu->addEntry("Dots");
    spectrumStyleMenu->addEntry("Bins");
    spectrumStyleMenu->addEntry("Fills");
    spectrumStyleMenu->setCurrent(0);
    styleControl(spectrumStyleMenu);
    spectrumPanel->addView(spectrumStyleMenu);
    
    // Toggle button
    CRect toggleRect(spectrumPanelRect.getWidth() - 70, 12, spectrumPanelRect.getWidth() - 20, 32);
    spectrumToggleButton = new CTextButton(toggleRect, nullptr, PARAM_SPECTRUM_ENABLED, "ON");
    styleControl(spectrumToggleButton);
    spectrumPanel->addView(spectrumToggleButton);
    
    // Main spectrum visualization area
    CRect spectrumRect(20, 40, spectrumPanelRect.getWidth() - 20, spectrumPanelRect.getHeight() - 10);
    spectrumView = new SpectrumAnalyzerView(spectrumRect);
    spectrumPanel->addView(spectrumView);
}

void MyPluginEditor::styleControl(CView* control) {
    if (!control) return;
    
    // Apply professional styling to controls
    if (auto knob = dynamic_cast<CKnob*>(control)) {
        knob->setColorShadowHandle(CColor(40, 40, 50, 255));
        knob->setColorHandle(kAccentColor);
    } else if (auto slider = dynamic_cast<CSlider*>(control)) {
        slider->setFrameColor(CColor(80, 80, 90, 255));
        slider->setBackColor(CColor(40, 40, 50, 255));
        slider->setValueColor(kAccentColor);
    } else if (auto button = dynamic_cast<CTextButton*>(control)) {
        button->setFrameColor(CColor(80, 80, 90, 255));
        // button->setBackColor(kPanelColor);  // CTextButton may not have setBackColor
        button->setTextColor(kTextColor);
    } else if (auto menu = dynamic_cast<COptionMenu*>(control)) {
        menu->setFrameColor(CColor(80, 80, 90, 255));
        menu->setBackColor(kPanelColor);
        menu->setFontColor(kTextColor);
    }
}

void MyPluginEditor::updateParameter(int paramId, double value) {
    // Update internal parameter storage
    if (paramId >= 0 && paramId < PARAM_COUNT) {
        currentParams[paramId] = value;
    }
    
    // Update GUI controls
    switch (paramId) {
        case PARAM_CUTOFF:
            if (cutoffKnob) cutoffKnob->setValue(value);
            break;
        case PARAM_RESONANCE:
            if (resonanceKnob) resonanceKnob->setValue(value);
            break;
        case PARAM_DRIVE:
            if (driveKnob) driveKnob->setValue(value);
            break;
        case PARAM_OUTPUT:
            if (outputKnob) outputKnob->setValue(value);
            break;
        case PARAM_MIX:
            if (mixSlider) mixSlider->setValue(value * 100.0);
            break;
        case PARAM_BYPASS:
            if (bypassButton) {
                bypassButton->setValue(value > 0.5 ? 1.0 : 0.0);
            }
            break;
        case PARAM_SPECTRUM_ENABLED:
            if (spectrumToggleButton) {
                spectrumToggleButton->setValue(value > 0.5 ? 1.0 : 0.0);
                spectrumToggleButton->setTitle(value > 0.5 ? "ON" : "OFF");
            }
            if (spectrumView) {
                spectrumView->setEnabled(value > 0.5);
            }
            if (pluginInstance) {
                set_plugin_spectrum_enabled(pluginInstance, value > 0.5);
            }
            break;
        case PARAM_SPECTRUM_STYLE:
            if (spectrumStyleMenu) {
                spectrumStyleMenu->setCurrent((int)value);
            }
            if (spectrumView) {
                spectrumView->setDrawStyle((SpectrumDrawStyle)(int)value);
            }
            if (pluginInstance) {
                set_plugin_spectrum_style(pluginInstance, (SpectrumDrawStyle)(int)value);
            }
            break;
    }
    
    // Update EQ visualization
    if (eqView) {
        double eq_freq[3] = {200.0, 1000.0, 5000.0};
        double eq_gain[3] = {currentParams[PARAM_EQ_GAIN1], currentParams[PARAM_EQ_GAIN2], currentParams[PARAM_EQ_GAIN3]};
        double eq_q[3] = {currentParams[PARAM_EQ_Q1], currentParams[PARAM_EQ_Q2], currentParams[PARAM_EQ_Q3]};
        eqView->updateEQData(eq_freq, eq_gain, eq_q);
    }
}

void MyPluginEditor::updateSpectrumDisplay() {
    // Update the spectrum display with new data from the plugin
    if (spectrumView && pluginInstance) {
        std::vector<float> magnitudes, frequencies;
        
        if (get_plugin_spectrum_data(pluginInstance, magnitudes, frequencies)) {
            spectrumView->updateSpectrumData(magnitudes, frequencies);
        }
    }
}

void MyPluginEditor::onParameterChanged(int paramId, double value) {
    updateParameter(paramId, value);
    
    // Here you would typically notify the host about parameter changes
    // This would require implementing the CLAP parameter extension
    std::cout << "Parameter " << paramId << " changed to " << value << std::endl;
}