#include "UI/AppConfig.h"
#include <QFileDialog>

static AppConfig Config;

AppConfig* AppConfig::Instance()
{
    return &Config;
}

#pragma region Tracking Menu

QStringList AppConfig::GetRecentFiles()
{
    return Config.Settings.value("RecentFiles").toStringList();
}

bool AppConfig::GetAutoLoadTrackingFile()
{
    return Config.Settings.value("AutoLoadMostRecentTrackFile", true).toBool();
}

bool AppConfig::GetAutoLoadSpoilerLog()
{
    return Config.Settings.value("AutoLoadMostRecentSpoilerLog", true).toBool();
}

QString AppConfig::GetLastSpoilerLogPath()
{
    return Config.Settings.value("LastSpoilerPath").toString();
}

void AppConfig::SetRecentFiles(QStringList NewValue)
{
    Config.Settings.setValue("RecentFiles", NewValue);
}

void AppConfig::SetAutoLoadTrackingFile(bool NewValue)
{
    Config.Settings.setValue("AutoLoadMostRecentTrackFile", NewValue);
}

void AppConfig::SetAutoLoadSpoilerLog(bool NewValue)
{
    Config.Settings.setValue("AutoLoadMostRecentSpoilerLog", NewValue);
}

void AppConfig::SetLastSpoilerLogPath(QString NewValue)
{
    Config.Settings.setValue("LastSpoilerPath", NewValue);
}

#pragma endregion

#pragma region Option Menu

bool AppConfig::GetAutoSnapView()
{
    return Config.Settings.value("AutoSnapView", true).toBool();
}

bool AppConfig::GetAutoZoom()
{
    return Config.Settings.value("AutoZoom", true).toBool();
}

bool AppConfig::GetAutoSave()
{
    return Config.Settings.value("AutoSave", true).toBool();
}

QString AppConfig::GetAutoSavePath()
{
    return  QDir().absolutePath() + "/AutoSave/";
}

QString AppConfig::GetAutoSaveFullPath()
{
    return  AppConfig::GetAutoSavePath() + Config.CurrentAutoSaveFile;
}

void AppConfig::SetAutoSnapView(bool NewValue)
{
    Config.Settings.setValue("AutoSnapView", NewValue);
}

void AppConfig::SetAutoZoom(bool NewValue)
{
    Config.Settings.setValue("AutoZoom", NewValue);
}

void AppConfig::SetAutoSave(bool NewValue)
{
    Config.Settings.setValue("AutoSave", NewValue);
}

#pragma endregion