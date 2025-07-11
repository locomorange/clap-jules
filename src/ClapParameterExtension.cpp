#include "ClapParameterExtension.h"
#include "MVVM.h"
#include "../my_plugin.h"
#include <string.h>
#include <stdio.h>

namespace ClapeJules::ClapExt {

ParameterExtension::ParameterExtension(std::shared_ptr<Parameter> frequencyParam)
    : m_frequencyParam(frequencyParam) {
}

uint32_t ParameterExtension::count(const clap_plugin *plugin) {
    return 1; // Only frequency parameter for now
}

bool ParameterExtension::get_info(const clap_plugin *plugin, uint32_t param_index, clap_param_info *param_info) {
    if (param_index != 0 || !param_info) {
        return false;
    }
    
    param_info->id = FREQ_PARAM_ID;
    param_info->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_MODULATABLE;
    param_info->cookie = nullptr;
    strncpy(param_info->name, "Frequency", CLAP_NAME_SIZE - 1);
    param_info->name[CLAP_NAME_SIZE - 1] = '\0';
    strncpy(param_info->module, "Filter", CLAP_NAME_SIZE - 1);
    param_info->module[CLAP_NAME_SIZE - 1] = '\0';
    param_info->min_value = 20.0;
    param_info->max_value = 20000.0;
    param_info->default_value = 1000.0;
    
    return true;
}

bool ParameterExtension::get_value(const clap_plugin *plugin, clap_id param_id, double *value) {
    if (param_id != FREQ_PARAM_ID || !value) {
        return false;
    }
    
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->view) {
        auto param = self->view->getFrequencyParameter();
        if (param) {
            *value = static_cast<double>(param->getValue());
            return true;
        }
    }
    
    return false;
}

bool ParameterExtension::value_to_text(const clap_plugin *plugin, clap_id param_id, double value, char *display, uint32_t size) {
    if (param_id != FREQ_PARAM_ID || !display || size == 0) {
        return false;
    }
    
    snprintf(display, size, "%.1f Hz", value);
    return true;
}

bool ParameterExtension::text_to_value(const clap_plugin *plugin, clap_id param_id, const char *display, double *value) {
    if (param_id != FREQ_PARAM_ID || !display || !value) {
        return false;
    }
    
    double parsed_value;
    if (sscanf(display, "%lf", &parsed_value) == 1) {
        *value = parsed_value;
        return true;
    }
    
    return false;
}

void ParameterExtension::flush(const clap_plugin *plugin, const clap_input_events *in, const clap_output_events *out) {
    if (!in) return;
    
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (!self || !self->view) return;
    
    auto freqParam = self->view->getFrequencyParameter();
    if (!freqParam) return;
    
    const uint32_t num_events = in->size(in);
    for (uint32_t i = 0; i < num_events; ++i) {
        const clap_event_header_t *hdr = in->get(in, i);
        if (hdr->space_id != CLAP_CORE_EVENT_SPACE_ID) continue;
        
        if (hdr->type == CLAP_EVENT_PARAM_VALUE) {
            const clap_event_param_value *param_event = (const clap_event_param_value *)hdr;
            if (param_event->param_id == FREQ_PARAM_ID) {
                freqParam->setValue(static_cast<float>(param_event->value));
            }
        }
    }
}

const clap_plugin_params* ParameterExtension::getInterface() {
    return &s_params_extension;
}

const clap_plugin_params s_params_extension = {
    .count = ParameterExtension::count,
    .get_info = ParameterExtension::get_info,
    .get_value = ParameterExtension::get_value,
    .value_to_text = ParameterExtension::value_to_text,
    .text_to_value = ParameterExtension::text_to_value,
    .flush = ParameterExtension::flush,
};

} // namespace ClapeJules::ClapExt