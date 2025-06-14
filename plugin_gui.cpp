#include "plugin_gui.h"
#include "vstgui/vstgui.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include <cstdio>

using namespace VSTGUI;

PluginGUI::PluginGUI(const clap_plugin_t* plugin) 
    : plugin_(plugin), frame_(nullptr), volumeSlider_(nullptr), paramDisplay_(nullptr) {
}

PluginGUI::~PluginGUI() {
    destroyWindow();
}

bool PluginGUI::createWindow(clap_window_t* window) {
    if (frame_) {
        return false; // Already created
    }
    
    // Create the main frame
    CRect frameSize(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    frame_ = makeOwned<CFrame>(frameSize, nullptr);
    
    if (!frame_) {
        return false;
    }
    
    // Set up the GUI elements
    setupGUI();
    
    // Set the parent window
    if (window && window->ptr) {
#if LINUX
        frame_->open(window->ptr);
#endif
    }
    
    return true;
}

void PluginGUI::destroyWindow() {
    if (frame_) {
        frame_->close();
        frame_ = nullptr;
    }
    volumeSlider_ = nullptr;
    paramDisplay_ = nullptr;
}

bool PluginGUI::setParent(const clap_window_t* window) {
    if (!frame_ || !window || !window->ptr) {
        return false;
    }
    
    // For now, just return true - proper X11 embedding would require more complex code
    return true;
}

bool PluginGUI::setSize(uint32_t width, uint32_t height) {
    if (!frame_) {
        return false;
    }
    
    return frame_->setSize(width, height);
}

bool PluginGUI::getSize(uint32_t* width, uint32_t* height) {
    if (!frame_ || !width || !height) {
        return false;
    }
    
    CRect size;
    if (frame_->getSize(size)) {
        *width = size.getWidth();
        *height = size.getHeight();
        return true;
    }
    return false;
}

bool PluginGUI::canResize() {
    return true;
}

bool PluginGUI::getResizeHints(clap_gui_resize_hints_t* hints) {
    if (!hints) {
        return false;
    }
    
    hints->can_resize_horizontally = true;
    hints->can_resize_vertically = true;
    hints->preserve_aspect_ratio = false;
    hints->aspect_ratio_width = 0;
    hints->aspect_ratio_height = 0;
    return true;
}

bool PluginGUI::adjustSize(uint32_t* width, uint32_t* height) {
    if (!width || !height) {
        return false;
    }
    
    // Ensure minimum size
    if (*width < 200) *width = 200;
    if (*height < 150) *height = 150;
    
    // Ensure maximum size
    if (*width > 800) *width = 800;
    if (*height > 600) *height = 600;
    
    return true;
}

void PluginGUI::setScale(double scale) {
    // VSTGUI handles scaling internally
}

bool PluginGUI::show() {
    if (!frame_) {
        return false;
    }
    
    frame_->setVisible(true);
    return true;
}

bool PluginGUI::hide() {
    if (!frame_) {
        return false;
    }
    
    frame_->setVisible(false);
    return true;
}

void PluginGUI::setupGUI() {
    if (!frame_) {
        return;
    }
    
    // Set background color
    frame_->setBackgroundColor(CColor(240, 240, 240, 255));
    
    // Create a title label
    CRect labelRect(10, 10, 390, 40);
    auto titleLabel = makeOwned<CTextLabel>(labelRect, "My CLAP Plugin with VSTGUI");
    titleLabel->setFontColor(CColor(0, 0, 0, 255));
    titleLabel->setHoriAlign(kCenterText);
    frame_->addView(titleLabel);
    
    // Create a volume slider
    CRect sliderRect(50, 80, 350, 100);
    volumeSlider_ = makeOwned<CSlider>(sliderRect, nullptr, 0, 0, 100, nullptr, nullptr);
    volumeSlider_->setBackground(nullptr); // Use default background
    volumeSlider_->setFrameColor(CColor(100, 100, 100, 255));
    frame_->addView(volumeSlider_);
    
    // Create a parameter display
    CRect displayRect(50, 120, 350, 150);
    paramDisplay_ = makeOwned<CTextEdit>(displayRect, nullptr, 0);
    paramDisplay_->setText("Volume: 0%");
    paramDisplay_->setBackColor(CColor(255, 255, 255, 255));
    paramDisplay_->setFontColor(CColor(0, 0, 0, 255));
    frame_->addView(paramDisplay_);
    
    // Create an info label
    CRect infoRect(10, 200, 390, 280);
    auto infoLabel = makeOwned<CTextLabel>(infoRect, "VSTGUI successfully integrated!\nThis is a basic GUI example.");
    infoLabel->setFontColor(CColor(100, 100, 100, 255));
    infoLabel->setHoriAlign(kCenterText);
    frame_->addView(infoLabel);
}

// CLAP GUI extension functions
static bool plugin_gui_is_api_supported(const clap_plugin_t* plugin, const char* api, bool is_floating) {
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) {
        return true;
    }
    return false;
}

static bool plugin_gui_get_preferred_api(const clap_plugin_t* plugin, const char** api, bool* is_floating) {
    if (api) *api = CLAP_WINDOW_API_X11;
    if (is_floating) *is_floating = false;
    return true;
}

static bool plugin_gui_create(const clap_plugin_t* plugin, const char* api, bool is_floating) {
    if (strcmp(api, CLAP_WINDOW_API_X11) != 0) {
        return false;
    }
    
    // Create the GUI instance (this should be stored in plugin-specific data)
    // For this example, we'll create it temporarily
    return true;
}

static void plugin_gui_destroy(const clap_plugin_t* plugin) {
    // Destroy the GUI instance
}

static bool plugin_gui_set_size(const clap_plugin_t* plugin, uint32_t width, uint32_t height) {
    // This should get the GUI instance from plugin data and call setSize
    return true;
}

static bool plugin_gui_get_size(const clap_plugin_t* plugin, uint32_t* width, uint32_t* height) {
    if (width) *width = PluginGUI::DEFAULT_WIDTH;
    if (height) *height = PluginGUI::DEFAULT_HEIGHT;
    return true;
}

static bool plugin_gui_can_resize(const clap_plugin_t* plugin) {
    return true;
}

static bool plugin_gui_get_resize_hints(const clap_plugin_t* plugin, clap_gui_resize_hints_t* hints) {
    if (!hints) return false;
    
    hints->can_resize_horizontally = true;
    hints->can_resize_vertically = true;
    hints->preserve_aspect_ratio = false;
    hints->aspect_ratio_width = 0;
    hints->aspect_ratio_height = 0;
    return true;
}

static bool plugin_gui_adjust_size(const clap_plugin_t* plugin, uint32_t* width, uint32_t* height) {
    if (!width || !height) return false;
    
    if (*width < 200) *width = 200;
    if (*height < 150) *height = 150;
    if (*width > 800) *width = 800;
    if (*height > 600) *height = 600;
    
    return true;
}

static bool plugin_gui_set_scale(const clap_plugin_t* plugin, double scale) {
    // Handle scale changes
    return true;
}

static bool plugin_gui_set_parent(const clap_plugin_t* plugin, const clap_window_t* window) {
    // This should get the GUI instance from plugin data and call setParent
    return true;
}

static bool plugin_gui_set_transient(const clap_plugin_t* plugin, const clap_window_t* window) {
    return true;
}

static void plugin_gui_suggest_title(const clap_plugin_t* plugin, const char* title) {
    // Handle title suggestions
}

static bool plugin_gui_show(const clap_plugin_t* plugin) {
    return true;
}

static bool plugin_gui_hide(const clap_plugin_t* plugin) {
    return true;
}

const clap_plugin_gui_t plugin_gui_extension = {
    plugin_gui_is_api_supported,
    plugin_gui_get_preferred_api,
    plugin_gui_create,
    plugin_gui_destroy,
    plugin_gui_set_scale,
    plugin_gui_get_size,
    plugin_gui_can_resize,
    plugin_gui_get_resize_hints,
    plugin_gui_adjust_size,
    plugin_gui_set_size,
    plugin_gui_set_parent,
    plugin_gui_set_transient,
    plugin_gui_suggest_title,
    plugin_gui_show,
    plugin_gui_hide
};