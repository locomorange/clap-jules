#pragma once
#include <functional>
#include <memory> // For std::unique_ptr

// Forward declare Brisk types to reduce header includes here if possible,
// or include necessary Brisk headers directly.
// Assuming Brisk uses namespaces and has common widget types.
namespace brisk {
    class Window; // Main window or embeddable area
    class Knob;
    class Label;
    // It's likely Brisk might have an Application or EventLoop class too.
}

// GUI View interface
class IGui {
public:
    virtual ~IGui() = default;
    virtual void setFrequency(double frequency) = 0;
    virtual double getFrequency() const = 0; // May not be strictly needed if ViewModel is source of truth
    virtual void setFrequencyChangeCallback(std::function<void(double)> callback) = 0;

    // Methods for CLAP GUI extension
    virtual bool create(const clap_window_t* window_api) = 0; // Corresponds to clap_plugin_gui.create
    virtual void destroy() = 0;                               // Corresponds to clap_plugin_gui.destroy
    virtual bool set_size(uint32_t width, uint32_t height) = 0; // Corresponds to clap_plugin_gui.set_size
    virtual bool get_size(uint32_t* width, uint32_t* height) = 0; // Corresponds to clap_plugin_gui.get_size
    virtual void show() = 0; // Corresponds to clap_plugin_gui.show
    virtual void hide() = 0; // Corresponds to clap_plugin_gui.hide

    virtual bool isVisible() const = 0; // Helper
    virtual void* getNativeWindowHandle() = 0; // For embedding, may not be needed if Brisk handles it.
};


#ifdef HAVE_BRISK
#include <brisk/brisk.hpp> // Main Brisk header
#include <brisk/widgets/Window.hpp>
#include <brisk/widgets/Knob.hpp>
#include <brisk/widgets/Label.hpp>
#include <brisk/widgets/BoxLayout.hpp> // For layout
#include <brisk/gui/EmbedWindow.hpp> // For CLAP embedding

// Brisk-based GUI implementation
class BriskPluginGUI : public IGui {
public:
    BriskPluginGUI();
    ~BriskPluginGUI() override;

    void setFrequency(double frequency) override;
    double getFrequency() const override;
    void setFrequencyChangeCallback(std::function<void(double)> callback) override;

    // CLAP GUI methods
    bool create(const clap_window_t* window_api) override;
    void destroy() override;
    bool set_size(uint32_t width, uint32_t height) override;
    bool get_size(uint32_t* width, uint32_t* height) override;
    void show() override;
    void hide() override;

    bool isVisible() const override;
    void* getNativeWindowHandle() override;

private:
    std::unique_ptr<brisk::EmbedWindow> m_embed_window; // Embeddable Brisk window
    brisk::Knob* m_freq_knob; // Using raw pointer, assuming m_window owns it
    brisk::Label* m_freq_label;

    std::function<void(double)> m_frequency_change_callback;
    double m_current_frequency; // Cache last known frequency for display

    void onKnobValueChanged(double value);
};

#else
// Fallback SimplePluginGUI if Brisk is not available (from previous implementation)
#include <cstdio> // For printf in SimplePluginGUI
class SimplePluginGUI : public IGui {
public:
    SimplePluginGUI() : m_frequency(1000.0), m_visible(false) {}
    
    void setFrequency(double frequency) override {
        m_frequency = frequency;
        if (m_visible) printf("SimpleGUI: Freq set to %.2f Hz\n", frequency);
    }
    
    double getFrequency() const override { return m_frequency; }
    
    void setFrequencyChangeCallback(std::function<void(double)> callback) override {
        m_callback = callback;
    }
    
    bool create(const clap_window_t* window_api) override {
        printf("SimpleGUI: create called\n");
        // In a real non-Brisk GUI, you'd attach to window_api->parent here
        return true;
    }
    void destroy() override { printf("SimpleGUI: destroy called\n"); }
    bool set_size(uint32_t width, uint32_t height) override {
        printf("SimpleGUI: set_size to %u x %u\n", width, height);
        return true;
    }
    bool get_size(uint32_t* width, uint32_t* height) override {
        if (width) *width = 300; // Default size
        if (height) *height = 200;
        printf("SimpleGUI: get_size called\n");
        return true;
    }
    void show() override {
        m_visible = true;
        printf("SimpleGUI: show called\n");
    }
    void hide() override {
        m_visible = false;
        printf("SimpleGUI: hide called\n");
    }
    bool isVisible() const override { return m_visible; }
    void* getNativeWindowHandle() override { return nullptr; } // No native handle for simple GUI

    // Simulate user interaction (for testing without real UI)
    void simulateFrequencyChange(double newFrequency) {
        m_frequency = newFrequency;
        if (m_callback) {
            m_callback(newFrequency);
        }
    }
    
private:
    double m_frequency;
    bool m_visible;
    std::function<void(double)> m_callback;
};
#endif // HAVE_BRISK