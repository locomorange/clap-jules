#include "my_plugin_parameters.h"
#include <cmath>
#include <cstring>

MyPluginParameterManager::MyPluginParameterManager() {
    initializeParameters();
}

void MyPluginParameterManager::initializeParameters() {
    // パラメータ情報の初期化 (Initialize parameter information)
    parameter_infos = {
        // メイン制御パラメータ (Main control parameters)
        {PARAM_DEPTH, "Depth", "Main", 0.0, 1.0, 0.5, CLAP_PARAM_IS_AUTOMATABLE},
        {PARAM_SHARPNESS, "Sharpness", "Main", 0.0, 1.0, 0.7, CLAP_PARAM_IS_AUTOMATABLE},
        {PARAM_SELECTIVITY, "Selectivity", "Main", 0.0, 1.0, 0.6, CLAP_PARAM_IS_AUTOMATABLE},
        {PARAM_ATTACK, "Attack", "Envelope", 0.001, 1.0, 0.01, CLAP_PARAM_IS_AUTOMATABLE},
        {PARAM_RELEASE, "Release", "Envelope", 0.001, 1.0, 0.1, CLAP_PARAM_IS_AUTOMATABLE},
        {PARAM_QUALITY, "Quality", "Main", 0.0, 1.0, 1.0, CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED},
        {PARAM_MODE, "Mode", "Main", 0.0, 2.0, 0.0, CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED},
        {PARAM_STEREO_LINK, "Stereo Link", "Main", 0.0, 1.0, 1.0, CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED},
        {PARAM_BALANCE, "Balance", "Main", -1.0, 1.0, 0.0, CLAP_PARAM_IS_AUTOMATABLE},
        {PARAM_OUTPUT_GAIN, "Output Gain", "Main", 0.0, 2.0, 1.0, CLAP_PARAM_IS_AUTOMATABLE},
        {PARAM_BYPASS, "Bypass", "Main", 0.0, 1.0, 0.0, CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED | CLAP_PARAM_IS_BYPASS},
        
        // EQカーブパラメータ (EQ curve parameters)
        {PARAM_EQ_POINT_1_FREQ, "EQ Point 1 Freq", "EQ", 20.0, 20000.0, 200.0, CLAP_PARAM_IS_AUTOMATABLE},
        {PARAM_EQ_POINT_1_GAIN, "EQ Point 1 Gain", "EQ", -12.0, 12.0, 0.0, CLAP_PARAM_IS_AUTOMATABLE},
        {PARAM_EQ_POINT_2_FREQ, "EQ Point 2 Freq", "EQ", 20.0, 20000.0, 1000.0, CLAP_PARAM_IS_AUTOMATABLE},
        {PARAM_EQ_POINT_2_GAIN, "EQ Point 2 Gain", "EQ", -12.0, 12.0, 0.0, CLAP_PARAM_IS_AUTOMATABLE},
        {PARAM_EQ_POINT_3_FREQ, "EQ Point 3 Freq", "EQ", 20.0, 20000.0, 5000.0, CLAP_PARAM_IS_AUTOMATABLE},
        {PARAM_EQ_POINT_3_GAIN, "EQ Point 3 Gain", "EQ", -12.0, 12.0, 0.0, CLAP_PARAM_IS_AUTOMATABLE}
    };
    
    // デフォルト値でパラメータ値を初期化 (Initialize parameter values with defaults)
    parameter_values.resize(PARAM_COUNT);
    for (size_t i = 0; i < parameter_infos.size(); ++i) {
        parameter_values[i] = parameter_infos[i].default_value;
    }
}

uint32_t MyPluginParameterManager::getParameterCount() const {
    return static_cast<uint32_t>(parameter_infos.size());
}

bool MyPluginParameterManager::getParameterInfo(uint32_t param_index, clap_param_info_t* param_info) const {
    if (param_index >= parameter_infos.size()) {
        return false;
    }
    
    const auto& info = parameter_infos[param_index];
    
    param_info->id = info.id;
    param_info->flags = info.flags;
    param_info->min_value = info.min_value;
    param_info->max_value = info.max_value;
    param_info->default_value = info.default_value;
    
    // 文字列をコピー (Copy strings)
    strncpy(param_info->name, info.name.c_str(), CLAP_NAME_SIZE - 1);
    param_info->name[CLAP_NAME_SIZE - 1] = '\0';
    
    strncpy(param_info->module, info.module.c_str(), CLAP_NAME_SIZE - 1);
    param_info->module[CLAP_NAME_SIZE - 1] = '\0';
    
    return true;
}

bool MyPluginParameterManager::getParameterValue(clap_id param_id, double* value) const {
    if (param_id >= parameter_values.size()) {
        return false;
    }
    
    *value = parameter_values[param_id];
    return true;
}

bool MyPluginParameterManager::setParameterValue(clap_id param_id, double value) {
    if (param_id >= parameter_values.size()) {
        return false;
    }
    
    // 値を範囲内にクランプ (Clamp value to range)
    const auto& info = parameter_infos[param_id];
    value = std::max(info.min_value, std::min(info.max_value, value));
    
    parameter_values[param_id] = value;
    return true;
}

void MyPluginParameterManager::flushParameterChanges() {
    // パラメータ変更のフラッシュ処理 (Flush parameter changes)
    // TODO: ホストに変更を通知する実装 (TODO: Implement host notification)
}

double MyPluginParameterManager::getParameterValueNormalized(clap_id param_id) const {
    if (param_id >= parameter_values.size()) {
        return 0.0;
    }
    
    const auto& info = parameter_infos[param_id];
    double value = parameter_values[param_id];
    
    // 対数スケールが必要な周波数パラメータの処理 (Handle frequency parameters that need logarithmic scale)
    if (param_id == PARAM_EQ_POINT_1_FREQ || param_id == PARAM_EQ_POINT_2_FREQ || param_id == PARAM_EQ_POINT_3_FREQ) {
        // 対数スケールで正規化 (Normalize with logarithmic scale)
        double log_min = std::log(info.min_value);
        double log_max = std::log(info.max_value);
        double log_value = std::log(value);
        return (log_value - log_min) / (log_max - log_min);
    }
    
    // 線形正規化 (Linear normalization)
    return (value - info.min_value) / (info.max_value - info.min_value);
}

void MyPluginParameterManager::setParameterValueNormalized(clap_id param_id, double normalized_value) {
    if (param_id >= parameter_values.size()) {
        return;
    }
    
    const auto& info = parameter_infos[param_id];
    normalized_value = std::max(0.0, std::min(1.0, normalized_value));
    
    double actual_value;
    
    // 対数スケールが必要な周波数パラメータの処理 (Handle frequency parameters that need logarithmic scale)
    if (param_id == PARAM_EQ_POINT_1_FREQ || param_id == PARAM_EQ_POINT_2_FREQ || param_id == PARAM_EQ_POINT_3_FREQ) {
        // 対数スケールで実際の値を計算 (Calculate actual value with logarithmic scale)
        double log_min = std::log(info.min_value);
        double log_max = std::log(info.max_value);
        double log_value = log_min + normalized_value * (log_max - log_min);
        actual_value = std::exp(log_value);
    } else {
        // 線形スケール (Linear scale)
        actual_value = info.min_value + normalized_value * (info.max_value - info.min_value);
    }
    
    setParameterValue(param_id, actual_value);
}