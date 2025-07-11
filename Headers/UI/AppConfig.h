#pragma once

#include <QSettings>
#include <QDateTime>

class AppConfig
{
public:

    QSettings Settings = QSettings("Loupimo", "OoTMMComboTracker");                                                          // The application persistant settings
    QString CurrentAutoSaveFile = "AutoSave-" + QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss") + ".trck";      // The current auto save file

public:


    /*
    *   Get the configuration instance.
    *
    *   @return The application configuration instance.
    */
    static AppConfig* Instance();

    /*
    *   Tells if the view should automatically be snapped to the last collected object.
    *
    *   @return <b>True</b> if the view should automatically snapped when an object is collected, <b>false</b> otherwise.
    */
    static bool GetAutoSnapView();

    /*
    *   Tells if the view should automatically be zoomed to the last collected object.
    *
    *   @return <b>True</b> if the view should automatically zoomed when an object is collected, <b>false</b> otherwise.
    */
    static bool GetAutoZoom();

    /*
    *   Tells if the session should automatically be saved when a new object is collected.
    *
    *   @return <b>True</b> if the should be automatically be saved when a new object is collected, <b>false</b> otherwise.
    */
    static bool GetAutoSave();

    /*
    *   Gets the auto save folder path.
    *
    *   @return The auto save folder path.
    */
    static QString GetAutoSavePath();


    /*
    *   Gets the auto save full file path.
    *
    *   @return The auto save full file path.
    */
    static QString GetAutoSaveFullPath();

    /*
    *   Sets the auto snap view state.
    *
    *   @param NewValue The new auto snap view value.
    */
    static void SetAutoSnapView(bool NewValue);

    /*
    *   Sets the auto zoom state.
    *
    *   @param NewValue The new auto zoom value.
    */
    static void SetAutoZoom(bool NewValue);

    /*
    *   Sets the auto save state.
    *
    *   @param NewValue The new auto save value.
    */
    static void SetAutoSave(bool NewValue);
};