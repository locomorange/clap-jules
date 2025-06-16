#include "ui_components.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>

namespace clap_jules {
namespace ui {

// Helper function to clamp values
template<typename T>
T clamp(T value, T min, T max) {
    return std::max(min, std::min(max, value));
}

// Helper function to convert degrees to radians
float degToRad(float degrees) {
    return degrees * M_PI / 180.0f;
}

// Helper function for logarithmic frequency scaling
float logScale(float value, float min, float max) {
    if (min <= 0 || max <= 0) return value;
    return std::log(value / min) / std::log(max / min);
}

float invLogScale(float value, float min, float max) {
    if (min <= 0 || max <= 0) return value;
    return min * std::pow(max / min, value);
}

//==============================================================================
// UIComponent Base Class
//==============================================================================

UIComponent::UIComponent(float x, float y, float width, float height)
    : x_(x), y_(y), width_(width), height_(height) {}

bool UIComponent::contains(float x, float y) const {
    return x >= x_ && x <= x_ + width_ && y >= y_ && y <= y_ + height_;
}

//==============================================================================
// Knob Implementation
//==============================================================================

Knob::Knob(float x, float y, float radius, float minValue, float maxValue)
    : UIComponent(x - radius, y - radius, radius * 2, radius * 2)
    , radius_(radius)
    , value_(minValue)
    , minValue_(minValue)
    , maxValue_(maxValue) {}

void Knob::draw(graphics::GraphicsContext* ctx) {
    if (!visible_) return;
    
    float centerX = x_ + radius_;
    float centerY = y_ + radius_;
    
    // Draw outer ring
    ctx->drawCircle(graphics::Point(centerX, centerY), radius_, outerColor_);
    
    // Draw inner circle
    float innerRadius = radius_ * 0.7f;
    ctx->drawCircle(graphics::Point(centerX, centerY), innerRadius, innerColor_);
    
    // Draw value indicator
    float normalizedValue = (value_ - minValue_) / (maxValue_ - minValue_);
    float angle = -135.0f + normalizedValue * 270.0f; // -135° to +135°
    float angleRad = degToRad(angle);
    
    float indicatorLength = radius_ * 0.8f;
    float endX = centerX + std::cos(angleRad) * indicatorLength;
    float endY = centerY + std::sin(angleRad) * indicatorLength;
    
    ctx->drawLine(graphics::Point(centerX, centerY), 
                  graphics::Point(endX, endY), 
                  indicatorColor_, 2.0f);
    
    // Draw small center dot
    ctx->drawCircle(graphics::Point(centerX, centerY), 3.0f, indicatorColor_);
    
    // Draw label below knob
    if (!label_.empty()) {
        ctx->drawText(label_, 
                     graphics::Point(centerX - 20, centerY + radius_ + 15), 
                     Colors::TEXT_LABEL, 10.0f);
    }
    
    // Draw value text
    char valueText[16];
    snprintf(valueText, sizeof(valueText), "%.2f", value_);
    ctx->drawText(valueText, 
                 graphics::Point(centerX - 15, centerY + radius_ + 28), 
                 Colors::TEXT_SECONDARY, 9.0f);
}

bool Knob::handleMouse(const MouseEvent& event) {
    if (!visible_) return false;
    
    float centerX = x_ + radius_;
    float centerY = y_ + radius_;
    float dx = event.x - centerX;
    float dy = event.y - centerY;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance <= radius_) {
        hovered_ = true;
        
        if (event.pressed && !dragging_) {
            dragging_ = true;
            lastMouseY_ = event.y;
            return true;
        }
    } else {
        hovered_ = false;
    }
    
    if (dragging_) {
        if (event.pressed) {
            // Adjust value based on vertical mouse movement
            float deltaY = lastMouseY_ - event.y; // Inverted for natural feel
            float sensitivity = (maxValue_ - minValue_) / 200.0f; // 200 pixels for full range
            float newValue = value_ + deltaY * sensitivity;
            
            setValue(newValue);
            lastMouseY_ = event.y;
            return true;
        } else {
            dragging_ = false;
        }
    }
    
    return hovered_;
}

void Knob::setValue(float value) {
    float newValue = clamp(value, minValue_, maxValue_);
    if (newValue != value_) {
        value_ = newValue;
        if (onValueChanged_) {
            onValueChanged_(value_);
        }
    }
}

void Knob::setRange(float minValue, float maxValue) {
    minValue_ = minValue;
    maxValue_ = maxValue;
    setValue(value_); // Clamp current value to new range
}

void Knob::setColors(const graphics::Color& outer, const graphics::Color& inner, const graphics::Color& indicator) {
    outerColor_ = outer;
    innerColor_ = inner;
    indicatorColor_ = indicator;
}

//==============================================================================
// Slider Implementation
//==============================================================================

Slider::Slider(float x, float y, float width, float height, float minValue, float maxValue)
    : UIComponent(x, y, width, height)
    , value_(minValue)
    , minValue_(minValue)
    , maxValue_(maxValue) {}

void Slider::draw(graphics::GraphicsContext* ctx) {
    if (!visible_) return;
    
    // Draw slider track
    float trackY = y_ + height_ / 2;
    ctx->drawLine(graphics::Point(x_, trackY), 
                  graphics::Point(x_ + width_, trackY), 
                  Colors::BACKGROUND_LIGHT, 2.0f);
    
    // Draw slider thumb
    float normalizedValue = (value_ - minValue_) / (maxValue_ - minValue_);
    float thumbX = x_ + normalizedValue * width_;
    float thumbRadius = 8.0f;
    
    ctx->drawCircle(graphics::Point(thumbX, trackY), thumbRadius, Colors::KNOB_OUTER);
    ctx->drawCircle(graphics::Point(thumbX, trackY), thumbRadius * 0.7f, Colors::KNOB_INNER);
    
    // Draw active track
    ctx->drawLine(graphics::Point(x_, trackY), 
                  graphics::Point(thumbX, trackY), 
                  Colors::ACCENT_BLUE, 2.0f);
    
    // Draw label
    if (!label_.empty()) {
        ctx->drawText(label_, 
                     graphics::Point(x_, y_ - 15), 
                     Colors::TEXT_LABEL, 10.0f);
    }
    
    // Draw value
    char valueText[16];
    snprintf(valueText, sizeof(valueText), "%.2f", value_);
    ctx->drawText(valueText, 
                 graphics::Point(x_ + width_ - 30, y_ - 15), 
                 Colors::TEXT_SECONDARY, 9.0f);
}

bool Slider::handleMouse(const MouseEvent& event) {
    if (!visible_) return false;
    
    if (contains(event.x, event.y)) {
        hovered_ = true;
        
        if (event.pressed) {
            dragging_ = true;
            
            // Calculate new value based on mouse position
            float normalizedX = (event.x - x_) / width_;
            normalizedX = clamp(normalizedX, 0.0f, 1.0f);
            float newValue = minValue_ + normalizedX * (maxValue_ - minValue_);
            setValue(newValue);
            return true;
        }
    } else {
        hovered_ = false;
    }
    
    if (dragging_) {
        if (event.pressed) {
            // Continue dragging
            float normalizedX = (event.x - x_) / width_;
            normalizedX = clamp(normalizedX, 0.0f, 1.0f);
            float newValue = minValue_ + normalizedX * (maxValue_ - minValue_);
            setValue(newValue);
            return true;
        } else {
            dragging_ = false;
        }
    }
    
    return hovered_;
}

void Slider::setValue(float value) {
    float newValue = clamp(value, minValue_, maxValue_);
    if (newValue != value_) {
        value_ = newValue;
        if (onValueChanged_) {
            onValueChanged_(value_);
        }
    }
}

void Slider::setRange(float minValue, float maxValue) {
    minValue_ = minValue;
    maxValue_ = maxValue;
    setValue(value_); // Clamp current value to new range
}

//==============================================================================
// Button Implementation
//==============================================================================

Button::Button(float x, float y, float width, float height, const std::string& text)
    : UIComponent(x, y, width, height), text_(text) {}

void Button::draw(graphics::GraphicsContext* ctx) {
    if (!visible_) return;
    
    // Choose colors based on state
    graphics::Color bgColor = Colors::BACKGROUND_MEDIUM;
    graphics::Color textColor = Colors::TEXT_PRIMARY;
    
    if (toggleState_ || pressed_) {
        bgColor = Colors::ACCENT_BLUE;
        textColor = graphics::Color(255, 255, 255);
    } else if (hovered_) {
        bgColor = Colors::BACKGROUND_LIGHT;
    }
    
    // Draw button background
    ctx->drawRect(graphics::Rect(x_, y_, width_, height_), bgColor);
    
    // Draw button border
    ctx->drawLine(graphics::Point(x_, y_), graphics::Point(x_ + width_, y_), Colors::BACKGROUND_LIGHT, 1.0f);
    ctx->drawLine(graphics::Point(x_, y_), graphics::Point(x_, y_ + height_), Colors::BACKGROUND_LIGHT, 1.0f);
    ctx->drawLine(graphics::Point(x_ + width_, y_), graphics::Point(x_ + width_, y_ + height_), Colors::BACKGROUND_DARK, 1.0f);
    ctx->drawLine(graphics::Point(x_, y_ + height_), graphics::Point(x_ + width_, y_ + height_), Colors::BACKGROUND_DARK, 1.0f);
    
    // Draw button text
    float textX = x_ + width_ / 2 - text_.length() * 3; // Rough centering
    float textY = y_ + height_ / 2 + 4;
    ctx->drawText(text_, graphics::Point(textX, textY), textColor, 12.0f);
}

bool Button::handleMouse(const MouseEvent& event) {
    if (!visible_) return false;
    
    if (contains(event.x, event.y)) {
        hovered_ = true;
        
        if (event.pressed && !pressed_) {
            pressed_ = true;
            return true;
        } else if (!event.pressed && pressed_) {
            pressed_ = false;
            
            if (isToggle_) {
                toggleState_ = !toggleState_;
            }
            
            if (onClick_) {
                onClick_();
            }
            return true;
        }
    } else {
        hovered_ = false;
        if (!event.pressed) {
            pressed_ = false;
        }
    }
    
    return hovered_;
}

//==============================================================================
// Label Implementation
//==============================================================================

Label::Label(float x, float y, const std::string& text, float fontSize)
    : UIComponent(x, y, 0, 0), text_(text), fontSize_(fontSize) {}

void Label::draw(graphics::GraphicsContext* ctx) {
    if (!visible_) return;
    
    float textX = x_;
    if (alignment_ == 1) { // Center
        textX = x_ - text_.length() * fontSize_ * 0.3f; // Rough centering
    } else if (alignment_ == 2) { // Right
        textX = x_ - text_.length() * fontSize_ * 0.6f; // Right align
    }
    
    ctx->drawText(text_, graphics::Point(textX, y_), color_, fontSize_);
}

//==============================================================================
// EQGraph Implementation
//==============================================================================

EQGraph::EQGraph(float x, float y, float width, float height)
    : UIComponent(x, y, width, height) {
    // Add some default control points
    controlPoints_.push_back(EQPoint(100.0f, 0.0f, 1.0f));
    controlPoints_.push_back(EQPoint(1000.0f, 0.0f, 1.0f));
    controlPoints_.push_back(EQPoint(10000.0f, 0.0f, 1.0f));
}

void EQGraph::draw(graphics::GraphicsContext* ctx) {
    if (!visible_) return;
    
    // Draw background
    ctx->drawRect(graphics::Rect(x_, y_, width_, height_), Colors::EQ_BACKGROUND);
    
    // Draw grid and labels
    drawGrid(ctx);
    
    // Draw frequency response curve
    drawCurve(ctx);
    
    // Draw control points
    drawControlPoints(ctx);
    
    // Draw border
    ctx->drawLine(graphics::Point(x_, y_), graphics::Point(x_ + width_, y_), Colors::EQ_GRID, 1.0f);
    ctx->drawLine(graphics::Point(x_, y_), graphics::Point(x_, y_ + height_), Colors::EQ_GRID, 1.0f);
    ctx->drawLine(graphics::Point(x_ + width_, y_), graphics::Point(x_ + width_, y_ + height_), Colors::EQ_GRID, 1.0f);
    ctx->drawLine(graphics::Point(x_, y_ + height_), graphics::Point(x_ + width_, y_ + height_), Colors::EQ_GRID, 1.0f);
}

bool EQGraph::handleMouse(const MouseEvent& event) {
    if (!visible_) return false;
    
    if (contains(event.x, event.y)) {
        if (event.pressed && !dragging_) {
            // Find closest control point
            draggedPoint_ = findClosestPoint(event.x, event.y);
            if (draggedPoint_ >= 0) {
                dragging_ = true;
                lastMousePos_ = graphics::Point(event.x, event.y);
                return true;
            }
        }
    }
    
    if (dragging_ && draggedPoint_ >= 0) {
        if (event.pressed) {
            // Update control point position
            float freq = xToFreq(event.x - x_);
            float gain = yToGain(event.y - y_);
            
            freq = clamp(freq, minFreq_, maxFreq_);
            gain = clamp(gain, minGain_, maxGain_);
            
            controlPoints_[draggedPoint_].frequency = freq;
            controlPoints_[draggedPoint_].gain = gain;
            
            if (onPointChanged_) {
                onPointChanged_(draggedPoint_, controlPoints_[draggedPoint_]);
            }
            
            lastMousePos_ = graphics::Point(event.x, event.y);
            return true;
        } else {
            dragging_ = false;
            draggedPoint_ = -1;
        }
    }
    
    return contains(event.x, event.y);
}

void EQGraph::addControlPoint(const EQPoint& point) {
    controlPoints_.push_back(point);
}

void EQGraph::removeControlPoint(int index) {
    if (index >= 0 && index < controlPoints_.size()) {
        controlPoints_.erase(controlPoints_.begin() + index);
    }
}

void EQGraph::setControlPoint(int index, const EQPoint& point) {
    if (index >= 0 && index < controlPoints_.size()) {
        controlPoints_[index] = point;
    }
}

void EQGraph::setFrequencyRange(float minFreq, float maxFreq) {
    minFreq_ = minFreq;
    maxFreq_ = maxFreq;
}

void EQGraph::setGainRange(float minGain, float maxGain) {
    minGain_ = minGain;
    maxGain_ = maxGain;
}

float EQGraph::freqToX(float frequency) const {
    float logNorm = logScale(frequency, minFreq_, maxFreq_);
    return logNorm * width_;
}

float EQGraph::gainToY(float gain) const {
    float norm = (gain - minGain_) / (maxGain_ - minGain_);
    return height_ * (1.0f - norm); // Invert Y axis
}

float EQGraph::xToFreq(float x) const {
    float logNorm = x / width_;
    return invLogScale(logNorm, minFreq_, maxFreq_);
}

float EQGraph::yToGain(float y) const {
    float norm = 1.0f - (y / height_); // Invert Y axis
    return minGain_ + norm * (maxGain_ - minGain_);
}

void EQGraph::drawGrid(graphics::GraphicsContext* ctx) {
    // Draw frequency grid lines
    std::vector<float> frequencies = {50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    for (float freq : frequencies) {
        if (freq >= minFreq_ && freq <= maxFreq_) {
            float x = x_ + freqToX(freq);
            ctx->drawLine(graphics::Point(x, y_), graphics::Point(x, y_ + height_), Colors::EQ_GRID, 1.0f);
            
            // Draw frequency label
            char freqText[16];
            if (freq < 1000) {
                snprintf(freqText, sizeof(freqText), "%.0f", freq);
            } else {
                snprintf(freqText, sizeof(freqText), "%.1fk", freq / 1000.0f);
            }
            ctx->drawText(freqText, graphics::Point(x - 10, y_ + height_ + 12), Colors::TEXT_LABEL, 8.0f);
        }
    }
    
    // Draw gain grid lines
    for (float gain = minGain_; gain <= maxGain_; gain += 6.0f) {
        float y = y_ + gainToY(gain);
        ctx->drawLine(graphics::Point(x_, y), graphics::Point(x_ + width_, y), Colors::EQ_GRID, 1.0f);
        
        // Draw gain label
        char gainText[16];
        snprintf(gainText, sizeof(gainText), "%.0fdB", gain);
        ctx->drawText(gainText, graphics::Point(x_ - 30, y + 3), Colors::TEXT_LABEL, 8.0f);
    }
    
    // Draw 0dB reference line
    float zeroY = y_ + gainToY(0.0f);
    ctx->drawLine(graphics::Point(x_, zeroY), graphics::Point(x_ + width_, zeroY), Colors::TEXT_SECONDARY, 1.5f);
}

void EQGraph::drawCurve(graphics::GraphicsContext* ctx) {
    // Draw the frequency response curve
    const int numPoints = 200;
    std::vector<graphics::Point> curvePoints;
    
    for (int i = 0; i < numPoints; ++i) {
        float x = (float)i / (numPoints - 1) * width_;
        float freq = xToFreq(x);
        float response = calculateResponse(freq);
        float y = gainToY(response);
        
        curvePoints.push_back(graphics::Point(x_ + x, y_ + y));
    }
    
    // Draw the curve as connected line segments
    for (int i = 0; i < curvePoints.size() - 1; ++i) {
        ctx->drawLine(curvePoints[i], curvePoints[i + 1], Colors::EQ_CURVE, 2.0f);
    }
}

void EQGraph::drawControlPoints(graphics::GraphicsContext* ctx) {
    for (int i = 0; i < controlPoints_.size(); ++i) {
        const EQPoint& point = controlPoints_[i];
        if (!point.active) continue;
        
        float x = x_ + freqToX(point.frequency);
        float y = y_ + gainToY(point.gain);
        
        graphics::Color pointColor = Colors::EQ_CONTROL_POINT;
        if (i == draggedPoint_ || (contains(lastMousePos_.x, lastMousePos_.y) && 
                                  findClosestPoint(lastMousePos_.x, lastMousePos_.y) == i)) {
            pointColor = Colors::EQ_CONTROL_POINT_HOVER;
        }
        
        // Draw control point
        ctx->drawCircle(graphics::Point(x, y), 6.0f, pointColor);
        ctx->drawCircle(graphics::Point(x, y), 3.0f, graphics::Color(255, 255, 255));
        
        // Draw frequency and gain labels
        char freqText[16];
        if (point.frequency < 1000) {
            snprintf(freqText, sizeof(freqText), "%.0f Hz", point.frequency);
        } else {
            snprintf(freqText, sizeof(freqText), "%.1f kHz", point.frequency / 1000.0f);
        }
        ctx->drawText(freqText, graphics::Point(x - 20, y - 15), Colors::TEXT_SECONDARY, 9.0f);
        
        char gainText[16];
        snprintf(gainText, sizeof(gainText), "%.1f dB", point.gain);
        ctx->drawText(gainText, graphics::Point(x - 15, y + 20), Colors::TEXT_SECONDARY, 9.0f);
    }
}

int EQGraph::findClosestPoint(float x, float y) const {
    int closest = -1;
    float minDistance = 20.0f; // Minimum distance for selection
    
    for (int i = 0; i < controlPoints_.size(); ++i) {
        const EQPoint& point = controlPoints_[i];
        if (!point.active) continue;
        
        float px = x_ + freqToX(point.frequency);
        float py = y_ + gainToY(point.gain);
        
        float dx = x - px;
        float dy = y - py;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance < minDistance) {
            minDistance = distance;
            closest = i;
        }
    }
    
    return closest;
}

float EQGraph::calculateResponse(float frequency) const {
    float totalResponse = 0.0f;
    
    for (const EQPoint& point : controlPoints_) {
        if (!point.active) continue;
        
        // Simple bell curve approximation for EQ response
        float freqRatio = frequency / point.frequency;
        float logRatio = std::log2(freqRatio);
        float bandwidth = 1.0f / point.q;
        
        // Gaussian-like curve
        float response = point.gain * std::exp(-(logRatio * logRatio) / (2.0f * bandwidth * bandwidth));
        totalResponse += response;
    }
    
    return totalResponse;
}

//==============================================================================
// UIContainer Implementation
//==============================================================================

void UIContainer::addComponent(std::unique_ptr<UIComponent> component) {
    components_.push_back(std::move(component));
}

void UIContainer::removeComponent(UIComponent* component) {
    components_.erase(
        std::remove_if(components_.begin(), components_.end(),
            [component](const std::unique_ptr<UIComponent>& ptr) {
                return ptr.get() == component;
            }),
        components_.end());
}

void UIContainer::draw(graphics::GraphicsContext* ctx) {
    for (auto& component : components_) {
        component->draw(ctx);
    }
}

bool UIContainer::handleMouse(const MouseEvent& event) {
    // Handle in reverse order so top components get priority
    for (auto it = components_.rbegin(); it != components_.rend(); ++it) {
        if ((*it)->handleMouse(event)) {
            return true; // Event was handled
        }
    }
    return false;
}

void UIContainer::update(float deltaTime) {
    for (auto& component : components_) {
        component->update(deltaTime);
    }
}

} // namespace ui
} // namespace clap_jules