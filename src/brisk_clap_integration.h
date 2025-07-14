#pragma once

#include <clap/clap.h>
#include <cstdint>
#include <memory>

namespace brisk_clap {

// Simple GUI wrapper for CLAP integration
// Using pimpl idiom to hide Brisk implementation details
class BriskClapGUI {
public:
    BriskClapGUI();
    ~BriskClapGUI();
    
    // Prevent copying
    BriskClapGUI(const BriskClapGUI&) = delete;
    BriskClapGUI& operator=(const BriskClapGUI&) = delete;
    
    // Initialization
    bool initialize();
    void shutdown();
    
    // Window management
    bool createWindow(const clap_window_t* parentWindow);
    void destroyWindow();
    bool setSize(uint32_t width, uint32_t height);
    void getSize(uint32_t* width, uint32_t* height) const;
    
    // Visibility
    void show();
    void hide();
    
    // Event processing
    void processEvents();
    
    // Parameter updates
    void updateParameter(clap_id paramId, double value);
    
private:
    // Private implementation to keep Brisk types out of the header
    class Impl;
    std::unique_ptr<Impl> m_impl;
    
    // State
    bool m_initialized = false;
    bool m_windowCreated = false;
    uint32_t m_width = 400;
    uint32_t m_height = 300;
    
    // Parameter values
    double m_gain = 0.7;
    double m_cutoff = 1000.0;
    bool m_bypass = false;
    
    // GUI layout creation
    void createGUILayout();
};

// Global initialization helper
class BriskClapInitializer {
public:
    static BriskClapInitializer& instance();
    
    void initialize(int argc = 0, char** argv = nullptr);
    void shutdown();
    bool isInitialized() const { return m_initialized; }
    
private:
    BriskClapInitializer() = default;
    ~BriskClapInitializer() = default;
    
    bool m_initialized = false;
};

} // namespace brisk_clap