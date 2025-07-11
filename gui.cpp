#include "gui.h"

#ifdef HAVE_BRISK
#include <brisk/gui/Application.hpp> // For brisk::Application::instance() if needed for event loop or context
#include <brisk/logger/Logging.hpp>   // For BRISK_LOG_INFO, etc.
#include <brisk/platform/Platform.hpp> // For platform specific details if needed

// Anonymous namespace for helper functions or constants if any
namespace {
    const uint32_t DEFAULT_WIDTH = 300;
    const uint32_t DEFAULT_HEIGHT = 200;
}

BriskPluginGUI::BriskPluginGUI()
    : m_freq_knob(nullptr), m_freq_label(nullptr), m_current_frequency(1000.0) {
    BRISK_LOG_INFO("BriskPluginGUI constructor called");
    // Widgets will be created in the `create` method, as we need the parent window handle first.
    // brisk::Application::instance().initialize(); // May be needed if not initialized elsewhere
}

BriskPluginGUI::~BriskPluginGUI() {
    BRISK_LOG_INFO("BriskPluginGUI destructor called");
    // m_embed_window unique_ptr will handle deletion of the Brisk window and its children.
    // brisk::Application::instance().shutdown(); // If initialize was called
}

bool BriskPluginGUI::create(const clap_window_t* window_api) {
    BRISK_LOG_INFO("BriskPluginGUI create called. Parent HWND: %p, OS: %s", window_api->parent, window_api->plugin_api);

    if (!window_api || !window_api->parent) {
        BRISK_LOG_ERROR("Invalid window_api or parent handle provided.");
        return false;
    }

    // TODO: Ensure Brisk is initialized (e.g., brisk::Application or similar)
    // This might need to happen globally for the plugin instance, or once per GUI.
    // For now, assume Brisk's context is ready or will be by EmbedWindow.

    m_embed_window = std::make_unique<brisk::EmbedWindow>();
    if (!m_embed_window->embed(const_cast<void*>(window_api->parent))) { // Brisk might take non-const
        BRISK_LOG_ERROR("Failed to embed Brisk window.");
        m_embed_window.reset();
        return false;
    }

    // Setup layout
    auto* layout = new brisk::BoxLayout(brisk::Orientation::Vertical); // Raw new, EmbedWindow should take ownership
    m_embed_window->setLayout(layout);

    // Create Knob for frequency
    m_freq_knob = new brisk::Knob();
    m_freq_knob->setRange(20.0, 20000.0); // Audible range
    m_freq_knob->setValue(m_current_frequency);
    m_freq_knob->setStep(1.0); // Or logarithmic steps
    // Connect knob's valueChanged signal to our handler
    m_freq_knob->valueChanged.connect(this, &BriskPluginGUI::onKnobValueChanged);
    layout->add(m_freq_knob);

    // Create Label for frequency display
    m_freq_label = new brisk::Label();
    m_freq_label->setText("Freq: " + std::to_string(static_cast<int>(m_current_frequency)) + " Hz");
    layout->add(m_freq_label);

    // Set initial size (Brisk might do this automatically or need explicit call)
    // m_embed_window->setSize(DEFAULT_WIDTH, DEFAULT_HEIGHT); // Or use clap_plugin_gui.set_size

    BRISK_LOG_INFO("Brisk GUI created and widgets initialized.");
    return true;
}

void BriskPluginGUI::destroy() {
    BRISK_LOG_INFO("BriskPluginGUI destroy called");
    // m_embed_window unique_ptr will automatically delete the window when reset.
    // Brisk should handle cleanup of child widgets.
    if (m_embed_window) {
        m_embed_window->close(); // Ensure Brisk window is properly closed/detached
        m_embed_window.reset();
    }
    m_freq_knob = nullptr; // Pointers are managed by Brisk window
    m_freq_label = nullptr;
}

void BriskPluginGUI::setFrequency(double frequency) {
    m_current_frequency = frequency;
    if (m_freq_knob && m_freq_knob->value() != frequency) { // Avoid feedback loop if possible
        m_freq_knob->setValue(frequency);
    }
    if (m_freq_label) {
        m_freq_label->setText("Freq: " + std::to_string(static_cast<int>(m_current_frequency)) + " Hz");
    }
    // BRISK_LOG_DEBUG("BriskPluginGUI: Frequency set to %.2f Hz", frequency);
}

double BriskPluginGUI::getFrequency() const {
    return m_current_frequency;
}

void BriskPluginGUI::setFrequencyChangeCallback(std::function<void(double)> callback) {
    m_frequency_change_callback = callback;
}

bool BriskPluginGUI::set_size(uint32_t width, uint32_t height) {
    BRISK_LOG_INFO("BriskPluginGUI set_size: %u x %u", width, height);
    if (m_embed_window) {
        // Brisk might have a setContentSize or similar, or setSize might be for the top-level window.
        // For an embedded window, resizing might be handled by the host via the parent handle,
        // and Brisk might adapt automatically, or need a specific update call.
        m_embed_window->setSize(width, height);
        return true;
    }
    return false;
}

bool BriskPluginGUI::get_size(uint32_t* width, uint32_t* height) {
    if (m_embed_window) {
        // This should return the actual size of the Brisk content.
        auto size = m_embed_window->size();
        if (width) *width = static_cast<uint32_t>(size.width);
        if (height) *height = static_cast<uint32_t>(size.height);
        return true;
    }
    // Default if window not created
    if (width) *width = DEFAULT_WIDTH;
    if (height) *height = DEFAULT_HEIGHT;
    return false;
}

void BriskPluginGUI::show() {
    BRISK_LOG_INFO("BriskPluginGUI show called");
    if (m_embed_window) {
        m_embed_window->show();
        // May need to trigger a repaint or update if Brisk doesn't do it automatically on show
        m_embed_window->forceUpdate();
    }
}

void BriskPluginGUI::hide() {
    BRISK_LOG_INFO("BriskPluginGUI hide called");
    if (m_embed_window) {
        m_embed_window->hide();
    }
}

bool BriskPluginGUI::isVisible() const {
    return m_embed_window && m_embed_window->isVisible();
}

void* BriskPluginGUI::getNativeWindowHandle() {
    if (m_embed_window) {
        // This highly depends on how Brisk exposes the underlying native window,
        // or if it's even needed once embedded.
        // return m_embed_window->nativeHandle(); // Example, API may differ
    }
    return nullptr;
}

void BriskPluginGUI::onKnobValueChanged(double value) {
    // BRISK_LOG_DEBUG("Knob value changed: %.2f", value);
    m_current_frequency = value;
    if (m_freq_label) {
        m_freq_label->setText("Freq: " + std::to_string(static_cast<int>(m_current_frequency)) + " Hz");
    }
    if (m_frequency_change_callback) {
        m_frequency_change_callback(m_current_frequency);
    }
}

#endif // HAVE_BRISK
