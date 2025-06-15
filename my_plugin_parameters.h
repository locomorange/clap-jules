#pragma once

#include <clap/clap.h>
#include <clap/ext/params.h>
#include <string>
#include <vector>

// パラメータID定義 (Parameter ID definitions)
enum MyPluginParameterIds {
    PARAM_DEPTH = 0,          // メインDepthノブ (Main Depth knob)
    PARAM_SHARPNESS,          // Sharpnessノブ (Sharpness knob)  
    PARAM_SELECTIVITY,        // Selectivityノブ (Selectivity knob)
    PARAM_ATTACK,             // Attackノブ (Attack knob)
    PARAM_RELEASE,            // Releaseノブ (Release knob)
    PARAM_QUALITY,            // Qualityスイッチ (Quality switch)
    PARAM_MODE,               // Modeラジオボタン (Mode radio button)
    PARAM_STEREO_LINK,        // Stereoリンクトグル (Stereo link toggle)  
    PARAM_BALANCE,            // Balanceスライダー (Balance slider)
    PARAM_OUTPUT_GAIN,        // 出力ゲイン (Output gain)
    PARAM_BYPASS,             // バイパススイッチ (Bypass switch)
    
    // EQカーブポイント用パラメータ (EQ curve point parameters)
    PARAM_EQ_POINT_1_FREQ,    // EQポイント1周波数 (EQ point 1 frequency)
    PARAM_EQ_POINT_1_GAIN,    // EQポイント1ゲイン (EQ point 1 gain)
    PARAM_EQ_POINT_2_FREQ,    // EQポイント2周波数 (EQ point 2 frequency)
    PARAM_EQ_POINT_2_GAIN,    // EQポイント2ゲイン (EQ point 2 gain)
    PARAM_EQ_POINT_3_FREQ,    // EQポイント3周波数 (EQ point 3 frequency)
    PARAM_EQ_POINT_3_GAIN,    // EQポイント3ゲイン (EQ point 3 gain)
    
    PARAM_COUNT               // パラメータ総数 (Total parameter count)
};

// パラメータ情報構造体 (Parameter information structure)
struct MyPluginParameterInfo {
    clap_id id;
    std::string name;
    std::string module;
    double min_value;
    double max_value;
    double default_value;
    clap_param_info_flags flags;
};

// パラメータマネージャークラス (Parameter manager class)
class MyPluginParameterManager {
public:
    MyPluginParameterManager();
    
    // CLAPパラメータ拡張用メソッド (Methods for CLAP parameter extension)
    uint32_t getParameterCount() const;
    bool getParameterInfo(uint32_t param_index, clap_param_info_t* param_info) const;
    bool getParameterValue(clap_id param_id, double* value) const;
    bool setParameterValue(clap_id param_id, double value);
    void flushParameterChanges();
    
    // GUI用メソッド (Methods for GUI)
    double getParameterValueNormalized(clap_id param_id) const;
    void setParameterValueNormalized(clap_id param_id, double normalized_value);
    
private:
    std::vector<MyPluginParameterInfo> parameter_infos;
    std::vector<double> parameter_values;
    
    void initializeParameters();
};