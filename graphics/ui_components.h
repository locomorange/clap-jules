#pragma once

#include "skia_graphics.h"
#include <functional>
#include <vector>
#include <string>
#include <memory>

namespace clap_jules {
namespace ui {

// Forward declarations
class UIComponent;
class Knob;
class Slider;
class Button;
class EQGraph;
class Label;

// Mouse event structure
struct MouseEvent {
    float x, y;
    bool pressed;
    bool dragging;
    int button; // 0 = left, 1 = right, 2 = middle
};

// Color scheme for soothe2-style interface
namespace Colors {
    // Background colors
    const graphics::Color BACKGROUND_DARK = graphics::Color(25, 25, 30);
    const graphics::Color BACKGROUND_MEDIUM = graphics::Color(35, 35, 40);
    const graphics::Color BACKGROUND_LIGHT = graphics::Color(45, 45, 50);
    
    // Control colors
    const graphics::Color KNOB_OUTER = graphics::Color(60, 60, 65);
    const graphics::Color KNOB_INNER = graphics::Color(80, 80, 85);
    const graphics::Color KNOB_INDICATOR = graphics::Color(120, 180, 255);
    
    // EQ curve colors
    const graphics::Color EQ_BACKGROUND = graphics::Color(20, 20, 25);
    const graphics::Color EQ_GRID = graphics::Color(40, 40, 45);
    const graphics::Color EQ_CURVE = graphics::Color(120, 180, 255);
    const graphics::Color EQ_FILL = graphics::Color(120, 180, 255, 40);
    const graphics::Color EQ_CONTROL_POINT = graphics::Color(255, 120, 80);
    const graphics::Color EQ_CONTROL_POINT_HOVER = graphics::Color(255, 150, 100);
    
    // Text colors
    const graphics::Color TEXT_PRIMARY = graphics::Color(220, 220, 220);
    const graphics::Color TEXT_SECONDARY = graphics::Color(180, 180, 180);
    const graphics::Color TEXT_LABEL = graphics::Color(140, 140, 140);
    
    // Accent colors
    const graphics::Color ACCENT_BLUE = graphics::Color(120, 180, 255);
    const graphics::Color ACCENT_ORANGE = graphics::Color(255, 120, 80);
    const graphics::Color ACCENT_GREEN = graphics::Color(120, 255, 120);
}

// Base UI component class
class UIComponent {
public:
    UIComponent(float x, float y, float width, float height);
    virtual ~UIComponent() = default;
    
    virtual void draw(graphics::GraphicsContext* ctx) = 0;
    virtual bool handleMouse(const MouseEvent& event) = 0;
    virtual void update(float deltaTime) {}
    
    // Position and size
    void setPosition(float x, float y) { x_ = x; y_ = y; }
    void setSize(float width, float height) { width_ = width; height_ = height; }
    graphics::Rect getBounds() const { return graphics::Rect(x_, y_, width_, height_); }
    
    // Visibility
    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }
    
    // Hit testing
    bool contains(float x, float y) const;
    
protected:
    float x_, y_, width_, height_;
    bool visible_ = true;
    bool hovered_ = false;
    bool pressed_ = false;
};

// Rotary knob control
class Knob : public UIComponent {
public:
    Knob(float x, float y, float radius, float minValue = 0.0f, float maxValue = 1.0f);
    
    void draw(graphics::GraphicsContext* ctx) override;
    bool handleMouse(const MouseEvent& event) override;
    
    // Value control
    void setValue(float value);
    float getValue() const { return value_; }
    void setRange(float minValue, float maxValue);
    
    // Callbacks
    void setOnValueChanged(std::function<void(float)> callback) { onValueChanged_ = callback; }
    
    // Appearance
    void setLabel(const std::string& label) { label_ = label; }
    void setColors(const graphics::Color& outer, const graphics::Color& inner, const graphics::Color& indicator);
    
private:
    float radius_;
    float value_;
    float minValue_, maxValue_;
    float lastMouseY_;
    bool dragging_ = false;
    std::string label_;
    std::function<void(float)> onValueChanged_;
    
    // Colors
    graphics::Color outerColor_ = Colors::KNOB_OUTER;
    graphics::Color innerColor_ = Colors::KNOB_INNER;
    graphics::Color indicatorColor_ = Colors::KNOB_INDICATOR;
};

// Horizontal slider control
class Slider : public UIComponent {
public:
    Slider(float x, float y, float width, float height, float minValue = 0.0f, float maxValue = 1.0f);
    
    void draw(graphics::GraphicsContext* ctx) override;
    bool handleMouse(const MouseEvent& event) override;
    
    // Value control
    void setValue(float value);
    float getValue() const { return value_; }
    void setRange(float minValue, float maxValue);
    
    // Callbacks
    void setOnValueChanged(std::function<void(float)> callback) { onValueChanged_ = callback; }
    
    // Appearance
    void setLabel(const std::string& label) { label_ = label; }
    
private:
    float value_;
    float minValue_, maxValue_;
    bool dragging_ = false;
    std::string label_;
    std::function<void(float)> onValueChanged_;
};

// Button control
class Button : public UIComponent {
public:
    Button(float x, float y, float width, float height, const std::string& text);
    
    void draw(graphics::GraphicsContext* ctx) override;
    bool handleMouse(const MouseEvent& event) override;
    
    // Callbacks
    void setOnClick(std::function<void()> callback) { onClick_ = callback; }
    
    // Appearance
    void setText(const std::string& text) { text_ = text; }
    void setToggle(bool toggle) { isToggle_ = toggle; }
    void setPressed(bool pressed) { toggleState_ = pressed; }
    bool isPressed() const { return toggleState_; }
    
private:
    std::string text_;
    bool isToggle_ = false;
    bool toggleState_ = false;
    std::function<void()> onClick_;
};

// Text label
class Label : public UIComponent {
public:
    Label(float x, float y, const std::string& text, float fontSize = 12.0f);
    
    void draw(graphics::GraphicsContext* ctx) override;
    bool handleMouse(const MouseEvent& event) override { return false; } // Labels don't handle mouse
    
    // Appearance
    void setText(const std::string& text) { text_ = text; }
    void setFontSize(float size) { fontSize_ = size; }
    void setColor(const graphics::Color& color) { color_ = color; }
    void setAlignment(int alignment) { alignment_ = alignment; } // 0=left, 1=center, 2=right
    
private:
    std::string text_;
    float fontSize_;
    graphics::Color color_ = Colors::TEXT_PRIMARY;
    int alignment_ = 0;
};

// EQ control point
struct EQPoint {
    float frequency; // Hz
    float gain;      // dB
    float q;         // Quality factor
    bool active;
    
    EQPoint(float freq = 1000.0f, float g = 0.0f, float quality = 1.0f) 
        : frequency(freq), gain(g), q(quality), active(true) {}
};

// Interactive EQ graph
class EQGraph : public UIComponent {
public:
    EQGraph(float x, float y, float width, float height);
    
    void draw(graphics::GraphicsContext* ctx) override;
    bool handleMouse(const MouseEvent& event) override;
    
    // EQ control points
    void addControlPoint(const EQPoint& point);
    void removeControlPoint(int index);
    void setControlPoint(int index, const EQPoint& point);
    const std::vector<EQPoint>& getControlPoints() const { return controlPoints_; }
    
    // Frequency response
    void setFrequencyRange(float minFreq, float maxFreq);
    void setGainRange(float minGain, float maxGain);
    
    // Callbacks
    void setOnPointChanged(std::function<void(int, const EQPoint&)> callback) { onPointChanged_ = callback; }
    
private:
    std::vector<EQPoint> controlPoints_;
    float minFreq_ = 20.0f;
    float maxFreq_ = 20000.0f;
    float minGain_ = -24.0f;
    float maxGain_ = 24.0f;
    
    int draggedPoint_ = -1;
    bool dragging_ = false;
    graphics::Point lastMousePos_;
    
    std::function<void(int, const EQPoint&)> onPointChanged_;
    
    // Helper functions
    float freqToX(float frequency) const;
    float gainToY(float gain) const;
    float xToFreq(float x) const;
    float yToGain(float y) const;
    
    void drawGrid(graphics::GraphicsContext* ctx);
    void drawCurve(graphics::GraphicsContext* ctx);
    void drawControlPoints(graphics::GraphicsContext* ctx);
    
    // Find closest control point to mouse position
    int findClosestPoint(float x, float y) const;
    
    // Calculate frequency response at given frequency
    float calculateResponse(float frequency) const;
};

// UI Container to manage multiple components
class UIContainer {
public:
    UIContainer() = default;
    ~UIContainer() = default;
    
    // Disable copy operations since we have unique_ptr members
    UIContainer(const UIContainer&) = delete;
    UIContainer& operator=(const UIContainer&) = delete;
    
    // Enable move operations
    UIContainer(UIContainer&&) = default;
    UIContainer& operator=(UIContainer&&) = default;
    
    void addComponent(std::unique_ptr<UIComponent> component);
    void removeComponent(UIComponent* component);
    
    void draw(graphics::GraphicsContext* ctx);
    bool handleMouse(const MouseEvent& event);
    void update(float deltaTime);
    
    // Component access
    template<typename T>
    T* getComponent() {
        for (auto& comp : components_) {
            if (auto* typed = dynamic_cast<T*>(comp.get())) {
                return typed;
            }
        }
        return nullptr;
    }
    
private:
    std::vector<std::unique_ptr<UIComponent>> components_;
};

} // namespace ui
} // namespace clap_jules