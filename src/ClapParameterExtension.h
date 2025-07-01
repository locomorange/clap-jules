#pragma once

#include <clap/clap.h>
#include <clap/ext/params.h>
#include <memory>

namespace ClapeJules {
    class Parameter;
}

namespace ClapeJules::ClapExt {

// CLAP parameter extension implementation
class ParameterExtension {
public:
    explicit ParameterExtension(std::shared_ptr<Parameter> frequencyParam);
    
    // CLAP parameter interface
    static const clap_plugin_params* getInterface();
    
    // Parameter callbacks
    static uint32_t count(const clap_plugin *plugin);
    static bool get_info(const clap_plugin *plugin, uint32_t param_index, clap_param_info *param_info);
    static bool get_value(const clap_plugin *plugin, clap_id param_id, double *value);
    static bool value_to_text(const clap_plugin *plugin, clap_id param_id, double value, char *display, uint32_t size);
    static bool text_to_value(const clap_plugin *plugin, clap_id param_id, const char *display, double *value);
    static void flush(const clap_plugin *plugin, const clap_input_events *in, const clap_output_events *out);

private:
    std::shared_ptr<Parameter> m_frequencyParam;
    static constexpr clap_id FREQ_PARAM_ID = 1;
};

extern const clap_plugin_params s_params_extension;

} // namespace ClapeJules::ClapExt