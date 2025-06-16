#include "plugin_gui.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace clap_jules {
namespace gui {

// Utility functions
namespace {
    float clamp(float value, float min, float max) {
        return std::max(min, std::min(max, value));
    }
    
    float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }
    
    std::string formatValue(float value, const std::string& unit, int precision = 2) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision) << value << unit;
        return ss.str();
    }
}

// Knob implementation
Knob::Knob(const graphics::Rect& bounds, ParameterInfo* param)
    : UIComponent(bounds), parameter_(param), is_dragging_(false), 
      drag_start_y_(0), drag_start_value_(0) {
    if (param) {
        label_ = param->name;
    }
}

void Knob::draw(graphics::GraphicsContext* ctx) {
    if (!visible_ || !ctx) return;
    
    ctx->save();
    
    float centerX = bounds_.x + bounds_.width * 0.5f;
    float centerY = bounds_.y + bounds_.height * 0.4f;
    float radius = std::min(bounds_.width, bounds_.height) * 0.3f;
    
    // Draw outer ring
    ctx->drawCircle(graphics::Point(centerX, centerY), radius + 2, Colors::ControlBorder);
    ctx->drawCircle(graphics::Point(centerX, centerY), radius, Colors::ControlBackground);
    
    // Draw value arc
    if (parameter_) {
        float normalized = normalizeValue(parameter_->current_value);
        float angle = -2.4f + normalized * 4.8f; // -135° to +135°
        
        // Draw the arc indicator
        float indicatorX = centerX + (radius - 4) * std::cos(angle);
        float indicatorY = centerY + (radius - 4) * std::sin(angle);
        ctx->drawLine(graphics::Point(centerX, centerY), 
                     graphics::Point(indicatorX, indicatorY), 
                     Colors::Accent, 3.0f);
        
        // Draw small circle at the end
        ctx->drawCircle(graphics::Point(indicatorX, indicatorY), 3, Colors::Accent);
    }
    
    // Draw label
    if (!label_.empty()) {
        ctx->drawText(label_, 
                     graphics::Point(bounds_.x + bounds_.width * 0.5f - label_.length() * 3, 
                                   bounds_.y + bounds_.height * 0.75f), 
                     Colors::Text, 11.0f);
    }
    
    // Draw value
    if (parameter_) {
        std::string valueText = formatValue(parameter_->current_value, parameter_->unit);
        ctx->drawText(valueText, 
                     graphics::Point(bounds_.x + bounds_.width * 0.5f - valueText.length() * 3, 
                                   bounds_.y + bounds_.height * 0.9f), 
                     Colors::TextSecondary, 9.0f);
    }
    
    ctx->restore();
}

bool Knob::handleMouseDown(float x, float y) {
    if (!enabled_ || !containsPoint(x, y) || !parameter_) return false;
    
    is_dragging_ = true;
    drag_start_y_ = y;
    drag_start_value_ = parameter_->current_value;
    return true;
}

bool Knob::handleMouseUp(float x, float y) {
    if (is_dragging_) {
        is_dragging_ = false;
        return true;
    }
    return false;
}

bool Knob::handleMouseMove(float x, float y) {
    if (!is_dragging_ || !parameter_) return false;
    
    float deltaY = drag_start_y_ - y; // Inverted for natural feel
    float sensitivity = 0.005f;
    float deltaValue = deltaY * sensitivity * (parameter_->max_value - parameter_->min_value);
    
    updateValue(drag_start_value_ + deltaValue);
    return true;
}

bool Knob::handleMouseWheel(float delta) {
    if (!enabled_ || !parameter_) return false;
    
    float sensitivity = 0.05f;
    float deltaValue = delta * sensitivity * (parameter_->max_value - parameter_->min_value);
    updateValue(parameter_->current_value + deltaValue);
    return true;
}

void Knob::setValue(float value) {
    if (parameter_) {
        updateValue(value);
    }
}

float Knob::normalizeValue(float value) const {
    if (!parameter_) return 0.0f;
    return (value - parameter_->min_value) / (parameter_->max_value - parameter_->min_value);
}

float Knob::denormalizeValue(float normalized) const {
    if (!parameter_) return 0.0f;
    return parameter_->min_value + normalized * (parameter_->max_value - parameter_->min_value);
}

void Knob::updateValue(float new_value) {
    if (!parameter_) return;
    
    new_value = clamp(new_value, parameter_->min_value, parameter_->max_value);
    parameter_->current_value = new_value;
    
    if (parameter_->callback) {
        parameter_->callback(new_value);
    }
}

// Switch implementation
Switch::Switch(const graphics::Rect& bounds, ParameterInfo* param)
    : UIComponent(bounds), parameter_(param), is_pressed_(false) {
    if (param) {
        label_ = param->name;
    }
}

void Switch::draw(graphics::GraphicsContext* ctx) {
    if (!visible_ || !ctx) return;
    
    ctx->save();
    
    bool isOn = parameter_ && parameter_->current_value > 0.5f;
    graphics::Color bgColor = isOn ? Colors::Accent : Colors::ControlBackground;
    graphics::Color borderColor = is_pressed_ ? Colors::AccentHover : Colors::ControlBorder;
    
    // Draw switch background
    ctx->drawRect(bounds_, borderColor);
    graphics::Rect innerRect = {bounds_.x + 2, bounds_.y + 2, 
                               bounds_.width - 4, bounds_.height - 4};
    ctx->drawRect(innerRect, bgColor);
    
    // Draw switch indicator
    float indicatorWidth = bounds_.width * 0.4f;
    float indicatorHeight = bounds_.height * 0.6f;
    float indicatorX = isOn ? bounds_.x + bounds_.width - indicatorWidth - 4 : bounds_.x + 4;
    float indicatorY = bounds_.y + (bounds_.height - indicatorHeight) * 0.5f;
    
    graphics::Rect indicatorRect = {indicatorX, indicatorY, indicatorWidth, indicatorHeight};
    ctx->drawRect(indicatorRect, Colors::Text);
    
    // Draw label
    if (!label_.empty()) {
        ctx->drawText(label_, 
                     graphics::Point(bounds_.x + bounds_.width + 8, 
                                   bounds_.y + bounds_.height * 0.5f), 
                     Colors::Text, 11.0f);
    }
    
    ctx->restore();
}

bool Switch::handleMouseDown(float x, float y) {
    if (!enabled_ || !containsPoint(x, y) || !parameter_) return false;
    
    is_pressed_ = true;
    bool newValue = !getValue();
    setValue(newValue);
    return true;
}

void Switch::setValue(bool value) {
    if (parameter_) {
        parameter_->current_value = value ? 1.0f : 0.0f;
        if (parameter_->callback) {
            parameter_->callback(parameter_->current_value);
        }
    }
}

// EQGraph implementation
EQGraph::EQGraph(const graphics::Rect& bounds)
    : UIComponent(bounds), selected_point_(-1), is_dragging_(false) {
    // Add some default EQ points
    eq_points_.emplace_back(100.0f, 0.0f, 1.0f);   // Low
    eq_points_.emplace_back(1000.0f, 0.0f, 1.0f);  // Mid
    eq_points_.emplace_back(8000.0f, 0.0f, 1.0f);  // High
}

void EQGraph::draw(graphics::GraphicsContext* ctx) {
    if (!visible_ || !ctx) return;
    
    ctx->save();
    
    // Draw background
    ctx->drawRect(bounds_, Colors::PanelBackground);
    
    // Draw grid
    drawGrid(ctx);
    
    // Draw EQ curve
    drawCurve(ctx);
    
    // Draw control points
    drawPoints(ctx);
    
    ctx->restore();
}

void EQGraph::drawGrid(graphics::GraphicsContext* ctx) {
    // Frequency grid lines (log scale)
    std::vector<float> frequencies = {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    for (float freq : frequencies) {
        float x = freqToX(freq);
        ctx->drawLine(graphics::Point(x, bounds_.y), 
                     graphics::Point(x, bounds_.y + bounds_.height), 
                     Colors::GraphGrid, 1.0f);
    }
    
    // Gain grid lines
    for (int gain = -12; gain <= 12; gain += 3) {
        float y = gainToY(static_cast<float>(gain));
        ctx->drawLine(graphics::Point(bounds_.x, y), 
                     graphics::Point(bounds_.x + bounds_.width, y), 
                     Colors::GraphGrid, 1.0f);
    }
    
    // Zero line (thicker)
    float zeroY = gainToY(0.0f);
    ctx->drawLine(graphics::Point(bounds_.x, zeroY), 
                 graphics::Point(bounds_.x + bounds_.width, zeroY), 
                 Colors::ControlBorder, 2.0f);
}

void EQGraph::drawCurve(graphics::GraphicsContext* ctx) {
    auto curvePoints = calculateEQCurve();
    if (curvePoints.size() < 2) return;
    
    for (size_t i = 1; i < curvePoints.size(); ++i) {
        ctx->drawLine(curvePoints[i-1], curvePoints[i], Colors::GraphCurve, 2.0f);
    }
}

void EQGraph::drawPoints(graphics::GraphicsContext* ctx) {
    for (size_t i = 0; i < eq_points_.size(); ++i) {
        const auto& point = eq_points_[i];
        if (!point.active) continue;
        
        float x = freqToX(point.frequency);
        float y = gainToY(point.gain);
        
        graphics::Color pointColor = (i == selected_point_) ? Colors::AccentHover : point.color;
        ctx->drawCircle(graphics::Point(x, y), 6, pointColor);
        ctx->drawCircle(graphics::Point(x, y), 4, Colors::PanelBackground);
    }
}

std::vector<graphics::Point> EQGraph::calculateEQCurve() const {
    std::vector<graphics::Point> points;
    const int numPoints = 200;
    
    for (int i = 0; i < numPoints; ++i) {
        float x = bounds_.x + (bounds_.width * i) / (numPoints - 1);
        float frequency = xToFreq(x);
        float totalGain = 0.0f;
        
        // Simple EQ calculation - sum of bell filters
        for (const auto& eqPoint : eq_points_) {
            if (!eqPoint.active) continue;
            
            float ratio = frequency / eqPoint.frequency;
            float logRatio = std::log2(ratio);
            float bandwidth = 1.0f / eqPoint.q;
            
            // Bell filter approximation
            float gain = eqPoint.gain / (1.0f + std::pow(logRatio / bandwidth, 2.0f));
            totalGain += gain;
        }
        
        float y = gainToY(totalGain);
        points.emplace_back(x, y);
    }
    
    return points;
}

bool EQGraph::handleMouseDown(float x, float y) {
    if (!enabled_ || !containsPoint(x, y)) return false;
    
    // Check if clicking on existing point
    selected_point_ = -1;
    for (size_t i = 0; i < eq_points_.size(); ++i) {
        float pointX = freqToX(eq_points_[i].frequency);
        float pointY = gainToY(eq_points_[i].gain);
        
        float distance = std::sqrt((x - pointX) * (x - pointX) + (y - pointY) * (y - pointY));
        if (distance <= 10.0f) {
            selected_point_ = static_cast<int>(i);
            is_dragging_ = true;
            return true;
        }
    }
    
    return false;
}

bool EQGraph::handleMouseUp(float x, float y) {
    if (is_dragging_) {
        is_dragging_ = false;
        return true;
    }
    return false;
}

bool EQGraph::handleMouseMove(float x, float y) {
    if (!is_dragging_ || selected_point_ < 0 || 
        selected_point_ >= static_cast<int>(eq_points_.size())) return false;
    
    // Update selected point
    eq_points_[selected_point_].frequency = clamp(xToFreq(x), 20.0f, 20000.0f);
    eq_points_[selected_point_].gain = clamp(yToGain(y), -15.0f, 15.0f);
    
    return true;
}

float EQGraph::freqToX(float frequency) const {
    float logFreq = std::log10(clamp(frequency, 20.0f, 20000.0f));
    float logMin = std::log10(20.0f);
    float logMax = std::log10(20000.0f);
    float normalized = (logFreq - logMin) / (logMax - logMin);
    return bounds_.x + normalized * bounds_.width;
}

float EQGraph::xToFreq(float x) const {
    float normalized = (x - bounds_.x) / bounds_.width;
    normalized = clamp(normalized, 0.0f, 1.0f);
    float logMin = std::log10(20.0f);
    float logMax = std::log10(20000.0f);
    float logFreq = logMin + normalized * (logMax - logMin);
    return std::pow(10.0f, logFreq);
}

float EQGraph::gainToY(float gain) const {
    float normalized = (gain + 15.0f) / 30.0f; // -15dB to +15dB range
    normalized = clamp(normalized, 0.0f, 1.0f);
    return bounds_.y + bounds_.height - (normalized * bounds_.height); // Flip Y
}

float EQGraph::yToGain(float y) const {
    float normalized = (bounds_.y + bounds_.height - y) / bounds_.height; // Flip Y
    normalized = clamp(normalized, 0.0f, 1.0f);
    return (normalized * 30.0f) - 15.0f; // -15dB to +15dB range
}

void EQGraph::addPoint(const EQPoint& point) {
    eq_points_.push_back(point);
}

void EQGraph::removePoint(size_t index) {
    if (index < eq_points_.size()) {
        eq_points_.erase(eq_points_.begin() + index);
        if (selected_point_ == static_cast<int>(index)) {
            selected_point_ = -1;
        }
    }
}

void EQGraph::clearPoints() {
    eq_points_.clear();
    selected_point_ = -1;
}

// Button implementation
Button::Button(const graphics::Rect& bounds, const std::string& text)
    : UIComponent(bounds), text_(text), is_pressed_(false) {
}

void Button::draw(graphics::GraphicsContext* ctx) {
    if (!visible_ || !ctx) return;
    
    ctx->save();
    
    graphics::Color bgColor = is_pressed_ ? Colors::Accent : Colors::ControlBackground;
    graphics::Color borderColor = Colors::ControlBorder;
    
    ctx->drawRect(bounds_, borderColor);
    graphics::Rect innerRect = {bounds_.x + 1, bounds_.y + 1, 
                               bounds_.width - 2, bounds_.height - 2};
    ctx->drawRect(innerRect, bgColor);
    
    // Draw text
    ctx->drawText(text_, 
                 graphics::Point(bounds_.x + bounds_.width * 0.5f - text_.length() * 3, 
                               bounds_.y + bounds_.height * 0.5f), 
                 Colors::Text, 11.0f);
    
    ctx->restore();
}

bool Button::handleMouseDown(float x, float y) {
    if (!enabled_ || !containsPoint(x, y)) return false;
    
    is_pressed_ = true;
    if (callback_) {
        callback_();
    }
    return true;
}

bool Button::handleMouseUp(float x, float y) {
    if (is_pressed_) {
        is_pressed_ = false;
        return true;
    }
    return false;
}

// Label implementation
Label::Label(const graphics::Rect& bounds, const std::string& text)
    : UIComponent(bounds), text_(text), color_(Colors::Text), font_size_(12.0f) {
}

void Label::draw(graphics::GraphicsContext* ctx) {
    if (!visible_ || !ctx) return;
    
    ctx->drawText(text_, graphics::Point(bounds_.x, bounds_.y), color_, font_size_);
}

// PluginGUI implementation
PluginGUI::PluginGUI(int width, int height)
    : width_(width), height_(height), is_visible_(false), is_created_(false),
      focused_component_(nullptr), hover_component_(nullptr) {
    graphics_context_ = graphics::createGraphicsContext(width, height);
}

PluginGUI::~PluginGUI() {
    destroy();
}

bool PluginGUI::create(const char* api, bool is_floating) {
    if (is_created_) return true;
    
    // Initialize graphics context if needed
    if (!graphics_context_) {
        graphics_context_ = graphics::createGraphicsContext(width_, height_);
        if (!graphics_context_) return false;
    }
    
    setupDefaultLayout();
    is_created_ = true;
    return true;
}

void PluginGUI::destroy() {
    if (!is_created_) return;
    
    clearComponents();
    graphics_context_.reset();
    is_created_ = false;
    is_visible_ = false;
}

bool PluginGUI::setSize(uint32_t width, uint32_t height) {
    width_ = static_cast<int>(width);
    height_ = static_cast<int>(height);
    
    if (graphics_context_) {
        // Recreate graphics context with new size
        graphics_context_ = graphics::createGraphicsContext(width_, height_);
    }
    
    return true;
}

bool PluginGUI::getSize(uint32_t* width, uint32_t* height) {
    if (width) *width = static_cast<uint32_t>(width_);
    if (height) *height = static_cast<uint32_t>(height_);
    return true;
}

bool PluginGUI::show() {
    is_visible_ = true;
    return true;
}

bool PluginGUI::hide() {
    is_visible_ = false;
    return true;
}

void PluginGUI::render() {
    if (!is_visible_ || !graphics_context_) return;
    
    // Clear background
    graphics_context_->clear(Colors::Background);
    
    // Render all components
    for (auto& component : components_) {
        if (component && component->isVisible()) {
            component->draw(graphics_context_.get());
        }
    }
}

void PluginGUI::present() {
    if (graphics_context_) {
        graphics_context_->present();
    }
}

void PluginGUI::handleMouseDown(float x, float y) {
    focused_component_ = nullptr;
    
    // Find topmost component that handles the event
    for (auto it = components_.rbegin(); it != components_.rend(); ++it) {
        auto& component = *it;
        if (component && component->isVisible() && component->isEnabled()) {
            if (component->handleMouseDown(x, y)) {
                focused_component_ = component.get();
                break;
            }
        }
    }
}

void PluginGUI::handleMouseUp(float x, float y) {
    if (focused_component_) {
        focused_component_->handleMouseUp(x, y);
        focused_component_ = nullptr;
    }
}

void PluginGUI::handleMouseMove(float x, float y) {
    if (focused_component_) {
        focused_component_->handleMouseMove(x, y);
    }
}

void PluginGUI::handleMouseWheel(float delta) {
    if (hover_component_) {
        hover_component_->handleMouseWheel(delta);
    }
}

void PluginGUI::addParameter(const ParameterInfo& param) {
    parameters_.push_back(param);
}

void PluginGUI::updateParameter(uint32_t param_id, float value) {
    ParameterInfo* param = findParameter(param_id);
    if (param) {
        param->current_value = value;
    }
}

void PluginGUI::addComponent(std::unique_ptr<UIComponent> component) {
    if (component) {
        components_.push_back(std::move(component));
    }
}

void PluginGUI::removeComponent(UIComponent* component) {
    components_.erase(
        std::remove_if(components_.begin(), components_.end(),
            [component](const std::unique_ptr<UIComponent>& ptr) {
                return ptr.get() == component;
            }),
        components_.end());
}

void PluginGUI::clearComponents() {
    components_.clear();
    focused_component_ = nullptr;
    hover_component_ = nullptr;
}

ParameterInfo* PluginGUI::findParameter(uint32_t param_id) {
    for (auto& param : parameters_) {
        if (param.id == param_id) {
            return &param;
        }
    }
    return nullptr;
}

void PluginGUI::setupDefaultLayout() {
    createSoothe2StyleInterface();
}

void PluginGUI::createSoothe2StyleInterface() {
    clearComponents();
    
    // Add some default parameters
    parameters_.clear();
    
    // Left panel controls
    addParameter({0, "DEPTH", 0.0f, 100.0f, 50.0f, 50.0f, "%", nullptr});
    addParameter({1, "FREQUENCY", 20.0f, 20000.0f, 1000.0f, 1000.0f, "Hz", nullptr});
    addParameter({2, "RATIO", 1.0f, 10.0f, 4.0f, 4.0f, ":1", nullptr});
    addParameter({3, "ATTACK", 0.1f, 100.0f, 5.0f, 5.0f, "ms", nullptr});
    addParameter({4, "RELEASE", 1.0f, 1000.0f, 50.0f, 50.0f, "ms", nullptr});
    addParameter({5, "BYPASS", 0.0f, 1.0f, 0.0f, 0.0f, "", nullptr});
    
    // Title
    auto title = std::make_unique<Label>(graphics::Rect(20, 20, 200, 30), "SOOTHE2 STYLE");
    title->setColor(Colors::Accent);
    title->setFontSize(16.0f);
    addComponent(std::move(title));
    
    // Left panel knobs
    float knobSize = 80;
    float knobSpacing = 100;
    float leftPanelX = 40;
    float startY = 80;
    
    // Row 1: Main controls
    auto depthKnob = std::make_unique<Knob>(
        graphics::Rect(leftPanelX, startY, knobSize, knobSize), 
        findParameter(0));
    addComponent(std::move(depthKnob));
    
    auto freqKnob = std::make_unique<Knob>(
        graphics::Rect(leftPanelX + knobSpacing, startY, knobSize, knobSize), 
        findParameter(1));
    addComponent(std::move(freqKnob));
    
    auto ratioKnob = std::make_unique<Knob>(
        graphics::Rect(leftPanelX + knobSpacing * 2, startY, knobSize, knobSize), 
        findParameter(2));
    addComponent(std::move(ratioKnob));
    
    // Row 2: Timing controls
    auto attackKnob = std::make_unique<Knob>(
        graphics::Rect(leftPanelX, startY + 120, knobSize, knobSize), 
        findParameter(3));
    addComponent(std::move(attackKnob));
    
    auto releaseKnob = std::make_unique<Knob>(
        graphics::Rect(leftPanelX + knobSpacing, startY + 120, knobSize, knobSize), 
        findParameter(4));
    addComponent(std::move(releaseKnob));
    
    // Bypass switch
    auto bypassSwitch = std::make_unique<Switch>(
        graphics::Rect(leftPanelX + knobSpacing * 2, startY + 140, 60, 25), 
        findParameter(5));
    addComponent(std::move(bypassSwitch));
    
    // EQ Graph (right side)
    auto eqGraph = std::make_unique<EQGraph>(
        graphics::Rect(width_ * 0.5f, 80, width_ * 0.45f, height_ - 160));
    addComponent(std::move(eqGraph));
    
    // Bottom buttons
    auto undoButton = std::make_unique<Button>(
        graphics::Rect(20, height_ - 60, 60, 30), "UNDO");
    addComponent(std::move(undoButton));
    
    auto redoButton = std::make_unique<Button>(
        graphics::Rect(90, height_ - 60, 60, 30), "REDO");
    addComponent(std::move(redoButton));
    
    auto presetButton = std::make_unique<Button>(
        graphics::Rect(width_ - 120, height_ - 60, 100, 30), "PRESET");
    addComponent(std::move(presetButton));
}

} // namespace gui
} // namespace clap_jules