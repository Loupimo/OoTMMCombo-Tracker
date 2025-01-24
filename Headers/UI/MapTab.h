#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QGraphicsScene>
#include "ui_OoTMMComboTracker.h"
#include "SceneRenderer.h"

class MapTab : public QWidget
{
    Q_OBJECT

#pragma region Attributes

public:
    QHBoxLayout* MainLayout;
    QGraphicsView* View;
    QPixmap* Map;

    SceneRenderer* SceneToRender;

#pragma endregion

public:
    MapTab(QWidget* parent = nullptr);
    ~MapTab();

    void RenderTab();
    void UnloadTab();
};