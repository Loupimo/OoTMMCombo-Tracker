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
    //QPixmap* Map;

    std::vector<SceneRenderer *> ScenesToRender;
    SceneRenderer* RenderedScene = nullptr;

#pragma endregion

public:
    MapTab(SceneInfo* Scene, size_t NumOfScenes, QWidget* parent = nullptr);
    ~MapTab();

    void RenderTab();
    void UnloadTab();
};