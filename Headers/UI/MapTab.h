#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QGraphicsScene>
#include "ui_OoTMMComboTracker.h"
#include "ObjectRenderer.h"

class MapTab : public QWidget
{
    Q_OBJECT

#pragma region Attributes

public:
    QHBoxLayout* MainLayout;
    QGraphicsScene* Scene;
    QGraphicsView* View;
    QPixmap* Map;
    std::vector<ObjectRenderer*> Objects;

#pragma endregion

public:
    MapTab(const QString& MapPath, QWidget* parent = nullptr);
    ~MapTab();

};