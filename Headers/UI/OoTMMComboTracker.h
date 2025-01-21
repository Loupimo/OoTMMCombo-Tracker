#pragma once

#include <QtWidgets/QMainWindow>
#include <QTabWidget>
#include "ui_OoTMMComboTracker.h"
#include "LogTab.h"
#include "MapTab.h"

class OoTMMComboTracker : public QMainWindow
{
    Q_OBJECT

public:
    OoTMMComboTracker(QWidget *parent = nullptr);
    ~OoTMMComboTracker();

private:
    Ui::OoTMMComboTrackerClass ui;
    QTabWidget* TabWidget;
    LogTab* Log;
    MapTab* Maps;
};
