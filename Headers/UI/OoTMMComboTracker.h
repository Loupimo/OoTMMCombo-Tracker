#pragma once

#include <QtWidgets/QMainWindow>
#include <QTabWidget>
#include "ui_OoTMMComboTracker.h"
#include "LogTab.h"
#include "GameTab.h"

class OoTMMComboTracker : public QMainWindow
{
    Q_OBJECT

private:
    Ui::OoTMMComboTrackerClass ui;
    QTabWidget* TabWidget;
    LogTab* Log;
    GameTab* OoTTab;
    GameTab* MMTab;


public:
    OoTMMComboTracker(QWidget *parent = nullptr);
    ~OoTMMComboTracker();

    void UpdateTabNameText(int TabID);
    void LoadGameScenes(QString FilePath);
    void LoadGameSpoiler();

public slots:
    /*
    *   Updates the given object status and dispatch it to the corresponding scene view.
    *
    *   @param Game           The game the object belong to.
    *   @param ObjectFound    The object in which the item has been found.
    *   @param ItemFound      The item that has been found.
    */
    void UpdateTrackedObject(int Game, ObjectInfo* ObjectFound, const ItemInfo* ItemFound);
};
