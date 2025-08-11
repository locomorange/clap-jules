#include <gtest/gtest.h>
#include "src/minimal_brisk_gui.h"

class MinimalBriskGUITest : public ::testing::Test {
protected:
    void SetUp() override {
        gui = std::make_unique<clap_gui::MinimalBriskGUI>();
    }

    void TearDown() override {
        gui.reset();
    }

    std::unique_ptr<clap_gui::MinimalBriskGUI> gui;
};

TEST_F(MinimalBriskGUITest, ConstructorDestructor) {
    // コンストラクタとデストラクタのテスト
    EXPECT_NE(gui, nullptr);
}

TEST_F(MinimalBriskGUITest, CreateDestroy) {
    // create/destroyメソッドのテスト
    clap_window_t window{};
    window.api = CLAP_WINDOW_API_WIN32;  // プラットフォームに応じて変更
    
    bool created = gui->create(&window, false);
    EXPECT_TRUE(created);
    
    gui->destroy();
    // destroyは例外を投げないことを確認
}

TEST_F(MinimalBriskGUITest, SizeOperations) {
    // サイズ設定とサイズ取得のテスト
    bool size_set = gui->set_size(800, 600);
    EXPECT_TRUE(size_set);
    
    uint32_t width, height;
    bool size_got = gui->get_size(&width, &height);
    EXPECT_TRUE(size_got);
    EXPECT_EQ(width, 800);
    EXPECT_EQ(height, 600);
}

TEST_F(MinimalBriskGUITest, ShowHide) {
    // show/hideメソッドのテスト（例外が投げられないことを確認）
    EXPECT_NO_THROW(gui->show());
    EXPECT_NO_THROW(gui->hide());
}

TEST_F(MinimalBriskGUITest, ParameterUpdate) {
    // パラメータ更新のテスト（例外が投げられないことを確認）
    EXPECT_NO_THROW(gui->set_parameter_value(1, 0.5));
    EXPECT_NO_THROW(gui->set_parameter_value(2, 1.0));
}
