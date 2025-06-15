#include <gtest/gtest.h>
#include <cmath>
#include "../my_plugin.h"
#include "../spectrum_analyzer.h"

// Define M_PI if not already defined (Windows compatibility)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Test fixture for plugin tests
class MyPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a minimal host interface (just for testing)
        static clap_host_t test_host = {
            CLAP_VERSION,
            nullptr, // host_data
            "Test Host", // name
            "Test", // vendor
            "http://test.com", // url
            "1.0.0", // version
            nullptr, // get_extension
            nullptr, // request_restart
            nullptr, // request_process
            nullptr, // request_callback
        };
        
        // Create plugin instance
        plugin_instance = my_plugin_factory.create_plugin(&my_plugin_factory, &test_host, "com.example.spectrum-analyzer");
        ASSERT_NE(plugin_instance, nullptr);
        
        // Initialize plugin
        ASSERT_TRUE(plugin_instance->init(plugin_instance));
    }
    
    void TearDown() override {
        if (plugin_instance) {
            plugin_instance->destroy(plugin_instance);
        }
    }
    
    const clap_plugin_t* plugin_instance = nullptr;
};

TEST_F(MyPluginTest, PluginInitializesCorrectly) {
    ASSERT_NE(plugin_instance, nullptr);
    
    // Check that the plugin has been initialized
    my_plugin_t* self = static_cast<my_plugin_t*>(plugin_instance->plugin_data);
    ASSERT_NE(self, nullptr);
    ASSERT_NE(self->spectrum_analyzer, nullptr);
    ASSERT_NE(self->gui, nullptr);
}

TEST_F(MyPluginTest, ParametersExtensionWorks) {
    // Get parameters extension
    const clap_plugin_params_t* params_ext = static_cast<const clap_plugin_params_t*>(
        plugin_instance->get_extension(plugin_instance, CLAP_EXT_PARAMS));
    
    ASSERT_NE(params_ext, nullptr);
    
    // Check parameter count
    EXPECT_EQ(params_ext->count(plugin_instance), PARAM_COUNT);
    
    // Check parameter info
    clap_param_info_t param_info;
    ASSERT_TRUE(params_ext->get_info(plugin_instance, 0, &param_info));
    EXPECT_EQ(param_info.id, PARAM_SPECTRUM_DRAWING_STYLE);
    EXPECT_STREQ(param_info.name, "Drawing Style");
}

TEST_F(MyPluginTest, GUIExtensionWorks) {
    // Get GUI extension
    const clap_plugin_gui_t* gui_ext = static_cast<const clap_plugin_gui_t*>(
        plugin_instance->get_extension(plugin_instance, CLAP_EXT_GUI));
    
#if VSTGUI_ENABLED
    ASSERT_NE(gui_ext, nullptr);
    
    // Test API support
    EXPECT_TRUE(gui_ext->is_api_supported(plugin_instance, nullptr, true)); // floating window
    
    // Test size queries
    uint32_t width, height;
    ASSERT_TRUE(gui_ext->create(plugin_instance, nullptr, true));
    ASSERT_TRUE(gui_ext->get_size(plugin_instance, &width, &height));
    EXPECT_GT(width, 0u);
    EXPECT_GT(height, 0u);
    
    gui_ext->destroy(plugin_instance);
#else
    // GUI extension should not be available when VSTGUI is disabled
    EXPECT_EQ(gui_ext, nullptr);
#endif
}

TEST_F(MyPluginTest, SpectrumAnalyzerProcessesAudio) {
    my_plugin_t* self = static_cast<my_plugin_t*>(plugin_instance->plugin_data);
    
    // Activate plugin
    ASSERT_TRUE(plugin_instance->activate(plugin_instance, 44100.0, 64, 512));
    
    // Test spectrum analyzer with some dummy audio
    std::vector<float> test_samples(256);
    for (size_t i = 0; i < test_samples.size(); ++i) {
        test_samples[i] = std::sin(2.0f * M_PI * 440.0f * i / 44100.0f); // 440Hz tone
    }
    
    self->spectrum_analyzer->process_samples(test_samples.data(), test_samples.size());
    
    // Check that spectrum data is generated
    const auto& spectrum_data = self->spectrum_analyzer->get_spectrum_data();
    const auto& freq_bins = self->spectrum_analyzer->get_frequency_bins();
    
    EXPECT_GT(spectrum_data.size(), 0u);
    EXPECT_GT(freq_bins.size(), 0u);
    EXPECT_EQ(spectrum_data.size(), freq_bins.size());
    
    plugin_instance->deactivate(plugin_instance);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
