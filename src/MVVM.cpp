#include "MVVM.h"
#include "AudioProcessor.h"
#include <algorithm>

namespace ClapeJules {

// Parameter implementation
void Parameter::setValue(float value) {
    value = std::clamp(value, m_minValue, m_maxValue);
    m_value.store(value);
    
    if (onValueChanged) {
        onValueChanged(value);
    }
}

float Parameter::getValue() const {
    return m_value.load();
}

// FilterViewModel implementation
FilterViewModel::FilterViewModel(std::shared_ptr<AudioProcessor> processor)
    : m_audioProcessor(processor) {
    
    // Create frequency parameter (20Hz to 20kHz)
    m_frequencyParam = std::make_shared<Parameter>(1000.0f, 20.0f, 20000.0f);
    
    // Bind parameter to audio processor
    m_frequencyParam->onValueChanged = [this](float frequency) {
        onFrequencyChanged(frequency);
    };
}

FilterViewModel::~FilterViewModel() = default;

void FilterViewModel::onFrequencyChanged(float frequency) {
    if (m_audioProcessor) {
        m_audioProcessor->setFrequency(frequency);
    }
}

// FilterView implementation
FilterView::FilterView(std::shared_ptr<FilterViewModel> viewModel)
    : m_viewModel(viewModel) {
}

FilterView::~FilterView() = default;

std::shared_ptr<Parameter> FilterView::getFrequencyParameter() const {
    if (m_viewModel) {
        return m_viewModel->getFrequencyParameter();
    }
    return nullptr;
}

} // namespace ClapeJules