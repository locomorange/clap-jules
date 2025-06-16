#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <string>
#include "skia_graphics.h"

namespace clap_jules {
namespace gui {

// Forward declarations
class UIComponent;
class Knob;
class Switch;
class Slider;
class EQGraph;
class Button;
class Label;

// Color scheme for soothe2-style interface
namespace Colors {
    const graphics::Color Background{28, 28, 28, 255};
    const graphics::Color PanelBackground{35, 35, 35, 255};
    const graphics::Color Accent{255, 140, 60, 255};      // Orange accent
    const graphics::Color AccentHover{255, 160, 80, 255};
    const graphics::Color Text{220, 220, 220, 255};
    const graphics::Color TextSecondary{150, 150, 150, 255};
    const graphics::Color ControlBackground{45, 45, 45, 255};
    const graphics::Color ControlBorder{80, 80, 80, 255};
    const graphics::Color GraphGrid{60, 60, 60, 255};
    const graphics::Color GraphCurve{255, 140, 60, 255};
    const graphics::Color GraphPoint{255, 180, 100, 255};
}

// Parameter binding interface
struct ParameterInfo {
    uint32_t id;
    std::string name;
    float min_value;
    float max_value;
    float default_value;
    float current_value;
    std::string unit;
    std::function<void(float)> callback;
};

// Base UI component class
class UIComponent {
public:
    UIComponent(const graphics::Rect& bounds) : bounds_(bounds), visible_(true), enabled_(true) {}
    virtual ~UIComponent() = default;
    
    virtual void draw(graphics::GraphicsContext* ctx) = 0;
    virtual bool handleMouseDown(float x, float y) { return false; }
    virtual bool handleMouseUp(float x, float y) { return false; }
    virtual bool handleMouseMove(float x, float y) { return false; }
    virtual bool handleMouseWheel(float delta) { return false; }
    
    const graphics::Rect& getBounds() const { return bounds_; }
    void setBounds(const graphics::Rect& bounds) { bounds_ = bounds; }
    
    bool isVisible() const { return visible_; }
    void setVisible(bool visible) { visible_ = visible; }
    
    bool isEnabled() const { return enabled_; }
    void setEnabled(bool enabled) { enabled_ = enabled; }
    
    bool containsPoint(float x, float y) const {
        return x >= bounds_.x && x <= bounds_.x + bounds_.width &&
               y >= bounds_.y && y <= bounds_.y + bounds_.height;
    }

protected:
    graphics::Rect bounds_;
    bool visible_;
    bool enabled_;
};

// Knob component for continuous parameters
class Knob : public UIComponent {
public:
    Knob(const graphics::Rect& bounds, ParameterInfo* param);
    
    void draw(graphics::GraphicsContext* ctx) override;
    bool handleMouseDown(float x, float y) override;
    bool handleMouseUp(float x, float y) override;
    bool handleMouseMove(float x, float y) override;
    bool handleMouseWheel(float delta) override;
    
    void setValue(float value);
    float getValue() const { return parameter_->current_value; }
    
    void setLabel(const std::string& label) { label_ = label; }
    const std::string& getLabel() const { return label_; }

private:
    ParameterInfo* parameter_;
    std::string label_;
    bool is_dragging_;
    float drag_start_y_;
    float drag_start_value_;
    
    float normalizeValue(float value) const;
    float denormalizeValue(float normalized) const;
    void updateValue(float new_value);
};

// Switch component for boolean parameters
class Switch : public UIComponent {
public:
    Switch(const graphics::Rect& bounds, ParameterInfo* param);
    
    void draw(graphics::GraphicsContext* ctx) override;
    bool handleMouseDown(float x, float y) override;
    
    void setValue(bool value);
    bool getValue() const { return parameter_->current_value > 0.5f; }
    
    void setLabel(const std::string& label) { label_ = label; }
    const std::string& getLabel() const { return label_; }

private:
    ParameterInfo* parameter_;
    std::string label_;
    bool is_pressed_;
};

// EQ Graph component with control points
class EQGraph : public UIComponent {
public:
    struct EQPoint {
        float frequency;  // Hz
        float gain;       // dB
        float q;          // Quality factor
        bool active;
        graphics::Color color;
        
        EQPoint(float freq, float g, float quality) 
            : frequency(freq), gain(g), q(quality), active(true), color(Colors::GraphPoint) {}
    };
    
    EQGraph(const graphics::Rect& bounds);
    
    void draw(graphics::GraphicsContext* ctx) override;
    bool handleMouseDown(float x, float y) override;
    bool handleMouseUp(float x, float y) override;
    bool handleMouseMove(float x, float y) override;
    
    void addPoint(const EQPoint& point);
    void removePoint(size_t index);
    void clearPoints();
    
    const std::vector<EQPoint>& getPoints() const { return eq_points_; }
    void setPoints(const std::vector<EQPoint>& points) { eq_points_ = points; }

private:
    std::vector<EQPoint> eq_points_;
    int selected_point_;
    bool is_dragging_;
    
    // Frequency and gain conversion
    float freqToX(float frequency) const;
    float xToFreq(float x) const;
    float gainToY(float gain) const;
    float yToGain(float y) const;
    
    // EQ curve calculation
    std::vector<graphics::Point> calculateEQCurve() const;
    void drawGrid(graphics::GraphicsContext* ctx);
    void drawCurve(graphics::GraphicsContext* ctx);
    void drawPoints(graphics::GraphicsContext* ctx);
};

// Simple button component
class Button : public UIComponent {
public:
    Button(const graphics::Rect& bounds, const std::string& text);
    
    void draw(graphics::GraphicsContext* ctx) override;
    bool handleMouseDown(float x, float y) override;
    bool handleMouseUp(float x, float y) override;
    
    void setCallback(std::function<void()> callback) { callback_ = callback; }
    void setText(const std::string& text) { text_ = text; }
    const std::string& getText() const { return text_; }

private:
    std::string text_;
    std::function<void()> callback_;
    bool is_pressed_;
};

// Label component for text display
class Label : public UIComponent {
public:
    Label(const graphics::Rect& bounds, const std::string& text);
    
    void draw(graphics::GraphicsContext* ctx) override;
    
    void setText(const std::string& text) { text_ = text; }
    const std::string& getText() const { return text_; }
    
    void setColor(const graphics::Color& color) { color_ = color; }
    void setFontSize(float size) { font_size_ = size; }

private:
    std::string text_;
    graphics::Color color_;
    float font_size_;
};

// Main plugin GUI class
class PluginGUI {
public:
    PluginGUI(int width = 800, int height = 600);
    ~PluginGUI();
    
    // CLAP GUI interface
    bool create(const char* api, bool is_floating);
    void destroy();
    bool setSize(uint32_t width, uint32_t height);
    bool getSize(uint32_t* width, uint32_t* height);
    bool show();
    bool hide();
    
    // Rendering
    void render();
    void present();
    
    // Event handling
    void handleMouseDown(float x, float y);
    void handleMouseUp(float x, float y);
    void handleMouseMove(float x, float y);
    void handleMouseWheel(float delta);
    
    // Parameter management
    void addParameter(const ParameterInfo& param);
    void updateParameter(uint32_t param_id, float value);
    
    // Component management
    void addComponent(std::unique_ptr<UIComponent> component);
    void removeComponent(UIComponent* component);
    void clearComponents();

private:
    std::unique_ptr<graphics::GraphicsContext> graphics_context_;
    std::vector<std::unique_ptr<UIComponent>> components_;
    std::vector<ParameterInfo> parameters_;
    
    int width_;
    int height_;
    bool is_visible_;
    bool is_created_;
    
    UIComponent* focused_component_;
    UIComponent* hover_component_;
    
    void setupDefaultLayout();
    void createSoothe2StyleInterface();
    ParameterInfo* findParameter(uint32_t param_id);
};

} // namespace gui
} // namespace clap_jules