#include "UI/AppConfig.h"
#include <QFileDialog>

static AppConfig Config;

AppConfig* AppConfig::Instance()
{
    return &Config;
}

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