#include "my_plugin_gui.h"
#include <vstgui/lib/controls/cknob.h>
#include <vstgui/lib/controls/cslider.h>
#include <vstgui/lib/controls/ctextlabel.h>
#include <iostream>

MyPluginEditor::MyPluginEditor()
    : frame(nullptr)
    , isCreated(false) 
    , isVisible(false)
    , currentWidth(400)
    , currentHeight(300)
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
        return true;
    }
    
    // Check if the API is supported for this platform
    if (!isApiSupported(api, isFloating)) {
        return false;
    }
    
    try {
        // Create VSTGUI frame
        CRect rect(0, 0, currentWidth, currentHeight);
        frame = new CFrame(rect, nullptr);
        
        if (frame) {
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
        frame->forget();
        frame = nullptr;
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
        // Embed into X11 window
        if (window->api && strcmp(window->api, CLAP_WINDOW_API_X11) == 0) {
            // Use VSTGUI's platform-specific methods to embed
            // For now, just return true to indicate we can handle the parent window
            std::cout << "MyPlugin GUI: Set parent window (X11 ID: " << window->x11 << ")" << std::endl;
            return true;
        }
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