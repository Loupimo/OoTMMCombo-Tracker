#pragma once

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include "ui_OoTMMComboTracker.h"
#include "ObjectRenderer.h"
#include "Multi/App.h"
#include <thread>

class LogTab : public QWidget
{
    Q_OBJECT

#pragma region Attributes

public:
    QVBoxLayout* MainLayout;
    QVBoxLayout* NetLayout;
    QHBoxLayout* MultiLayout;
    QGroupBox* LaunchGroup;
    QPushButton* LaunchButton;
    QCheckBox* NetCheckBox;
    QPlainTextEdit* LogViewer;
    QLineEdit* Host;
    QLineEdit* Port;

    bool EnableMultiplayer;
    bool IsRunning;

    App * Tracker;
    std::thread TrackerThread;

#pragma endregion

public:
    LogTab(QWidget* parent = nullptr);
    ~LogTab();

    void ToggleNetOption(int state);
    void PressLaunchButton();

public slots:
    void LogMessage(const QString& Message);
};