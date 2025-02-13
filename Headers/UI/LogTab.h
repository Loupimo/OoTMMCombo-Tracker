#pragma once

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
#include "Multi/App.h"
#include <thread>

class OoTMMComboTracker;

class LogTab : public QWidget
{
    Q_OBJECT

#pragma region Attributes

public:
    QVBoxLayout* MainLayout;
    QVBoxLayout* NetLayout;
    QHBoxLayout* MultiLayout;
    QHBoxLayout* FileLayout;
    QGroupBox* LaunchGroup;

    QPushButton* LaunchButton;

    // Save / Load options
    QPushButton* SaveButton;
    QPushButton* LoadButton;
    QPushButton* LoadSpoilerButton;

    // Network options
    QCheckBox* NetCheckBox;
    QPlainTextEdit* LogViewer;
    QLineEdit* Host;
    QLineEdit* Port;

    bool EnableMultiplayer;
    bool IsRunning;

    App * Tracker; 
    OoTMMComboTracker* WinOwner = nullptr;
    std::thread TrackerThread;

#pragma endregion

public:
    LogTab(OoTMMComboTracker* Owner, QWidget* parent = nullptr);
    ~LogTab();

    void ToggleNetOption(int state);
    void PressLaunchButton();
    void SaveTracking();
    void LoadTracking();
    void LoadSpoiler();

public slots:
    void LogMessage(const QString& Message);
};