#pragma once

#ifdef BRISK_FOUND
#include <brisk/core/App.hpp>
#include <brisk/window/Window.hpp>
#include <brisk/gui/Gui.hpp>
#include <brisk/widgets/Widgets.hpp>
#endif

#include <clap/clap.h>
#include <memory>

namespace clap_gui {

class MinimalBriskGUI {
public:
    MinimalBriskGUI();
    ~MinimalBriskGUI();

    // CLAP GUI interface methods
    bool create(const clap_window_t* window, bool is_floating);
    void destroy();
    bool set_size(uint32_t width, uint32_t height);
    bool get_size(uint32_t* width, uint32_t* height);
    void show();
    void hide();

    // Parameter update methods
    void set_parameter_value(clap_id param_id, double value);
    
private:
    void init_brisk_gui();
    void cleanup_brisk_gui();
    
#ifdef BRISK_FOUND
    void create_ui_layout();
    void reset_parameters();
#endif
    
#ifdef BRISK_FOUND
    std::unique_ptr<Brisk::Window> m_window;
    std::shared_ptr<Brisk::Widget> m_root_widget;
#endif
    
    bool m_initialized = false;
    uint32_t m_width = 400;
    uint32_t m_height = 300;
    
    // Plugin parameters
    double m_volume_value = 0.7;
    bool m_bypass = false;
};

} // namespace clap_gui
