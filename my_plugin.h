// my_plugin.h
#pragma once

#include "clap/helpers/plugin.hh"
#include "clap/extensions/gui_skia.h" // For clap_plugin_gui_skia and clap_plugin_gui_skia_paint_t

// Forward declare SkCanvas for onPaint
class SkCanvas;

// ImGui includes
#include "imgui.h"
#include "backends/imgui_skia/imgui_impl_skia.h"


// Define a unique ID for your plugin's Skia GUI
// This can be any string, but it's good practice to make it descriptive.
// For example, you could use your plugin's ID and append "/skia".
// For this example, we'll use a generic ID.
// Make sure this matches the ID you return in your get_extension for CLAP_EXT_GUI
#define MY_PLUGIN_SKIA_UI_ID "com.example.myplugin/skia"


class MyClapPlugin : public clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate,
                                                 clap::helpers::MainThreadCheck::Fatal> {
public:
    MyClapPlugin(const clap_plugin_descriptor_t* desc, const clap_host_t* host)
        : clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate,
                                 clap::helpers::MainThreadCheck::Fatal>(desc, host),
          _host(host) // Store host for later use if needed, e.g. for resizing GUI
          {
            // _guiSkia.plugin = nullptr; // Will be set by factory
            // _guiSkia.api_name = CLAP_GUI_API_SKIA;
            // _guiSkia.is_api_supported = my_gui_skia_is_api_supported;
            // _guiSkia.get_preferred_api = my_gui_skia_get_preferred_api;
            // _guiSkia.create = my_gui_skia_create;
            // _guiSkia.destroy = my_gui_skia_destroy;
            // _guiSkia.set_scale = my_gui_skia_set_scale;
            // _guiSkia.get_size = my_gui_skia_get_size;
            // _guiSkia.can_resize = my_gui_skia_can_resize;
            // _guiSkia.get_resize_hints = my_gui_skia_get_resize_hints;
            // _guiSkia.adjust_size = my_gui_skia_adjust_size;
            // _guiSkia.set_size = my_gui_skia_set_size;
            // _guiSkia.set_parent = my_gui_skia_set_parent;
            // _guiSkia.set_transient = my_gui_skia_set_transient;
            // _guiSkia.suggest_title = my_gui_skia_suggest_title;
            // _guiSkia.show = my_gui_skia_show;
            // _guiSkia.hide = my_gui_skia_hide;
            // // Skia specific part of GUI extension
            // _guiSkia.render = my_gui_skia_render; // This is where onPaint will be called from
          }

    // Override virtual methods from clap::helpers::Plugin
    bool init() noexcept override {
        // Example: Check for host extensions or perform other setup
        // const clap_host_log_t* logger = (const clap_host_log_t*)_host->get_extension(_host, CLAP_EXT_LOG);
        // if (logger) { logger->log(logger, CLAP_LOG_INFO, "[MyClapPlugin] Initializing..."); }


        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking (if using docking branch)
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport / Platform Windows (if using docking branch)
        ImGui::StyleColorsDark(); // or ImGui::StyleColorsClassic();
        // When viewports are enabled, tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        // ImGuiStyle& style = ImGui::GetStyle();
        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        //     style.WindowRounding = 0.0f;
        //     style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        // }


        // Initialize our Skia backend for ImGui
        if (!ImGui_ImplSkia_Init()) {
            // if (logger) { logger->log(logger, CLAP_LOG_ERROR, "[MyClapPlugin] Failed to initialize ImGui Skia backend."); }
            return false;
        }
        // if (logger) { logger->log(logger, CLAP_LOG_INFO, "[MyClapPlugin] Initialized successfully."); }
        return true;
    }

    void destroy() noexcept override {
        // const clap_host_log_t* logger = (const clap_host_log_t*)_host->get_extension(_host, CLAP_EXT_LOG);
        // if (logger) { logger->log(logger, CLAP_LOG_INFO, "[MyClapPlugin] Destroying..."); }

        ImGui_ImplSkia_Shutdown();
        ImGui::DestroyContext();
        // Perform any other cleanup needed by your plugin
    }

    // This is where the main audio processing happens.
    // For this example, we'll just pass audio through.
    clap_process_status process(const clap_process_t* process) noexcept override {
        // Example: Process audio from input to output (stereo)
        if (process->audio_outputs_count > 0 && process->audio_inputs_count > 0 &&
            process->audio_outputs[0].data32 && process->audio_inputs[0].data32 &&
            process->audio_inputs[0].channel_count >= 2 && process->audio_outputs[0].channel_count >=2) {

            uint32_t frames = process->frames_count;
            float** in = process->audio_inputs[0].data32;
            float** out = process->audio_outputs[0].data32;

            for (uint32_t i = 0; i < frames; ++i) {
                out[0][i] = in[0][i]; // Left channel
                out[1][i] = in[1][i]; // Right channel
            }
        }
        return CLAP_PROCESS_CONTINUE;
    }


    // This method will be called by the CLAP GUI Skia extension's render function.
    void onPaint(SkCanvas* canvas, int width, int height) {
        if (!canvas) return;

        // Start the ImGui frame
        ImGui_ImplSkia_NewFrame(); // Prepares backend
        ImGui::NewFrame();        // Prepares ImGui

        // 1. Show a simple ImGui window.
        bool show_demo_window = true;
        ImGui::ShowDemoWindow(&show_demo_window);

        // 2. A custom window
        ImGui::Begin("Hello, Skia from ImGui!");
        ImGui::Text("Plugin Name: %s", _descriptor->name);
        ImGui::Text("Plugin Version: %s", _descriptor->version);
        static float f = 0.0f;
        static int counter = 0;
        ImGui::Text("This is some useful text.");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        if (ImGui::Button("Button")) counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // Clear the canvas (optional, host might do this, or Skia backend might do it)
        // For Skia, if the surface is not opaque, clearing might be important.
        // canvas->clear(SK_ColorWHITE); // Example clear

        // Render ImGui draw data using the Skia backend
        ImGui::Render(); // Generates ImDrawData
        ImGui_ImplSkia_RenderDrawData(ImGui::GetDrawData(), canvas);

        // Other Skia drawing can happen here, before or after ImGui
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2.0f);
        // Draw a circle that pulses with the slider value
        float radius = 20.0f + f * 80.0f;
        canvas->drawCircle(width * 0.75f, height * 0.25f, radius, paint);
    }

    // CLAP extension support
    const void* getExtension(const char* id) noexcept override;

private:
    const clap_host_t* _host;
    // clap_plugin_gui_skia_t _guiSkia; // Instance of the Skia GUI extension struct

    // Placeholder for GUI size
    uint32_t _guiWidth = 800;
    uint32_t _guiHeight = 600;

    // GUI related static functions that will be pointed to by _guiSkia
    // friend static bool my_gui_skia_is_api_supported(const clap_plugin_t* plugin, const char* api, bool is_floating);
    // friend static bool my_gui_skia_get_preferred_api(const clap_plugin_t* plugin, const char** api, bool* is_floating);
    // friend static bool my_gui_skia_create(const clap_plugin_t* plugin, const char* api, bool is_floating);
    // friend static void my_gui_skia_destroy(const clap_plugin_t* plugin);
    // friend static bool my_gui_skia_set_scale(const clap_plugin_t* plugin, double scale);
    // friend static bool my_gui_skia_get_size(const clap_plugin_t* plugin, uint32_t* width, uint32_t* height);
    // friend static bool my_gui_skia_can_resize(const clap_plugin_t* plugin);
    // friend static bool my_gui_skia_get_resize_hints(const clap_plugin_t* plugin, clap_gui_resize_hints_t* hints);
    // friend static bool my_gui_skia_adjust_size(const clap_plugin_t* plugin, uint32_t* width, uint32_t* height);
    // friend static bool my_gui_skia_set_size(const clap_plugin_t* plugin, uint32_t width, uint32_t height);
    // friend static bool my_gui_skia_set_parent(const clap_plugin_t* plugin, const clap_window_t* window);
    // friend static bool my_gui_skia_set_transient(const clap_plugin_t* plugin, const clap_window_t* window);
    // friend static void my_gui_skia_suggest_title(const clap_plugin_t* plugin, const char* title);
    // friend static bool my_gui_skia_show(const clap_plugin_t* plugin);
    // friend static bool my_gui_skia_hide(const clap_plugin_t* plugin);
    // friend static bool my_gui_skia_render(const clap_plugin_t* plugin, const clap_plugin_gui_skia_paint_t* paint_event);

};

// The factory to create the plugin
extern const clap_plugin_factory_t MyClapPluginFactory;
