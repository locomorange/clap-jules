#include "plugin_gui.h"
#include <QtWidgets/QApplication>
#include <QtCore/QDebug>

PluginWidget::PluginWidget(QWidget *parent)
    : QWidget(parent)
    , m_layout(new QVBoxLayout(this))
    , m_titleLabel(new QLabel("My CLAP Plugin GUI", this))
    , m_volumeLabel(new QLabel("Volume: 50", this))
    , m_volumeSlider(new QSlider(Qt::Horizontal, this))
    , m_testButton(new QPushButton("Test Button", this))
    , m_host(nullptr)
{
    setWindowTitle("My CLAP Plugin");
    setFixedSize(300, 200);
    
    // Setup slider
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(50);
    
    // Add widgets to layout
    m_layout->addWidget(m_titleLabel);
    m_layout->addWidget(m_volumeLabel);
    m_layout->addWidget(m_volumeSlider);
    m_layout->addWidget(m_testButton);
    
    // Connect signals
    connect(m_volumeSlider, &QSlider::valueChanged, this, &PluginWidget::onVolumeChanged);
    connect(m_testButton, &QPushButton::clicked, this, &PluginWidget::onButtonClicked);
    
    // Style the widget
    setStyleSheet(
        "QWidget {"
        "    background-color: #2b2b2b;"
        "    color: white;"
        "    font-family: Arial, sans-serif;"
        "}"
        "QLabel {"
        "    font-size: 14px;"
        "    margin: 5px;"
        "}"
        "QSlider::groove:horizontal {"
        "    border: 1px solid #999999;"
        "    height: 8px;"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);"
        "    margin: 2px 0;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);"
        "    border: 1px solid #5c5c5c;"
        "    width: 18px;"
        "    margin: -2px 0;"
        "    border-radius: 3px;"
        "}"
        "QPushButton {"
        "    background-color: #404040;"
        "    border: 1px solid #606060;"
        "    padding: 8px;"
        "    border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #505050;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #303030;"
        "}"
    );
}

PluginWidget::~PluginWidget()
{
}

void PluginWidget::setHost(const clap_host_t *host)
{
    m_host = host;
}

void PluginWidget::onVolumeChanged(int value)
{
    m_volumeLabel->setText(QString("Volume: %1").arg(value));
    qDebug() << "Volume changed to:" << value;
}

void PluginWidget::onButtonClicked()
{
    qDebug() << "Test button clicked!";
    m_testButton->setText(m_testButton->text() == "Test Button" ? "Clicked!" : "Test Button");
}

