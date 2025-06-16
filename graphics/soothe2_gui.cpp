#include "soothe2_gui.h"
#include <algorithm>

namespace clap_jules {
namespace ui {

Soothe2GUI::Soothe2GUI(int width, int height) 
    : width_(width), height_(height) {
    setupLayout();
}

void Soothe2GUI::draw(graphics::GraphicsContext* ctx) {
    // Clear background with dark theme
    ctx->clear(Colors::BACKGROUND_DARK);
    
    // Draw main background panel
    ctx->drawRect(graphics::Rect(0, 0, width_, height_), Colors::BACKGROUND_DARK);
    
    // Draw left control panel background
    float leftPanelWidth = width_ * 0.35f;
    ctx->drawRect(graphics::Rect(MARGIN, MARGIN, leftPanelWidth - MARGIN, height_ - 2 * MARGIN), 
                  Colors::BACKGROUND_MEDIUM);
    
    // Draw right EQ panel background
    float rightPanelX = leftPanelWidth + MARGIN;
    float rightPanelWidth = width_ - rightPanelX - MARGIN;
    ctx->drawRect(graphics::Rect(rightPanelX, MARGIN, rightPanelWidth, height_ - 2 * MARGIN), 
                  Colors::BACKGROUND_MEDIUM);
    
    // Draw section separators
    float separatorX = leftPanelWidth;
    ctx->drawLine(graphics::Point(separatorX, MARGIN), 
                  graphics::Point(separatorX, height_ - MARGIN), 
                  Colors::BACKGROUND_LIGHT, 2.0f);
    
    // Draw all UI components
    container_.draw(ctx);
    
    // Draw title bar
    ctx->drawRect(graphics::Rect(0, 0, width_, 35), Colors::BACKGROUND_LIGHT);
    ctx->drawText("Soothe2 - Dynamic EQ Suppressor", 
                  graphics::Point(MARGIN, 22), 
                  Colors::TEXT_PRIMARY, 16.0f);
    
    // Draw version/info
    ctx->drawText("v1.0", 
                  graphics::Point(width_ - 50, 22), 
                  Colors::TEXT_SECONDARY, 12.0f);
}

bool Soothe2GUI::handleMouse(const MouseEvent& event) {
    return container_.handleMouse(event);
}

void Soothe2GUI::update(float deltaTime) {
    container_.update(deltaTime);
}

void Soothe2GUI::resize(int width, int height) {
    width_ = width;
    height_ = height;
    setupLayout(); // Recreate layout with new dimensions
}

void Soothe2GUI::setupLayout() {
    // Clear existing components by creating a new container
    container_ = UIContainer{};
    
    setupMainControls();
    setupEQSection();
    setupButtons();
    setupLabels();
}

void Soothe2GUI::setupMainControls() {
    float leftPanelWidth = width_ * 0.35f;
    float controlAreaY = 50; // Start below title bar
    float currentY = controlAreaY;
    
    // Main control section
    float knobX1 = MARGIN + 30;
    float knobX2 = MARGIN + 120;
    float knobX3 = MARGIN + 210;
    
    // Row 1: Primary controls
    depthKnob_ = std::make_unique<Knob>(knobX1, currentY, KNOB_SIZE, 0.0f, 1.0f);
    depthKnob_->setValue(params.depth);
    depthKnob_->setLabel("DEPTH");
    depthKnob_->setColors(Colors::KNOB_OUTER, Colors::KNOB_INNER, Colors::ACCENT_ORANGE);
    depthKnob_->setOnValueChanged([this](float value) {
        params.depth = value;
        onParameterChanged("depth", value);
    });
    container_.addComponent(std::move(depthKnob_));
    
    selectivityKnob_ = std::make_unique<Knob>(knobX2, currentY, KNOB_SIZE, 0.0f, 1.0f);
    selectivityKnob_->setValue(params.selectivity);
    selectivityKnob_->setLabel("SELECTIVITY");
    selectivityKnob_->setColors(Colors::KNOB_OUTER, Colors::KNOB_INNER, Colors::ACCENT_BLUE);
    selectivityKnob_->setOnValueChanged([this](float value) {
        params.selectivity = value;
        onParameterChanged("selectivity", value);
    });
    container_.addComponent(std::move(selectivityKnob_));
    
    currentY += KNOB_SIZE * 2 + SECTION_SPACING;
    
    // Row 2: Timing controls
    attackKnob_ = std::make_unique<Knob>(knobX1, currentY, KNOB_SIZE, 0.0f, 1.0f);
    attackKnob_->setValue(params.attack);
    attackKnob_->setLabel("ATTACK");
    attackKnob_->setColors(Colors::KNOB_OUTER, Colors::KNOB_INNER, Colors::ACCENT_GREEN);
    attackKnob_->setOnValueChanged([this](float value) {
        params.attack = value;
        onParameterChanged("attack", value);
    });
    container_.addComponent(std::move(attackKnob_));
    
    releaseKnob_ = std::make_unique<Knob>(knobX2, currentY, KNOB_SIZE, 0.0f, 1.0f);
    releaseKnob_->setValue(params.release);
    releaseKnob_->setLabel("RELEASE");
    releaseKnob_->setColors(Colors::KNOB_OUTER, Colors::KNOB_INNER, Colors::ACCENT_GREEN);
    releaseKnob_->setOnValueChanged([this](float value) {
        params.release = value;
        onParameterChanged("release", value);
    });
    container_.addComponent(std::move(releaseKnob_));
    
    currentY += KNOB_SIZE * 2 + SECTION_SPACING;
    
    // Row 3: Mix and advanced controls
    mixKnob_ = std::make_unique<Knob>(knobX1, currentY, KNOB_SIZE, 0.0f, 1.0f);
    mixKnob_->setValue(params.mix);
    mixKnob_->setLabel("MIX");
    mixKnob_->setColors(Colors::KNOB_OUTER, Colors::KNOB_INNER, Colors::ACCENT_BLUE);
    mixKnob_->setOnValueChanged([this](float value) {
        params.mix = value;
        onParameterChanged("mix", value);
    });
    container_.addComponent(std::move(mixKnob_));
    
    sharpnessKnob_ = std::make_unique<Knob>(knobX2, currentY, KNOB_SIZE, 0.0f, 1.0f);
    sharpnessKnob_->setValue(params.sharpness);
    sharpnessKnob_->setLabel("SHARPNESS");
    sharpnessKnob_->setColors(Colors::KNOB_OUTER, Colors::KNOB_INNER, Colors::ACCENT_ORANGE);
    sharpnessKnob_->setOnValueChanged([this](float value) {
        params.sharpness = value;
        onParameterChanged("sharpness", value);
    });
    container_.addComponent(std::move(sharpnessKnob_));
}

void Soothe2GUI::setupEQSection() {
    float leftPanelWidth = width_ * 0.35f;
    float rightPanelX = leftPanelWidth + MARGIN * 2;
    float rightPanelWidth = width_ - rightPanelX - MARGIN;
    float eqHeight = height_ - 100; // Leave space for controls below
    
    // Create EQ graph
    eqGraph_ = std::make_unique<EQGraph>(rightPanelX, 50, rightPanelWidth, eqHeight);
    eqGraph_->setFrequencyRange(20.0f, 20000.0f);
    eqGraph_->setGainRange(-24.0f, 24.0f);
    
    // Add EQ bands as control points
    eqGraph_->addControlPoint(EQPoint(params.band1.frequency, params.band1.gain, params.band1.q));
    eqGraph_->addControlPoint(EQPoint(params.band2.frequency, params.band2.gain, params.band2.q));
    eqGraph_->addControlPoint(EQPoint(params.band3.frequency, params.band3.gain, params.band3.q));
    eqGraph_->addControlPoint(EQPoint(params.band4.frequency, params.band4.gain, params.band4.q));
    
    eqGraph_->setOnPointChanged([this](int index, const EQPoint& point) {
        updateParametersFromEQ();
        onParameterChanged("eq_band_" + std::to_string(index), point.gain);
    });
    
    container_.addComponent(std::move(eqGraph_));
}

void Soothe2GUI::setupButtons() {
    float leftPanelWidth = width_ * 0.35f;
    float buttonY = height_ - 80;
    float buttonSpacing = BUTTON_WIDTH + 10;
    
    // Left panel buttons
    float buttonX = MARGIN + 20;
    
    bypassButton_ = std::make_unique<Button>(buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, "BYPASS");
    bypassButton_->setToggle(true);
    bypassButton_->setPressed(params.bypass);
    bypassButton_->setOnClick([this]() {
        params.bypass = !params.bypass;
        onParameterChanged("bypass", params.bypass ? 1.0f : 0.0f);
    });
    container_.addComponent(std::move(bypassButton_));
    
    buttonX += buttonSpacing;
    soloButton_ = std::make_unique<Button>(buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, "SOLO");
    soloButton_->setToggle(true);
    soloButton_->setPressed(params.solo);
    soloButton_->setOnClick([this]() {
        params.solo = !params.solo;
        onParameterChanged("solo", params.solo ? 1.0f : 0.0f);
    });
    container_.addComponent(std::move(soloButton_));
    
    buttonX += buttonSpacing;
    deltaButton_ = std::make_unique<Button>(buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, "DELTA");
    deltaButton_->setToggle(true);
    deltaButton_->setPressed(params.delta);
    deltaButton_->setOnClick([this]() {
        params.delta = !params.delta;
        onParameterChanged("delta", params.delta ? 1.0f : 0.0f);
    });
    container_.addComponent(std::move(deltaButton_));
    
    // Right panel button
    float rightButtonX = leftPanelWidth + MARGIN + 20;
    softKneeButton_ = std::make_unique<Button>(rightButtonX, buttonY, BUTTON_WIDTH + 20, BUTTON_HEIGHT, "SOFT KNEE");
    softKneeButton_->setToggle(true);
    softKneeButton_->setPressed(params.softKnee);
    softKneeButton_->setOnClick([this]() {
        params.softKnee = !params.softKnee;
        onParameterChanged("softKnee", params.softKnee ? 1.0f : 0.0f);
    });
    container_.addComponent(std::move(softKneeButton_));
}

void Soothe2GUI::setupLabels() {
    // Main title is drawn directly in draw() method
    
    // Section labels
    auto sectionLabel1 = std::make_unique<Label>(MARGIN + 20, 40, "DYNAMICS", 14.0f);
    sectionLabel1->setColor(Colors::TEXT_PRIMARY);
    container_.addComponent(std::move(sectionLabel1));
    
    float leftPanelWidth = width_ * 0.35f;
    auto sectionLabel2 = std::make_unique<Label>(leftPanelWidth + MARGIN + 20, 40, "FREQUENCY RESPONSE", 14.0f);
    sectionLabel2->setColor(Colors::TEXT_PRIMARY);
    container_.addComponent(std::move(sectionLabel2));
    
    // Parameter value displays
    auto depthValueLabel = std::make_unique<Label>(MARGIN + 75, height_ - 140, "", 10.0f);
    depthValueLabel->setColor(Colors::TEXT_SECONDARY);
    container_.addComponent(std::move(depthValueLabel));
}

void Soothe2GUI::onParameterChanged(const std::string& paramName, float value) {
    if (parameterChangeCallback_) {
        parameterChangeCallback_(paramName, value);
    }
}

void Soothe2GUI::updateEQFromParameters() {
    if (!eqGraph_) return;
    
    // Update EQ control points from parameter values
    auto& points = const_cast<std::vector<EQPoint>&>(eqGraph_->getControlPoints());
    if (points.size() >= 4) {
        points[0] = EQPoint(params.band1.frequency, params.band1.gain, params.band1.q);
        points[1] = EQPoint(params.band2.frequency, params.band2.gain, params.band2.q);
        points[2] = EQPoint(params.band3.frequency, params.band3.gain, params.band3.q);
        points[3] = EQPoint(params.band4.frequency, params.band4.gain, params.band4.q);
    }
}

void Soothe2GUI::updateParametersFromEQ() {
    if (!eqGraph_) return;
    
    // Update parameter values from EQ control points
    const auto& points = eqGraph_->getControlPoints();
    if (points.size() >= 4) {
        params.band1.frequency = points[0].frequency;
        params.band1.gain = points[0].gain;
        params.band1.q = points[0].q;
        
        params.band2.frequency = points[1].frequency;
        params.band2.gain = points[1].gain;
        params.band2.q = points[1].q;
        
        params.band3.frequency = points[2].frequency;
        params.band3.gain = points[2].gain;
        params.band3.q = points[2].q;
        
        params.band4.frequency = points[3].frequency;
        params.band4.gain = points[3].gain;
        params.band4.q = points[3].q;
    }
}

} // namespace ui
} // namespace clap_jules