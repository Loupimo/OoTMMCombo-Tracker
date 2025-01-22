#pragma once

#include <QtWidgets/QMainWindow>
#include <QTabWidget>
#include "ui_OoTMMComboTracker.h"
#include "LogTab.h"
#include "GameTab.h"

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
    GameTab OoTTab = GameTab(OOT_GAME);
    GameTab MMTab = GameTab(MM_GAME);
};
