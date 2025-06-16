#include <gtest/gtest.h>
#include "graphics/plugin_gui.h"
#include <memory>

using namespace clap_jules::gui;
using namespace clap_jules::graphics;

class PluginGUITest : public ::testing::Test {
protected:
    void SetUp() override {
        gui = std::make_unique<PluginGUI>(800, 600);
    }

    void TearDown() override {
        gui.reset();
    }

    std::unique_ptr<PluginGUI> gui;
};

TEST_F(PluginGUITest, BasicCreation) {
    EXPECT_TRUE(gui != nullptr);
    
    uint32_t width, height;
    EXPECT_TRUE(gui->getSize(&width, &height));
    EXPECT_EQ(width, 800);
    EXPECT_EQ(height, 600);
}

TEST_F(PluginGUITest, GUILifecycle) {
    // Test creation
    EXPECT_TRUE(gui->create("test", false));
    
    // Test show/hide
    EXPECT_TRUE(gui->show());
    EXPECT_TRUE(gui->hide());
    
    // Test size changes
    EXPECT_TRUE(gui->setSize(1024, 768));
    
    uint32_t width, height;
    EXPECT_TRUE(gui->getSize(&width, &height));
    EXPECT_EQ(width, 1024);
    EXPECT_EQ(height, 768);
    
    // Test destruction
    gui->destroy();
}

TEST_F(PluginGUITest, ParameterManagement) {
    ParameterInfo param;
    param.id = 1;
    param.name = "Test Param";
    param.min_value = 0.0f;
    param.max_value = 100.0f;
    param.default_value = 50.0f;
    param.current_value = 50.0f;
    param.unit = "%";
    param.callback = nullptr;
    
    gui->addParameter(param);
    gui->updateParameter(1, 75.0f);
    
    // Basic test - just ensure no crashes
    EXPECT_TRUE(true);
}

TEST_F(PluginGUITest, ComponentCreation) {
    // Test knob creation
    ParameterInfo param = {0, "Knob", 0.0f, 100.0f, 50.0f, 50.0f, "%", nullptr};
    auto knob = std::make_unique<Knob>(Rect(10, 10, 80, 80), &param);
    EXPECT_TRUE(knob != nullptr);
    
    // Test switch creation
    ParameterInfo switch_param = {1, "Switch", 0.0f, 1.0f, 0.0f, 0.0f, "", nullptr};
    auto switch_comp = std::make_unique<Switch>(Rect(100, 10, 60, 25), &switch_param);
    EXPECT_TRUE(switch_comp != nullptr);
    
    // Test EQ graph creation
    auto eq_graph = std::make_unique<EQGraph>(Rect(200, 50, 400, 300));
    EXPECT_TRUE(eq_graph != nullptr);
    
    // Test button creation
    auto button = std::make_unique<Button>(Rect(50, 400, 80, 30), "Test");
    EXPECT_TRUE(button != nullptr);
    
    // Test label creation
    auto label = std::make_unique<Label>(Rect(50, 450, 100, 20), "Test Label");
    EXPECT_TRUE(label != nullptr);
}

TEST_F(PluginGUITest, KnobValues) {
    ParameterInfo param = {0, "Test", 0.0f, 100.0f, 50.0f, 50.0f, "%", nullptr};
    Knob knob(Rect(0, 0, 80, 80), &param);
    
    EXPECT_FLOAT_EQ(knob.getValue(), 50.0f);
    
    knob.setValue(75.0f);
    EXPECT_FLOAT_EQ(knob.getValue(), 75.0f);
    
    // Test bounds
    knob.setValue(150.0f); // Should clamp to max
    EXPECT_FLOAT_EQ(knob.getValue(), 100.0f);
    
    knob.setValue(-10.0f); // Should clamp to min
    EXPECT_FLOAT_EQ(knob.getValue(), 0.0f);
}

TEST_F(PluginGUITest, SwitchValues) {
    ParameterInfo param = {0, "Test", 0.0f, 1.0f, 0.0f, 0.0f, "", nullptr};
    Switch switch_comp(Rect(0, 0, 60, 25), &param);
    
    EXPECT_FALSE(switch_comp.getValue());
    
    switch_comp.setValue(true);
    EXPECT_TRUE(switch_comp.getValue());
    
    switch_comp.setValue(false);
    EXPECT_FALSE(switch_comp.getValue());
}

TEST_F(PluginGUITest, EQGraphPoints) {
    EQGraph eq_graph(Rect(0, 0, 400, 300));
    
    // Test adding points
    eq_graph.addPoint(EQGraph::EQPoint(440.0f, 3.0f, 1.5f));
    eq_graph.addPoint(EQGraph::EQPoint(1000.0f, -2.0f, 2.0f));
    
    EXPECT_EQ(eq_graph.getPoints().size(), 5); // 3 default + 2 added
    
    // Test clearing points
    eq_graph.clearPoints();
    EXPECT_EQ(eq_graph.getPoints().size(), 0);
}

TEST_F(PluginGUITest, ColorScheme) {
    // Test that colors are defined and valid
    EXPECT_EQ(Colors::Background.r, 28);
    EXPECT_EQ(Colors::Background.g, 28);
    EXPECT_EQ(Colors::Background.b, 28);
    EXPECT_EQ(Colors::Background.a, 255);
    
    EXPECT_EQ(Colors::Accent.r, 255);
    EXPECT_EQ(Colors::Accent.g, 140);
    EXPECT_EQ(Colors::Accent.b, 60);
    EXPECT_EQ(Colors::Accent.a, 255);
}

TEST_F(PluginGUITest, MouseInteraction) {
    ParameterInfo param = {0, "Test", 0.0f, 100.0f, 50.0f, 50.0f, "%", nullptr};
    Knob knob(Rect(10, 10, 80, 80), &param);
    
    // Test mouse down in bounds
    EXPECT_TRUE(knob.handleMouseDown(50, 50));
    
    // Test mouse down out of bounds
    EXPECT_FALSE(knob.handleMouseDown(150, 150));
    
    // Test mouse up
    EXPECT_TRUE(knob.handleMouseUp(50, 50));
    
    // Test contains point
    EXPECT_TRUE(knob.containsPoint(50, 50));
    EXPECT_FALSE(knob.containsPoint(150, 150));
}

TEST_F(PluginGUITest, Rendering) {
    // Create GUI and test basic rendering (should not crash)
    EXPECT_TRUE(gui->create("test", false));
    
    // These should not crash
    gui->render();
    gui->present();
    
    // Test mouse handling
    gui->handleMouseDown(100, 100);
    gui->handleMouseMove(110, 110);
    gui->handleMouseUp(110, 110);
    gui->handleMouseWheel(1.0f);
    
    EXPECT_TRUE(true); // Test passes if no crashes
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}