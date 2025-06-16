#pragma once

#include "ui_components.h"
#include <memory>

namespace clap_jules {
namespace ui {

// Soothe2-style GUI layout and parameters
class Soothe2GUI {
public:
    Soothe2GUI(int width = 800, int height = 600);
    ~Soothe2GUI() = default;
    
    // Main interface
    void draw(graphics::GraphicsContext* ctx);
    bool handleMouse(const MouseEvent& event);
    void update(float deltaTime);
    void resize(int width, int height);
    
    // Parameter access
    struct Parameters {
        // Main controls
        float depth = 0.5f;        // Main depth/intensity
        float selectivity = 0.5f;  // Frequency selectivity
        float attack = 0.3f;       // Attack time
        float release = 0.7f;      // Release time
        
        // EQ bands
        struct EQBand {
            float frequency = 1000.0f;
            float gain = 0.0f;
            float q = 1.0f;
            bool enabled = true;
        };
        
        EQBand band1 = {200.0f, 0.0f, 1.0f, true};
        EQBand band2 = {800.0f, 0.0f, 1.0f, true};
        EQBand band3 = {3200.0f, 0.0f, 1.0f, true};
        EQBand band4 = {12800.0f, 0.0f, 1.0f, true};
        
        // Processing options
        bool bypass = false;
        bool solo = false;
        bool delta = false;        // Listen to what's being removed
        float mix = 1.0f;          // Dry/wet mix
        
        // Advanced
        float lookahead = 0.0f;
        float sharpness = 0.5f;
        bool softKnee = true;
    } params;
    
    // Callbacks for parameter changes
    void setParameterChangeCallback(std::function<void(const std::string&, float)> callback) {
        parameterChangeCallback_ = callback;
    }
    
private:
    int width_, height_;
    UIContainer container_;
    
    // Parameter change callback
    std::function<void(const std::string&, float)> parameterChangeCallback_;
    
    // UI components
    std::unique_ptr<Knob> depthKnob_;
    std::unique_ptr<Knob> selectivityKnob_;
    std::unique_ptr<Knob> attackKnob_;
    std::unique_ptr<Knob> releaseKnob_;
    std::unique_ptr<Knob> mixKnob_;
    std::unique_ptr<Knob> sharpnessKnob_;
    
    std::unique_ptr<Button> bypassButton_;
    std::unique_ptr<Button> soloButton_;
    std::unique_ptr<Button> deltaButton_;
    std::unique_ptr<Button> softKneeButton_;
    
    std::unique_ptr<EQGraph> eqGraph_;
    
    std::unique_ptr<Label> titleLabel_;
    std::unique_ptr<Label> depthLabel_;
    std::unique_ptr<Label> selectivityLabel_;
    std::unique_ptr<Label> attackLabel_;
    std::unique_ptr<Label> releaseLabel_;
    std::unique_ptr<Label> mixLabel_;
    std::unique_ptr<Label> sharpnessLabel_;
    
    // Layout methods
    void setupLayout();
    void setupMainControls();
    void setupEQSection();
    void setupButtons();
    void setupLabels();
    
    // Helper methods
    void onParameterChanged(const std::string& paramName, float value);
    void updateEQFromParameters();
    void updateParametersFromEQ();
    
    // Layout constants
    static constexpr int MARGIN = 20;
    static constexpr int KNOB_SIZE = 35;
    static constexpr int BUTTON_WIDTH = 60;
    static constexpr int BUTTON_HEIGHT = 25;
    static constexpr int CONTROL_SPACING = 15;
    static constexpr int SECTION_SPACING = 30;
};

} // namespace ui
} // namespace clap_jules