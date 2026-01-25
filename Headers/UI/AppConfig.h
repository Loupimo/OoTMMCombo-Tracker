#pragma once

#include <QSettings>
#include <QDateTime>
#include <QStringList>

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

#pragma region Tracking Menu
    
    /*
    *   Gets the list of rencently opened files.
    *
    *   @return The list of the last opened files.
    */
    static QStringList GetRecentFiles();

    /*
    *   Tells if the last recent save should be automatically loaded.
    *
    *   @return <b>True</b> if the most recent file should be automatically loaded, <b>false</b> otherwise.
    */
    static bool GetAutoLoadTrackingFile();

    /*
    *   Tells if the last recent spoiler log should be automatically loaded.
    *
    *   @return <b>True</b> if the most recent spoiler log should be automatically loaded, <b>false</b> otherwise.
    */
    static bool GetAutoLoadSpoilerLog();

    /*
    *   Gets the path of the last opened spoiler log.
    *
    *   @return The path of the last opened spoiler log.
    */
    static QString GetLastSpoilerLogPath();

    /*
    *   Gets the use multiplayer state.
    *
    *   @return The use multiplayer state.
    */
    static bool GetUseMultiplayer();

    /*
    *   Sets the list of the last opened files.
    *
    *   @param NewValue The new list of the last opened files.
    */
    static void SetRecentFiles(QStringList NewValue);

    /*
    *   Sets the auto load most recent file state.
    *
    *   @param NewValue The new auto load most recent file state.
    */
    static void SetAutoLoadTrackingFile(bool NewValue);

    /*
    *   Sets the auto load most recent spoiler log state.
    *
    *   @param NewValue The new auto load most recent spoiler log state.
    */
    static void SetAutoLoadSpoilerLog(bool NewValue);
    
    /*
    *   Sets the most recent spoiler log path.
    *
    *   @param NewValue The new most recent spoiler log path.
    */
    static void SetLastSpoilerLogPath(QString NewValue);
    
    /*
    *   Sets the use multiplayer state.
    *
    *   @param NewValue The new use multiplayer state.
    */
    static void SetUseMultiplayer(bool NewValue);

#pragma endregion

#pragma region Options Menu

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
    *   @return <b>True</b> if the session should be automatically be saved when a new object is collected, <b>false</b> otherwise.
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
    *   Tells if the object should be hidden when collected.
    *
    *   @return <b>True</b> if the object should be hidden when collected, <b>false</b> otherwise.
    */
    static bool GetHideCollectedObject();

    /*
    *   Sets the auto save file path.
    *
    *   @param NewValue The new auto snap view value.
    */
    static void SetAutoSavePath(QString NewFilePath);

    /*
    *   Sets the auto snap view state.
    *
    *   @param NewFilePath The new auto save file path.
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

    /*
    *   Sets the hide collected object value.
    *
    *   @param NewValue The new hide collected object value.
    */
    static void SetHideCollectedObject(bool NewValue);

#pragma endregion

};