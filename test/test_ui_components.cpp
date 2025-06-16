#include <gtest/gtest.h>
#include "graphics/ui_components.h"
#include "graphics/soothe2_gui.h"
#include <chrono>

using namespace clap_jules::ui;
using namespace clap_jules::graphics;

class UIComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        graphics = createGraphicsContext(800, 600);
        ASSERT_NE(graphics, nullptr);
    }
    
    std::unique_ptr<GraphicsContext> graphics;
};

TEST_F(UIComponentTest, KnobCreationAndBasicFunctionality) {
    Knob knob(100, 100, 30, 0.0f, 1.0f);
    
    // Test initial state
    EXPECT_FLOAT_EQ(knob.getValue(), 0.0f);
    
    // Test value setting
    knob.setValue(0.5f);
    EXPECT_FLOAT_EQ(knob.getValue(), 0.5f);
    
    // Test value clamping
    knob.setValue(-1.0f);
    EXPECT_FLOAT_EQ(knob.getValue(), 0.0f);
    
    knob.setValue(2.0f);
    EXPECT_FLOAT_EQ(knob.getValue(), 1.0f);
    
    // Test range setting
    knob.setRange(-10.0f, 10.0f);
    knob.setValue(5.0f);
    EXPECT_FLOAT_EQ(knob.getValue(), 5.0f);
}

TEST_F(UIComponentTest, SliderCreationAndBasicFunctionality) {
    Slider slider(50, 200, 200, 20, 0.0f, 100.0f);
    
    // Test initial state
    EXPECT_FLOAT_EQ(slider.getValue(), 0.0f);
    
    // Test value setting
    slider.setValue(50.0f);
    EXPECT_FLOAT_EQ(slider.getValue(), 50.0f);
    
    // Test value clamping
    slider.setValue(-10.0f);
    EXPECT_FLOAT_EQ(slider.getValue(), 0.0f);
    
    slider.setValue(150.0f);
    EXPECT_FLOAT_EQ(slider.getValue(), 100.0f);
}

TEST_F(UIComponentTest, ButtonCreationAndToggling) {
    Button button(100, 300, 80, 30, "Test");
    
    // Test initial state
    EXPECT_FALSE(button.isPressed());
    
    // Test toggle functionality
    button.setToggle(true);
    button.setPressed(true);
    EXPECT_TRUE(button.isPressed());
    
    button.setPressed(false);
    EXPECT_FALSE(button.isPressed());
}

TEST_F(UIComponentTest, EQGraphBasicFunctionality) {
    EQGraph eqGraph(50, 50, 400, 300);
    
    // Test initial state
    EXPECT_GT(eqGraph.getControlPoints().size(), 0);
    
    // Test adding control points
    size_t initialSize = eqGraph.getControlPoints().size();
    eqGraph.addControlPoint(EQPoint(440.0f, 3.0f, 2.0f));
    EXPECT_EQ(eqGraph.getControlPoints().size(), initialSize + 1);
    
    // Test frequency range setting
    eqGraph.setFrequencyRange(10.0f, 30000.0f);
    eqGraph.setGainRange(-30.0f, 30.0f);
    
    // Basic drawing test (should not crash)
    EXPECT_NO_THROW(eqGraph.draw(graphics.get()));
}

TEST_F(UIComponentTest, MouseEventHandling) {
    Knob knob(100, 100, 30, 0.0f, 1.0f);
    
    MouseEvent mouseEvent;
    mouseEvent.pressed = false;  // Start with unpressed
    mouseEvent.dragging = false;
    mouseEvent.button = 0;
    
    // Test hit detection (center of knob should be handled)
    mouseEvent.x = 100;  // Center of knob
    mouseEvent.y = 100;  // Center of knob
    mouseEvent.pressed = true;
    bool handled = knob.handleMouse(mouseEvent);
    EXPECT_TRUE(handled); // Should handle mouse events within bounds
    
    // Reset mouse state
    mouseEvent.pressed = false;
    knob.handleMouse(mouseEvent);
    
    // Test miss detection (far outside knob bounds)
    mouseEvent.x = 300;
    mouseEvent.y = 300;
    mouseEvent.pressed = true;
    handled = knob.handleMouse(mouseEvent);
    EXPECT_FALSE(handled); // Should not handle mouse events outside bounds
}

TEST_F(UIComponentTest, UIContainerManagement) {
    UIContainer container;
    
    // Add components
    auto knob = std::make_unique<Knob>(50, 50, 20, 0.0f, 1.0f);
    auto slider = std::make_unique<Slider>(100, 100, 100, 20, 0.0f, 1.0f);
    
    Knob* knobPtr = knob.get();
    Slider* sliderPtr = slider.get();
    
    container.addComponent(std::move(knob));
    container.addComponent(std::move(slider));
    
    // Test component retrieval
    auto* foundKnob = container.getComponent<Knob>();
    auto* foundSlider = container.getComponent<Slider>();
    
    EXPECT_EQ(foundKnob, knobPtr);
    EXPECT_EQ(foundSlider, sliderPtr);
    
    // Test drawing (should not crash)
    EXPECT_NO_THROW(container.draw(graphics.get()));
}

class Soothe2GUITest : public ::testing::Test {
protected:
    void SetUp() override {
        graphics = createGraphicsContext(800, 600);
        ASSERT_NE(graphics, nullptr);
        
        gui = std::make_unique<Soothe2GUI>(800, 600);
        ASSERT_NE(gui, nullptr);
    }
    
    std::unique_ptr<GraphicsContext> graphics;
    std::unique_ptr<Soothe2GUI> gui;
};

TEST_F(Soothe2GUITest, InitialParameterValues) {
    // Test that parameters are initialized to reasonable defaults
    EXPECT_FLOAT_EQ(gui->params.depth, 0.5f);
    EXPECT_FLOAT_EQ(gui->params.selectivity, 0.5f);
    EXPECT_FLOAT_EQ(gui->params.attack, 0.3f);
    EXPECT_FLOAT_EQ(gui->params.release, 0.7f);
    EXPECT_FLOAT_EQ(gui->params.mix, 1.0f);
    EXPECT_FALSE(gui->params.bypass);
    EXPECT_FALSE(gui->params.solo);
    EXPECT_FALSE(gui->params.delta);
}

TEST_F(Soothe2GUITest, EQBandParameters) {
    // Test EQ band initialization
    EXPECT_GT(gui->params.band1.frequency, 0.0f);
    EXPECT_GT(gui->params.band2.frequency, 0.0f);
    EXPECT_GT(gui->params.band3.frequency, 0.0f);
    EXPECT_GT(gui->params.band4.frequency, 0.0f);
    
    // Test that frequencies are in ascending order
    EXPECT_LT(gui->params.band1.frequency, gui->params.band2.frequency);
    EXPECT_LT(gui->params.band2.frequency, gui->params.band3.frequency);
    EXPECT_LT(gui->params.band3.frequency, gui->params.band4.frequency);
}

TEST_F(Soothe2GUITest, GUIDrawing) {
    // Test that drawing doesn't crash
    EXPECT_NO_THROW(gui->draw(graphics.get()));
}

TEST_F(Soothe2GUITest, GUIUpdate) {
    // Test that updating doesn't crash
    EXPECT_NO_THROW(gui->update(0.016f));
}

TEST_F(Soothe2GUITest, ParameterChangeCallback) {
    bool callbackCalled = false;
    std::string lastParamName;
    float lastParamValue = 0.0f;
    
    gui->setParameterChangeCallback([&](const std::string& name, float value) {
        callbackCalled = true;
        lastParamName = name;
        lastParamValue = value;
    });
    
    // The callback testing would require more complex mouse simulation
    // or direct parameter manipulation which would be implementation specific
    EXPECT_TRUE(true); // Placeholder for callback testing
}

TEST_F(Soothe2GUITest, GUIResizing) {
    // Test resizing functionality
    EXPECT_NO_THROW(gui->resize(1000, 700));
    
    // Drawing should still work after resize
    EXPECT_NO_THROW(gui->draw(graphics.get()));
}

TEST_F(Soothe2GUITest, ColorSchemeTest) {
    // Test that colors are properly defined
    EXPECT_NE(Colors::BACKGROUND_DARK.r, 0);
    EXPECT_NE(Colors::KNOB_OUTER.r, 0);
    EXPECT_NE(Colors::EQ_CURVE.r, 0);
    EXPECT_NE(Colors::TEXT_PRIMARY.r, 0);
}

// Performance test for rendering
TEST_F(Soothe2GUITest, RenderingPerformance) {
    const int numFrames = 100;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numFrames; ++i) {
        gui->update(0.016f);
        gui->draw(graphics.get());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should complete 100 frames in reasonable time (less than 1 second)
    EXPECT_LT(duration.count(), 1000000); // 1 second in microseconds
    
    float averageFrameTime = duration.count() / (float)numFrames;
    std::cout << "Average frame render time: " << averageFrameTime << " microseconds\n";
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}