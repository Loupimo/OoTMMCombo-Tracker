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
#include "UI/MemoryReader.h"
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
    /* Main vertical layout that contains all sub-layouts and groups in the tab. */
    QVBoxLayout* MainLayout;
    QVBoxLayout* NetLayout;
    QHBoxLayout* MultiLayout;
    QHBoxLayout* FileLayout;
    QGridLayout* TrackLayout;
    /* Group box wrapping the tracker launch controls. */
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
    /* Text area that displays tracker log messages at runtime. */
    QPlainTextEdit* LogViewer;
    QLineEdit* Host;
    QLineEdit* Port;
    QValidator* PortValidator;

    /* Tells if multiplayer networking is enabled. */
    bool EnableMultiplayer;                 // Tells if multiplay is enabled or not
    bool IsRunning;                         // Tells if the tracker is running

    App * Tracker;                          // A reference to the Nax's app side
    /* The memory reader that polls shared DLL memory to track rando progression. */
    MemoryReader * MemRead;                 // The memory reader used to directly track the rando progression.
    OoTMMComboTracker* WinOwner = nullptr;  // A reference to the owning window
    std::thread TrackerThread;              // A thread used to run the Nax's app
    std::thread MemReaderThread;            // A thread used to run the memory reader

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
    *   Set the "Use Multiplayer" checkbox state. Checking / unchecking the box propagates
    *   through ToggleNetOption, so the enabled flag, the host / port fields and the persisted
    *   config all stay in sync. Used to auto-enable multiplayer when a coop / multiworld
    *   spoiler is loaded and disable it for a single-world spoiler.
    *
    *   @param Enabled    True to enable multiplayer, false to disable it.
    */
    void SetMultiplayerEnabled(bool Enabled);

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