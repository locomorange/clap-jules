#pragma once

#include <functional>
#include <memory>
#include <atomic>
#include "common/dll_export.h"

namespace ClapeJules {

// Forward declarations
class AudioProcessor;

// Simple Parameter for MVVM binding
class CLAP_JULES_EXPORT Parameter {
public:
    Parameter(float initialValue, float minValue, float maxValue)
        : m_value(initialValue), m_minValue(minValue), m_maxValue(maxValue) {}
    
    void setValue(float value);
    float getValue() const;
    float getMinValue() const { return m_minValue; }
    float getMaxValue() const { return m_maxValue; }
    
    // Callback for value changes
    std::function<void(float)> onValueChanged;

private:
    std::atomic<float> m_value;
    float m_minValue;
    float m_maxValue;
};

// ViewModel for MVVM pattern
class CLAP_JULES_EXPORT FilterViewModel {
public:
    explicit FilterViewModel(std::shared_ptr<AudioProcessor> processor);
    ~FilterViewModel();

    // Parameter access
    std::shared_ptr<Parameter> getFrequencyParameter() const { return m_frequencyParam; }

private:
    std::shared_ptr<AudioProcessor> m_audioProcessor;
    std::shared_ptr<Parameter> m_frequencyParam;
    
    void onFrequencyChanged(float frequency);
};

// Simple UI View
class CLAP_JULES_EXPORT FilterView {
public:
    explicit FilterView(std::shared_ptr<FilterViewModel> viewModel);
    ~FilterView();
    
    // For now, just provide parameter access for CLAP parameter implementation
    std::shared_ptr<Parameter> getFrequencyParameter() const;

private:
    std::shared_ptr<FilterViewModel> m_viewModel;
};

} // namespace ClapeJules