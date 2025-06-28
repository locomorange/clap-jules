#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QPushButton>
#include <clap/clap.h>

class PluginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PluginWidget(QWidget *parent = nullptr);
    ~PluginWidget();

    void setHost(const clap_host_t *host);

private slots:
    void onVolumeChanged(int value);
    void onButtonClicked();

private:
    QVBoxLayout *m_layout;
    QLabel *m_titleLabel;
    QLabel *m_volumeLabel;
    QSlider *m_volumeSlider;
    QPushButton *m_testButton;
    
    const clap_host_t *m_host;
};