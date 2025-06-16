#include "my_plugin.h"
#include "spectrum_analyzer.h"
#include "spectrum_gui.h"

#if VSTGUI_ENABLED
#include "my_plugin_gui.h"
#include <clap/ext/gui.h>
#if defined(__linux__)
#include "my_plugin_linux_extensions.h"
#endif
#endif
#include <clap/plugin-features.h>
#include <clap/ext/params.h>

#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc

// --- Forward declarations of plugin functions ---
static bool my_plugin_init(const struct clap_plugin *plugin);
static void my_plugin_destroy(const struct clap_plugin *plugin);
static bool my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count);
static void my_plugin_deactivate(const struct clap_plugin *plugin);
static bool my_plugin_start_processing(const struct clap_plugin *plugin);
static void my_plugin_stop_processing(const struct clap_plugin *plugin);
static void my_plugin_reset(const struct clap_plugin *plugin);
static clap_process_status my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process);
static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id);
static void my_plugin_on_main_thread(const struct clap_plugin *plugin);

// --- Parameters extension functions ---
static uint32_t my_plugin_params_count(const clap_plugin_t *plugin);
static bool my_plugin_params_get_info(const clap_plugin_t *plugin, uint32_t param_index, clap_param_info_t *param_info);
static bool my_plugin_params_get_value(const clap_plugin_t *plugin, clap_id param_id, double *value);
static bool my_plugin_params_value_to_text(const clap_plugin_t *plugin, clap_id param_id, double value, char *display, uint32_t size);
static bool my_plugin_params_text_to_value(const clap_plugin_t *plugin, clap_id param_id, const char *display, double *value);
static void my_plugin_params_flush(const clap_plugin_t *plugin, const clap_input_events_t *in, const clap_output_events_t *out);

// --- Plugin Descriptor ---
// Features array for the plugin descriptor  
static const char *const plugin_features[] = {CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, nullptr};

static const clap_plugin_descriptor_t my_plugin_descriptor = {
    CLAP_VERSION,
    "com.example.spectrum-analyzer", // id
    "Real-time Spectrum Analyzer", // name

    "Audio Innovations",       // vendor
    "https://example.com",     // url
    "https://example.com/manual", // manual_url
    "https://example.com/support", // support_url
    "1.0.0",                   // version
    "Real-time spectrum analyzer with GUI support and parameter automation.", // description

    plugin_features, // features
};


// --- Plugin Implementation ---
static bool my_plugin_init(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Initializing plugin\n");
    

    // Initialize plugin state
    self->sample_rate = 44100.0;
    self->max_frames_count = 512;
    
    // Initialize parameters with default values
    self->params.spectrum_drawing_style = 0.0f; // STYLE_LINES
    self->params.cutoff = 1000.0;
    self->params.resonance = 1.0;
    self->params.drive = 1.0;
    self->params.output = 1.0;
    self->params.mix = 1.0;
    self->params.bypass = false;
    
    for (int i = 0; i < 3; ++i) {
        self->params.eq_gain[i] = 0.0;
        self->params.eq_freq[i] = (i == 0) ? 100.0 : (i == 1) ? 1000.0 : 10000.0;
        self->params.eq_q[i] = 1.0;
    }

    // Initialize spectrum analyzer
    self->spectrum_analyzer = std::make_unique<SpectrumAnalyzer>();
    
    // Initialize GUI
    self->gui = std::make_unique<SpectrumGUI>(self);

    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    printf("MyPlugin: Destroying plugin\n");
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self) {
#if VSTGUI_ENABLED
        if (self->gui_editor) {
            delete self->gui_editor;
            self->gui_editor = nullptr;
        }
#endif
        free(self);
    }
}

static bool my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Activating plugin (Sample Rate: %.2f, Min Frames: %u, Max Frames: %u)\n", sample_rate, min_frames_count, max_frames_count);
    
    // Update sample rate and frame count
    self->sample_rate = sample_rate;
    self->max_frames_count = max_frames_count;
    
    // Initialize spectrum analyzer with sample rate
    if (self->spectrum_analyzer) {
        self->spectrum_analyzer->initialize(sample_rate);
    }
    
    // Allocate and prepare resources needed for processing (e.g., buffers)
    return true;
}

static void my_plugin_deactivate(const struct clap_plugin *plugin) {
    printf("MyPlugin: Deactivating plugin\n");
    // Free resources allocated in activate
}

static bool my_plugin_start_processing(const struct clap_plugin *plugin) {
    printf("MyPlugin: Starting processing\n");
    return true;
}

static void my_plugin_stop_processing(const struct clap_plugin *plugin) {
    printf("MyPlugin: Stopping processing\n");
}

static void my_plugin_reset(const struct clap_plugin *plugin) {
    printf("MyPlugin: Resetting plugin\n");
    // Reset plugin state (e.g., clear buffers, reset parameters)
}

static clap_process_status my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Process input events for parameter changes
    const uint32_t num_events = process->in_events->size(process->in_events);
    for (uint32_t i = 0; i < num_events; ++i) {
        const clap_event_header_t* hdr = process->in_events->get(process->in_events, i);
        if (hdr->space_id == CLAP_CORE_EVENT_SPACE_ID) {
            switch (hdr->type) {
                case CLAP_EVENT_PARAM_VALUE: {
                    const clap_event_param_value_t* pev = (const clap_event_param_value_t*)hdr;
                    if (pev->param_id == PARAM_SPECTRUM_DRAWING_STYLE) {
                        self->params.spectrum_drawing_style = (float)pev->value;
                    }
                    // Handle other parameter changes...
                    break;
                }
            }
        }
    }

    // Process audio from input to output (stereo) and analyze spectrum
    if (process->audio_outputs_count > 0 && process->audio_inputs_count > 0) {
        clap_audio_buffer_t *out_buf = &process->audio_outputs[0];
        const clap_audio_buffer_t *in_buf = &process->audio_inputs[0];

        if (out_buf->channel_count >= 2 && in_buf->channel_count >= 2 && out_buf->data32 && in_buf->data32) {
            // Copy input to output (passthrough)
            for (uint32_t i = 0; i < process->frames_count; ++i) {
                out_buf->data32[0][i] = in_buf->data32[0][i]; // Left channel
                out_buf->data32[1][i] = in_buf->data32[1][i]; // Right channel
            }
            
            // Analyze spectrum if analyzer is available
            if (self->spectrum_analyzer) {
                // Create mixed mono signal for analysis
                std::vector<float> mono_samples(process->frames_count);
                for (uint32_t i = 0; i < process->frames_count; ++i) {
                    mono_samples[i] = (in_buf->data32[0][i] + in_buf->data32[1][i]) * 0.5f;
                }
                
                // Process the audio through the spectrum analyzer
                self->spectrum_analyzer->process_samples(mono_samples.data(), process->frames_count);
            }
        }
    }
    
    return CLAP_PROCESS_CONTINUE;
}

// --- GUI Extension Implementation ---
#if VSTGUI_ENABLED
static bool my_plugin_gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->isApiSupported(api, is_floating);
    }
    return false;
}

static bool my_plugin_gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->getPreferredApi(api, is_floating);
    }
    return false;
}

static bool my_plugin_gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->create(api, is_floating);
    }
    return false;
}

static void my_plugin_gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        self->gui_editor->destroy();
    }
}

static bool my_plugin_gui_set_scale(const clap_plugin_t *plugin, double scale) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->setScale(scale);
    }
    return false;
}

static bool my_plugin_gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->getSize(width, height);
    }
    return false;
}

static bool my_plugin_gui_can_resize(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->canResize();
    }
    return false;
}

static bool my_plugin_gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->getResizeHints(hints);
    }
    return false;
}

static bool my_plugin_gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->adjustSize(width, height);
    }
    return false;
}

static bool my_plugin_gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->setSize(width, height);
    }
    return false;
}

static bool my_plugin_gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->setParent(window);
    }
    return false;
}

static bool my_plugin_gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->setTransient(window);
    }
    return false;
}

static void my_plugin_gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        self->gui_editor->suggestTitle(title);
    }
}

static bool my_plugin_gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->show();
    }
    return false;
}

static bool my_plugin_gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->hide();
    }
    return false;
}

static const clap_plugin_gui_t my_gui_extension = {
    my_plugin_gui_is_api_supported,
    my_plugin_gui_get_preferred_api,
    my_plugin_gui_create,
    my_plugin_gui_destroy,
    my_plugin_gui_set_scale,
    my_plugin_gui_get_size,
    my_plugin_gui_can_resize,
    my_plugin_gui_get_resize_hints,
    my_plugin_gui_adjust_size,
    my_plugin_gui_set_size,
    my_plugin_gui_set_parent,
    my_plugin_gui_set_transient,
    my_plugin_gui_suggest_title,
    my_plugin_gui_show,
    my_plugin_gui_hide
};
#endif // VSTGUI_ENABLED

// --- Parameters Extension Implementation ---
static uint32_t my_plugin_params_count(const clap_plugin_t *plugin) {
    return PARAM_COUNT;
}

static bool my_plugin_params_get_info(const clap_plugin_t *plugin, uint32_t param_index, clap_param_info_t *param_info) {
    switch (param_index) {
        case PARAM_SPECTRUM_DRAWING_STYLE:
            param_info->id = PARAM_SPECTRUM_DRAWING_STYLE;
            strncpy(param_info->name, "Drawing Style", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "Spectrum", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = 0.0;
            param_info->max_value = STYLE_COUNT - 1;
            param_info->default_value = 0.0;
            param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_ENUM;
            param_info->cookie = nullptr;
            return true;
        case PARAM_CUTOFF:
            param_info->id = PARAM_CUTOFF;
            strncpy(param_info->name, "Cutoff", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "Filter", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = 20.0;
            param_info->max_value = 20000.0;
            param_info->default_value = 1000.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        case PARAM_RESONANCE:
            param_info->id = PARAM_RESONANCE;
            strncpy(param_info->name, "Resonance", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "Filter", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = 0.1;
            param_info->max_value = 10.0;
            param_info->default_value = 1.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        case PARAM_DRIVE:
            param_info->id = PARAM_DRIVE;
            strncpy(param_info->name, "Drive", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "Distortion", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = 0.0;
            param_info->max_value = 10.0;
            param_info->default_value = 1.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        case PARAM_OUTPUT:
            param_info->id = PARAM_OUTPUT;
            strncpy(param_info->name, "Output", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "Main", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = 0.0;
            param_info->max_value = 2.0;
            param_info->default_value = 1.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        case PARAM_MIX:
            param_info->id = PARAM_MIX;
            strncpy(param_info->name, "Mix", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "Main", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = 0.0;
            param_info->max_value = 1.0;
            param_info->default_value = 1.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        case PARAM_BYPASS:
            param_info->id = PARAM_BYPASS;
            strncpy(param_info->name, "Bypass", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "Main", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = 0.0;
            param_info->max_value = 1.0;
            param_info->default_value = 0.0;
            param_info->flags = CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_BYPASS;
            param_info->cookie = nullptr;
            return true;
        case PARAM_EQ_GAIN1:
            param_info->id = PARAM_EQ_GAIN1;
            strncpy(param_info->name, "EQ Gain 1", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "EQ", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = -24.0;
            param_info->max_value = 24.0;
            param_info->default_value = 0.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        case PARAM_EQ_FREQ1:
            param_info->id = PARAM_EQ_FREQ1;
            strncpy(param_info->name, "EQ Freq 1", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "EQ", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = 20.0;
            param_info->max_value = 20000.0;
            param_info->default_value = 100.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        case PARAM_EQ_Q1:
            param_info->id = PARAM_EQ_Q1;
            strncpy(param_info->name, "EQ Q 1", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "EQ", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = 0.1;
            param_info->max_value = 10.0;
            param_info->default_value = 1.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        case PARAM_EQ_GAIN2:
            param_info->id = PARAM_EQ_GAIN2;
            strncpy(param_info->name, "EQ Gain 2", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "EQ", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = -24.0;
            param_info->max_value = 24.0;
            param_info->default_value = 0.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        case PARAM_EQ_FREQ2:
            param_info->id = PARAM_EQ_FREQ2;
            strncpy(param_info->name, "EQ Freq 2", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "EQ", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = 20.0;
            param_info->max_value = 20000.0;
            param_info->default_value = 1000.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        case PARAM_EQ_Q2:
            param_info->id = PARAM_EQ_Q2;
            strncpy(param_info->name, "EQ Q 2", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "EQ", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = 0.1;
            param_info->max_value = 10.0;
            param_info->default_value = 1.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        case PARAM_EQ_GAIN3:
            param_info->id = PARAM_EQ_GAIN3;
            strncpy(param_info->name, "EQ Gain 3", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "EQ", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = -24.0;
            param_info->max_value = 24.0;
            param_info->default_value = 0.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        case PARAM_EQ_FREQ3:
            param_info->id = PARAM_EQ_FREQ3;
            strncpy(param_info->name, "EQ Freq 3", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "EQ", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = 20.0;
            param_info->max_value = 20000.0;
            param_info->default_value = 10000.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        case PARAM_EQ_Q3:
            param_info->id = PARAM_EQ_Q3;
            strncpy(param_info->name, "EQ Q 3", sizeof(param_info->name));
            param_info->name[sizeof(param_info->name) - 1] = '\0';
            strncpy(param_info->module, "EQ", sizeof(param_info->module));
            param_info->module[sizeof(param_info->module) - 1] = '\0';
            param_info->min_value = 0.1;
            param_info->max_value = 10.0;
            param_info->default_value = 1.0;
            param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
            param_info->cookie = nullptr;
            return true;
        default:
            return false;
    }
}

static bool my_plugin_params_get_value(const clap_plugin_t *plugin, clap_id param_id, double *value) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    switch (param_id) {
        case PARAM_SPECTRUM_DRAWING_STYLE:
            *value = self->params.spectrum_drawing_style;
            return true;
        case PARAM_CUTOFF:
            *value = self->params.cutoff;
            return true;
        case PARAM_RESONANCE:
            *value = self->params.resonance;
            return true;
        case PARAM_DRIVE:
            *value = self->params.drive;
            return true;
        case PARAM_OUTPUT:
            *value = self->params.output;
            return true;
        case PARAM_MIX:
            *value = self->params.mix;
            return true;
        case PARAM_BYPASS:
            *value = self->params.bypass ? 1.0 : 0.0;
            return true;
        case PARAM_EQ_GAIN1:
            *value = self->params.eq_gain[0];
            return true;
        case PARAM_EQ_FREQ1:
            *value = self->params.eq_freq[0];
            return true;
        case PARAM_EQ_Q1:
            *value = self->params.eq_q[0];
            return true;
        case PARAM_EQ_GAIN2:
            *value = self->params.eq_gain[1];
            return true;
        case PARAM_EQ_FREQ2:
            *value = self->params.eq_freq[1];
            return true;
        case PARAM_EQ_Q2:
            *value = self->params.eq_q[1];
            return true;
        case PARAM_EQ_GAIN3:
            *value = self->params.eq_gain[2];
            return true;
        case PARAM_EQ_FREQ3:
            *value = self->params.eq_freq[2];
            return true;
        case PARAM_EQ_Q3:
            *value = self->params.eq_q[2];
            return true;
        default:
            return false;
    }
}

static bool my_plugin_params_value_to_text(const clap_plugin_t *plugin, clap_id param_id, double value, char *display, uint32_t size) {
    // For simplicity and CLAP validator compatibility, we don't provide custom text conversion
    // The host will use default numeric formatting for all parameters
    return false;
}

static bool my_plugin_params_text_to_value(const clap_plugin_t *plugin, clap_id param_id, const char *display, double *value) {
    // For simplicity and CLAP validator compatibility, we don't provide custom text conversion
    // The host will handle text to value conversion using default numeric parsing
    return false;
}

static void my_plugin_params_flush(const clap_plugin_t *plugin, const clap_input_events_t *in, const clap_output_events_t *out) {
    // Handle parameter automation events
    const uint32_t num_events = in->size(in);
    for (uint32_t i = 0; i < num_events; ++i) {
        const clap_event_header_t* hdr = in->get(in, i);
        if (hdr->space_id == CLAP_CORE_EVENT_SPACE_ID && hdr->type == CLAP_EVENT_PARAM_VALUE) {
            const clap_event_param_value_t* pev = (const clap_event_param_value_t*)hdr;
            // Handle parameter value change
        }
    }
}

static const clap_plugin_params_t my_plugin_params_extension = {
    my_plugin_params_count,
    my_plugin_params_get_info,
    my_plugin_params_get_value,
    my_plugin_params_value_to_text,
    my_plugin_params_text_to_value,
    my_plugin_params_flush,
};


static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    printf("MyPlugin: Host requesting extension: %s\n", id);
    

    if (strcmp(id, CLAP_EXT_PARAMS) == 0) {
        return &my_plugin_params_extension;
    }
    

#if VSTGUI_ENABLED
    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        return &my_gui_extension;
    }
#if defined(__linux__)
    if (strcmp(id, CLAP_EXT_TIMER_SUPPORT) == 0) {
        return &my_timer_support_extension;
    }
    if (strcmp(id, CLAP_EXT_POSIX_FD_SUPPORT) == 0) {
        return &my_posix_fd_support_extension;
    }
#endif
#endif
    
    return NULL; // No other extensions supported in this basic example
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    // Called by the host to perform tasks that must run on the main thread.
    // printf("MyPlugin: on_main_thread called\n");
}

// --- Plugin Entry Point (clap_plugin_entry) ---
// This is not directly part of the clap_plugin_t struct but is essential.
// It's usually defined in the factory.

// --- Plugin Factory ---
// This structure is responsible for creating plugin instances.

static uint32_t my_factory_get_plugin_count(const struct clap_plugin_factory *factory) {
    return 1; // We have one plugin in this factory
}

static const clap_plugin_descriptor_t *my_factory_get_plugin_descriptor(const struct clap_plugin_factory *factory, uint32_t index) {
    if (index == 0) {
        return &my_plugin_descriptor;
    }
    return NULL;
}

static const clap_plugin_t *my_factory_create_plugin(const struct clap_plugin_factory *factory, const clap_host_t *host, const char *plugin_id) {
    if (strcmp(plugin_id, my_plugin_descriptor.id) != 0) {
        fprintf(stderr, "MyPlugin: Error - incorrect plugin ID requested: %s\n", plugin_id);
        return NULL;
    }

    my_plugin_t *self = (my_plugin_t *)calloc(1, sizeof(my_plugin_t));
    if (!self) {
        fprintf(stderr, "MyPlugin: Error - failed to allocate memory for plugin instance\n");
        return NULL;
    }

    // Store host pointer for extension access
    self->host = host;

    // Initialize GUI editor
#if VSTGUI_ENABLED
    self->gui_editor = new MyPluginEditor(host);
    if (!self->gui_editor) {
        fprintf(stderr, "MyPlugin: Error - failed to create GUI editor\n");
        free(self);
        return NULL;
    }
#endif

    self->plugin.desc = &my_plugin_descriptor;
    self->plugin.plugin_data = self; // Point to ourself for context
    self->plugin.init = my_plugin_init;
    self->plugin.destroy = my_plugin_destroy;
    self->plugin.activate = my_plugin_activate;
    self->plugin.deactivate = my_plugin_deactivate;
    self->plugin.start_processing = my_plugin_start_processing;
    self->plugin.stop_processing = my_plugin_stop_processing;
    self->plugin.reset = my_plugin_reset;
    self->plugin.process = my_plugin_process;
    self->plugin.get_extension = my_plugin_get_extension;
    self->plugin.on_main_thread = my_plugin_on_main_thread;

    printf("MyPlugin: Plugin instance created successfully.\n");
    return &self->plugin;
}

const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory = {
    my_factory_get_plugin_count,
    my_factory_get_plugin_descriptor,
    my_factory_create_plugin,
};

// --- CLAP Entry Point ---
// This is the main entry point that the host will look for.
CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
    CLAP_VERSION,
    // init: Called once when the library is loaded.
    [](const char *plugin_path) -> bool {
        printf("MyPlugin: clap_entry.init called (path: %s)\n", plugin_path);
        // Perform any global library initialization here if needed
        return true;
    },
    // deinit: Called once when the library is unloaded.
    []() -> void {
        printf("MyPlugin: clap_entry.deinit called\n");
        // Perform any global library cleanup here if needed
    },
    // get_factory: Returns a factory based on its ID.
    [](const char *factory_id) -> const void * {
        printf("MyPlugin: clap_entry.get_factory called (ID: %s)\n", factory_id);
        if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0) {
            return &my_plugin_factory;
        }
        // To support other factory types, check their specific IDs here.
        // For example, CLAP_PLUGIN_VOICE_INFO_FACTORY_ID for voice info.
        // Or CLAP_PLUGIN_REMOTABLE_CONTROLS_FACTORY_ID for remotable controls.
        fprintf(stderr, "MyPlugin: Unknown factory ID requested: %s\n", factory_id);
        return NULL;
    }
};
