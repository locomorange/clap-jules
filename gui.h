#pragma once
#include <functional>
#include <cstdio>

// GUI View interface for future brisk integration
class PluginGUI {
public:
    virtual ~PluginGUI() = default;
    virtual void setFrequency(double frequency) = 0;
    virtual double getFrequency() const = 0;
    virtual void setFrequencyChangeCallback(std::function<void(double)> callback) = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual bool isVisible() const = 0;
};

// Simple implementation for now (to be replaced with Brisk later)
class SimplePluginGUI : public PluginGUI {
public:
    SimplePluginGUI() : m_frequency(1000.0), m_visible(false) {}
    
    void setFrequency(double frequency) override {
        m_frequency = frequency;
        printf("GUI: Frequency set to %.2f Hz\n", frequency);
    }
    
    double getFrequency() const override {
        return m_frequency;
    }
    
    void setFrequencyChangeCallback(std::function<void(double)> callback) override {
        m_callback = callback;
    }
    
    void show() override {
        m_visible = true;
        printf("GUI: Showing plugin interface\n");
    }
    
    void hide() override {
        m_visible = false;
        printf("GUI: Hiding plugin interface\n");
    }
    
    bool isVisible() const override {
        return m_visible;
    }
    
    // Simulate user interaction (this would be replaced by real UI controls)
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