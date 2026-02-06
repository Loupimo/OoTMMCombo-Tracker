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

/*
*   This class is responsible of handling all the launch options, logs and saving / loading features.
*/
class LogTab : public QWidget
{
    Q_OBJECT

#pragma region Attributes

public:

    // Layouts
    QVBoxLayout* MainLayout;
    QVBoxLayout* NetLayout;
    QHBoxLayout* MultiLayout;
    QHBoxLayout* FileLayout;
    QGridLayout* TrackLayout;
    QGroupBox* LaunchGroup;
    
    // Launch button
    QPushButton* ResetButton;
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
    QValidator* PortValidator;

    bool EnableMultiplayer;                 // Tells if multiplay is enabled or not
    bool IsRunning;                         // Tells if the tracker is running

    App * Tracker;                          // A reference to the Nax's app side
    OoTMMComboTracker* WinOwner = nullptr;  // A reference to the owning window
    std::thread TrackerThread;              // A thread used to run the Nax's app

#pragma endregion

public:

    /*
    *   Constructs the log tab and attach it to the given window.
    *
    *   @param Owner    The window that owns this log tab.
    *   @param Parent   The potential parent to attach this class to.
    */
    LogTab(OoTMMComboTracker* Owner, QWidget* parent = nullptr);

    /*
    *   The default destructor.
    */
    ~LogTab();

    /*
    *   Gets the spoiler log map array.
    *
    *   @return The spoiler log map array
    */
    const QHash<QString, QPair<uint32_t, uint32_t>> GetSpoilerMap();

    /*
    *   Toggle the network option regarding the given state.
    *
    *   @param State    The networking state : true = network enabled, false = network disabled.
    */
    void ToggleNetOption(int State);

    /*
    *   Start / stop the tracker.
    */
    void PressLaunchButton();

    /*
    *   Open a save file dialog to save the current tracking session.
    */
    void SaveTracking();

    /*
    *   Reset the tracker state.
    */
    void ResetTracking();

    /*
    *   Open a file dialog picker to load the desired tracking session.
    */
    void LoadTracking();

    /*
    *   Open a file dialog picker to load the desired spoiler log in order to load items names and attached them to their matching object.
    */
    void LoadSpoiler();


public slots:

    /*
    *   Logs the given message to the view.
    * 
    *   @param Message    The Message to log.
    */
    void LogMessage(const QString& Message);
};