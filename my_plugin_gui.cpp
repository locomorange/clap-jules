#include "my_plugin_gui.h"
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
#include <iostream>

#ifdef __linux__
#include <vstgui/lib/platform/platform_x11.h>
#include <vstgui/lib/platform/linux/x11platform.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

// Static initialization flag
static bool vstgui_initialized = false;

MyPluginEditor::MyPluginEditor(const clap_host_t* host)
    : frame(nullptr)
    , isCreated(false) 
    , isVisible(false)
    , currentWidth(800)  // Wider for equalizer layout
    , currentHeight(600) // Taller for spectrum analyzer
    , host(host)
{
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
        return true;
    }
    
    // Check if the API is supported for this platform
    if (!isApiSupported(api, isFloating)) {
        return false;
    }
    
    // Initialize VSTGUI if not already done
    if (!vstgui_initialized) {
        try {
#ifdef __linux__
            VSTGUI::init(nullptr);
#elif defined(__APPLE__)
            VSTGUI::init(CFBundleGetMainBundle());
#elif defined(_WIN32)
            VSTGUI::init(GetModuleHandle(nullptr));
#else
            VSTGUI::init(nullptr);
#endif
            vstgui_initialized = true;
            std::cout << "MyPlugin GUI: VSTGUI initialized" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "MyPlugin GUI: Error initializing VSTGUI: " << e.what() << std::endl;
            return false;
        }
    }
    
    try {
        // Create VSTGUI frame
        CRect rect(0, 0, currentWidth, currentHeight);
        frame = new CFrame(rect, nullptr);
        
        if (frame) {
            // Set a darker background for professional look
            frame->setBackgroundColor(CColor(30, 30, 35, 255)); // Dark gray background
            createControls();
            isCreated = true;
            std::cout << "MyPlugin GUI: Created successfully" << std::endl;
            return true;
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
            
            // For now, on Linux, we'll return true to indicate we can handle the parent
            // but won't actually call frame->open() to prevent crashes in headless environments
            // TODO: Implement proper VSTGUI event loop integration like clap-saw-demo
            std::cout << "MyPlugin GUI: Simulating successful parent embedding (GUI would embed in real X11 environment)" << std::endl;
            return true;
            
            /* DISABLED FOR NOW - causes crashes without proper event loop integration
            // Try to open the frame - this is where crashes can occur
            // We need proper VSTGUI event loop integration for this to work reliably
            try {
                bool result = frame->open((void*)(window->x11));
                if (result) {
                    std::cout << "MyPlugin GUI: Frame opened successfully" << std::endl;
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
            */
        }
#elif defined(__APPLE__)
        if (window->api && strcmp(window->api, CLAP_WINDOW_API_COCOA) == 0) {
            return frame->open(window->cocoa);
        }
#elif defined(_WIN32)
        if (window->api && strcmp(window->api, CLAP_WINDOW_API_WIN32) == 0) {
            return frame->open(window->win32);
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
    if (!isCreated || !frame) {
        return false;
    }
    
    try {
        frame->setVisible(true);
        isVisible = true;
        std::cout << "MyPlugin GUI: Shown" << std::endl;
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
    
    // Set a darker background for professional look
    frame->setBackgroundColor(CColor(30, 30, 35, 255)); // Dark gray background
    
    // === TOP SECTION: Utility buttons ===
    CRect topSectionRect(0, 0, currentWidth, 50);
    
    // Undo button
    CRect undoButtonRect(20, 15, 80, 35);
    auto undoButton = new CTextButton(undoButtonRect, nullptr, 0, "Undo");
    undoButton->setFrameColor(CColor(80, 80, 80, 255));
    undoButton->setBackColor(CColor(60, 60, 65, 255));
    undoButton->setTextColor(CColor(220, 220, 220, 255));
    frame->addView(undoButton);
    
    // Redo button
    CRect redoButtonRect(90, 15, 150, 35);
    auto redoButton = new CTextButton(redoButtonRect, nullptr, 0, "Redo");
    redoButton->setFrameColor(CColor(80, 80, 80, 255));
    redoButton->setBackColor(CColor(60, 60, 65, 255));
    redoButton->setTextColor(CColor(220, 220, 220, 255));
    frame->addView(redoButton);
    
    // A/B comparison buttons
    CRect aButtonRect(currentWidth - 150, 15, currentWidth - 110, 35);
    auto aButton = new CTextButton(aButtonRect, nullptr, 0, "A");
    aButton->setFrameColor(CColor(80, 80, 80, 255));
    aButton->setBackColor(CColor(120, 60, 60, 255)); // Red tint for A
    aButton->setTextColor(CColor(255, 255, 255, 255));
    frame->addView(aButton);
    
    CRect bButtonRect(currentWidth - 100, 15, currentWidth - 60, 35);
    auto bButton = new CTextButton(bButtonRect, nullptr, 0, "B");
    bButton->setFrameColor(CColor(80, 80, 80, 255));
    bButton->setBackColor(CColor(60, 60, 65, 255));
    bButton->setTextColor(CColor(220, 220, 220, 255));
    frame->addView(bButton);
    
    // Title
    CRect titleRect(160, 15, currentWidth - 160, 35);
    auto titleLabel = new CTextLabel(titleRect, "CLAP Equalizer - VSTGUI Interface");
    titleLabel->setFontColor(CColor(220, 220, 220, 255));
    titleLabel->setBackColor(CColor(0, 0, 0, 0)); // Transparent
    titleLabel->setHoriAlign(kCenterText);
    frame->addView(titleLabel);
    
    // === LEFT SECTION: Main Controls ===
    int leftWidth = currentWidth * 0.4; // 40% of width for controls
    int controlY = 70;
    int controlSpacing = 80;
    
    // Depth control (large knob)
    CRect depthKnobRect(30, controlY, 110, controlY + 80);
    auto depthKnob = new CKnob(depthKnobRect, this, PARAM_DEPTH, nullptr, nullptr);
    depthKnob->setDefaultValue(0.5f);
    depthKnob->setValue(0.5f);
    depthKnob->setColorShadowHandle(CColor(100, 100, 100, 255));
    depthKnob->setColorHandle(CColor(255, 120, 60, 255)); // Orange handle
    frame->addView(depthKnob);
    
    CRect depthLabelRect(20, controlY + 85, 120, controlY + 105);
    auto depthLabel = new CTextLabel(depthLabelRect, "DEPTH");
    depthLabel->setFontColor(CColor(255, 120, 60, 255)); // Match knob color
    depthLabel->setBackColor(CColor(0, 0, 0, 0));
    depthLabel->setHoriAlign(kCenterText);
    frame->addView(depthLabel);
    
    controlY += controlSpacing;
    
    // Sharpness control (large knob)
    CRect sharpnessKnobRect(30, controlY, 110, controlY + 80);
    auto sharpnessKnob = new CKnob(sharpnessKnobRect, this, PARAM_SHARPNESS, nullptr, nullptr);
    sharpnessKnob->setDefaultValue(0.3f);
    sharpnessKnob->setValue(0.3f);
    sharpnessKnob->setColorShadowHandle(CColor(100, 100, 100, 255));
    sharpnessKnob->setColorHandle(CColor(60, 180, 255, 255)); // Blue handle
    frame->addView(sharpnessKnob);
    
    CRect sharpnessLabelRect(20, controlY + 85, 120, controlY + 105);
    auto sharpnessLabel = new CTextLabel(sharpnessLabelRect, "SHARPNESS");
    sharpnessLabel->setFontColor(CColor(60, 180, 255, 255)); // Match knob color
    sharpnessLabel->setBackColor(CColor(0, 0, 0, 0));
    sharpnessLabel->setHoriAlign(kCenterText);
    frame->addView(sharpnessLabel);
    
    controlY += controlSpacing;
    
    // Selectivity control (large knob)
    CRect selectivityKnobRect(30, controlY, 110, controlY + 80);
    auto selectivityKnob = new CKnob(selectivityKnobRect, this, PARAM_SELECTIVITY, nullptr, nullptr);
    selectivityKnob->setDefaultValue(0.7f);
    selectivityKnob->setValue(0.7f);
    selectivityKnob->setColorShadowHandle(CColor(100, 100, 100, 255));
    selectivityKnob->setColorHandle(CColor(120, 255, 120, 255)); // Green handle
    frame->addView(selectivityKnob);
    
    CRect selectivityLabelRect(20, controlY + 85, 120, controlY + 105);
    auto selectivityLabel = new CTextLabel(selectivityLabelRect, "SELECTIVITY");
    selectivityLabel->setFontColor(CColor(120, 255, 120, 255)); // Match knob color
    selectivityLabel->setBackColor(CColor(0, 0, 0, 0));
    selectivityLabel->setHoriAlign(kCenterText);
    frame->addView(selectivityLabel);
    
    controlY += controlSpacing;
    
    // Mode selector (option menu)
    CRect modeMenuRect(30, controlY, 120, controlY + 25);
    auto modeMenu = new COptionMenu(modeMenuRect, this, PARAM_MODE);
    modeMenu->addEntry("Gentle");
    modeMenu->addEntry("Moderate");
    modeMenu->addEntry("Aggressive");
    modeMenu->addEntry("Extreme");
    modeMenu->setCurrent(0);
    modeMenu->setFrameColor(CColor(80, 80, 80, 255));
    modeMenu->setBackColor(CColor(50, 50, 55, 255));
    modeMenu->setFontColor(CColor(220, 220, 220, 255));
    frame->addView(modeMenu);
    
    CRect modeLabelRect(20, controlY + 30, 120, controlY + 50);
    auto modeLabel = new CTextLabel(modeLabelRect, "MODE");
    modeLabel->setFontColor(CColor(220, 220, 220, 255));
    modeLabel->setBackColor(CColor(0, 0, 0, 0));
    modeLabel->setHoriAlign(kCenterText);
    frame->addView(modeLabel);
    
    controlY += controlSpacing - 20;
    
    // Balance slider (horizontal)
    CRect balanceSliderRect(30, controlY, 120, controlY + 20);
    auto balanceSlider = new CHorizontalSlider(balanceSliderRect, this, PARAM_BALANCE, -1.0, 1.0, nullptr, nullptr);
    balanceSlider->setValue(0.0f);
    balanceSlider->setFrameColor(CColor(80, 80, 80, 255));
    balanceSlider->setBackColor(CColor(40, 40, 45, 255));
    balanceSlider->setValueColor(CColor(255, 255, 120, 255)); // Yellow slider
    frame->addView(balanceSlider);
    
    CRect balanceLabelRect(20, controlY + 25, 120, controlY + 45);
    auto balanceLabel = new CTextLabel(balanceLabelRect, "BALANCE");
    balanceLabel->setFontColor(CColor(255, 255, 120, 255)); // Match slider color
    balanceLabel->setBackColor(CColor(0, 0, 0, 0));
    balanceLabel->setHoriAlign(kCenterText);
    frame->addView(balanceLabel);
    
    // === RIGHT SECTION: Spectrum Analyzer and EQ Curve ===
    int rightX = leftWidth + 20;
    int rightWidth = currentWidth - rightX - 20;
    int spectrumY = 70;
    int spectrumHeight = currentHeight - spectrumY - 30;
    
    // Spectrum analyzer background
    CRect spectrumRect(rightX, spectrumY, rightX + rightWidth, spectrumY + spectrumHeight);
    auto spectrumView = new CView(spectrumRect);
    spectrumView->setBackgroundColor(CColor(15, 15, 20, 255)); // Very dark background
    frame->addView(spectrumView);
    
    // Grid lines for spectrum (placeholder - would be drawn with custom view)
    for (int i = 1; i < 10; i++) {
        int gridY = spectrumY + (spectrumHeight * i / 10);
        CRect gridLineRect(rightX, gridY, rightX + rightWidth, gridY + 1);
        auto gridLine = new CView(gridLineRect);
        gridLine->setBackgroundColor(CColor(40, 40, 45, 100)); // Semi-transparent grid
        frame->addView(gridLine);
    }
    
    for (int i = 1; i < 8; i++) {
        int gridX = rightX + (rightWidth * i / 8);
        CRect gridLineRect(gridX, spectrumY, gridX + 1, spectrumY + spectrumHeight);
        auto gridLine = new CView(gridLineRect);
        gridLine->setBackgroundColor(CColor(40, 40, 45, 100)); // Semi-transparent grid
        frame->addView(gridLine);
    }
    
    // EQ curve control points (placeholder - draggable points would be implemented with custom control)
    int numPoints = 5;
    for (int i = 0; i < numPoints; i++) {
        int pointX = rightX + (rightWidth * (i + 1) / (numPoints + 1));
        int pointY = spectrumY + spectrumHeight / 2 + (i % 2 ? -30 : 30); // Alternate high/low
        
        CRect pointRect(pointX - 6, pointY - 6, pointX + 6, pointY + 6);
        auto eqPoint = new CView(pointRect);
        eqPoint->setBackgroundColor(CColor(255, 120, 60, 255)); // Orange EQ points
        frame->addView(eqPoint);
    }
    
    // Frequency labels at bottom
    const char* freqLabels[] = {"20Hz", "100Hz", "1kHz", "5kHz", "20kHz"};
    for (int i = 0; i < 5; i++) {
        int labelX = rightX + (rightWidth * (i + 1) / 6);
        CRect freqLabelRect(labelX - 25, spectrumY + spectrumHeight + 5, labelX + 25, spectrumY + spectrumHeight + 20);
        auto freqLabel = new CTextLabel(freqLabelRect, freqLabels[i]);
        freqLabel->setFontColor(CColor(160, 160, 160, 255));
        freqLabel->setBackColor(CColor(0, 0, 0, 0));
        freqLabel->setHoriAlign(kCenterText);
        frame->addView(freqLabel);
    }
    
    // dB labels on left side
    const char* dbLabels[] = {"+12", "0", "-12", "-24", "-36"};
    for (int i = 0; i < 5; i++) {
        int labelY = spectrumY + (spectrumHeight * i / 4);
        CRect dbLabelRect(rightX - 35, labelY - 8, rightX - 5, labelY + 8);
        auto dbLabel = new CTextLabel(dbLabelRect, dbLabels[i]);
        dbLabel->setFontColor(CColor(160, 160, 160, 255));
        dbLabel->setBackColor(CColor(0, 0, 0, 0));
        dbLabel->setHoriAlign(kRightText);
        frame->addView(dbLabel);
    }
    
    // Spectrum analyzer title
    CRect spectrumTitleRect(rightX, spectrumY - 20, rightX + rightWidth, spectrumY);
    auto spectrumTitle = new CTextLabel(spectrumTitleRect, "SPECTRUM ANALYZER & EQ CURVE");
    spectrumTitle->setFontColor(CColor(200, 200, 200, 255));
    spectrumTitle->setBackColor(CColor(0, 0, 0, 0));
    spectrumTitle->setHoriAlign(kCenterText);
    frame->addView(spectrumTitle);
}

void MyPluginEditor::valueChanged(CControl* pControl) {
    if (!pControl) return;
    
    clap_id paramId = static_cast<clap_id>(pControl->getTag());
    double value = pControl->getValue();
    
    // Normalize values based on parameter ranges
    switch (paramId) {
        case PARAM_DEPTH:
        case PARAM_SHARPNESS:  
        case PARAM_SELECTIVITY:
            // These are already 0-1 range
            break;
        case PARAM_MODE:
            // Mode is 0-3, but COptionMenu returns current selection index
            value = static_cast<double>(static_cast<COptionMenu*>(pControl)->getCurrent());
            break;
        case PARAM_BALANCE:
            // Balance is -1 to +1, slider value is already in this range
            break;
        default:
            return; // Unknown parameter
    }
    
    notifyParameterChange(paramId, value);
}

void MyPluginEditor::updateParameter(clap_id paramId, double value) {
    if (!frame) return;
    
    // Find and update the corresponding control
    // This would require storing references to controls or using tags to find them
    // For now, this is a placeholder for the interface
}

void MyPluginEditor::notifyParameterChange(clap_id paramId, double value) {
    if (!host) return;
    
    // Notify the host of parameter changes
    const clap_host_params_t* host_params = 
        static_cast<const clap_host_params_t*>(host->get_extension(host, CLAP_EXT_PARAMS));
    
    if (host_params && host_params->request_flush) {
        host_params->request_flush(host);
    }
}