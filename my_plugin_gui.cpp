#include "my_plugin_gui.h"
#include <vstgui/lib/controls/cknob.h>
#include <vstgui/lib/controls/cslider.h>
#include <vstgui/lib/controls/ctextlabel.h>
#include <vstgui/lib/platform/iplatformframecallback.h>
#include <vstgui/lib/vstguiinit.h>
#include <vstgui/vstgui.h>
#include <iostream>

MyPluginEditor::MyPluginEditor()
    : frame(nullptr)
    , isCreated(false) 
    , isVisible(false)
    , currentWidth(400)
    , currentHeight(300)
    , parentWindowHandle(nullptr)
{
}

MyPluginEditor::~MyPluginEditor() {
    destroy();
}

bool MyPluginEditor::isApiSupported(const char* api, bool isFloating) {
    // Support platform-specific APIs
#ifdef __linux__
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) {
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
    
    // Check if the API is supported for this platform
    if (!isApiSupported(api, isFloating)) {
        std::cout << "MyPlugin GUI: API not supported: " << api << std::endl;
        return false;
    }
    
    std::cout << "MyPlugin GUI: Creating frame..." << std::endl;
    
    try {
        // Create VSTGUI frame with proper size
        CRect rect(0, 0, currentWidth, currentHeight);
        std::cout << "MyPlugin GUI: Creating CFrame with rect " << rect.left << "," << rect.top << " " << rect.getWidth() << "x" << rect.getHeight() << std::endl;
        
        frame = new CFrame(rect, this);
        
        if (frame) {
            std::cout << "MyPlugin GUI: Frame created, adding controls..." << std::endl;
            createControls();
            isCreated = true;
            std::cout << "MyPlugin GUI: Created frame successfully" << std::endl;
            return true;
        } else {
            std::cout << "MyPlugin GUI: Failed to create CFrame" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << "MyPlugin GUI: Exception creating GUI: " << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "MyPlugin GUI: Unknown exception creating GUI" << std::endl;
    }
    
    return false;
}

void MyPluginEditor::destroy() {
    if (frame) {
        // Close the frame properly first
        if (frame->isAttached()) {
            frame->close();
        }
        frame->forget();
        frame = nullptr;
    }
    isCreated = false;
    isVisible = false;
    parentWindowHandle = nullptr;
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
        std::cout << "MyPlugin GUI: setParent failed - invalid state" << std::endl;
        return false;
    }
    
    try {
        // Handle X11 window embedding
        if (window->api && strcmp(window->api, CLAP_WINDOW_API_X11) == 0) {
            // Store the parent window handle
            parentWindowHandle = (void*)(uintptr_t)window->x11;
            
            std::cout << "MyPlugin GUI: Embedding into X11 window ID: " << window->x11 << std::endl;
            
            // Validate that we have a reasonable window ID (not zero and not obviously invalid)
            if (window->x11 == 0) {
                std::cout << "MyPlugin GUI: Invalid X11 window ID (0)" << std::endl;
                return false;
            }
            
            // Open the frame with the X11 parent window
            // Note: This may fail if the window ID is not valid, but should not crash
            try {
                if (frame->open(parentWindowHandle, PlatformType::kX11EmbedWindowID)) {
                    std::cout << "MyPlugin GUI: Successfully embedded into X11 window" << std::endl;
                    return true;
                } else {
                    std::cout << "MyPlugin GUI: Failed to open frame with X11 parent (window may not exist)" << std::endl;
                    return false;
                }
            } catch (const std::exception& e) {
                std::cout << "MyPlugin GUI: Exception during frame open: " << e.what() << std::endl;
                return false;
            } catch (...) {
                std::cout << "MyPlugin GUI: Unknown exception during frame open" << std::endl;
                return false;
            }
        }
    }
    catch (const std::exception& e) {
        std::cout << "MyPlugin GUI: Error setting parent: " << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "MyPlugin GUI: Unknown error setting parent" << std::endl;
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
        std::cout << "MyPlugin GUI: Cannot show - frame not created" << std::endl;
        return false;
    }
    
    try {
        // If frame is not yet opened (embedded), we can't show it
        if (!frame->isAttached()) {
            std::cout << "MyPlugin GUI: Cannot show - frame not embedded in parent window" << std::endl;
            return false;
        }
        
        frame->setVisible(true);
        isVisible = true;
        std::cout << "MyPlugin GUI: Shown successfully" << std::endl;
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
        std::cout << "MyPlugin GUI: Hidden successfully" << std::endl;
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
    
    // Create a simple title label
    CRect titleRect(10, 10, 390, 30);
    auto titleLabel = new CTextLabel(titleRect, "My First CLAP Plugin with VSTGUI");
    titleLabel->setFontColor(kBlackCColor);
    titleLabel->setBackColor(kTransparentCColor);
    frame->addView(titleLabel);
    
    // Create a simple knob control (using simplified constructor)
    CRect knobRect(50, 50, 100, 100);
    auto knob = new CKnob(knobRect, nullptr, 0, nullptr, nullptr);
    knob->setDefaultValue(0.5f);
    knob->setValue(0.5f);
    frame->addView(knob);
    
    // Create a label for the knob
    CRect knobLabelRect(30, 110, 120, 130);
    auto knobLabel = new CTextLabel(knobLabelRect, "Volume");
    knobLabel->setFontColor(kBlackCColor);
    knobLabel->setBackColor(kTransparentCColor);
    frame->addView(knobLabel);
    
    // Create a horizontal slider control
    CRect sliderRect(150, 50, 350, 70);
    auto slider = new CHorizontalSlider(sliderRect, nullptr, 0, 0, 100, nullptr, nullptr);
    slider->setValue(50.0f);
    frame->addView(slider);
    
    // Create a label for the slider
    CRect sliderLabelRect(150, 80, 350, 100);
    auto sliderLabel = new CTextLabel(sliderLabelRect, "Filter Cutoff");
    sliderLabel->setFontColor(kBlackCColor);
    sliderLabel->setBackColor(kTransparentCColor);
    frame->addView(sliderLabel);
}