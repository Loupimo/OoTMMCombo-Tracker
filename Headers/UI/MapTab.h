#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QGraphicsScene>
#include <QComboBox>
#include <QTreeWidget>
#include <QLineEdit>
#include <QSplitter>
#include "RegionTab.h"
#include "ui_OoTMMComboTracker.h"
#include "SceneRenderer.h"

class MapTab : public QWidget
{
    Q_OBJECT

#pragma region Attributes

public:

    // Containers
    QWidget* MapContainer;
    QWidget* ObjectContainer;

    // Layouts
    QHBoxLayout* MainLayout;
    QVBoxLayout* MapTreeLayout;
    QVBoxLayout* ObjectTreeLayout;
    QSplitter* LayoutSplitter;

    // Main View
    QGraphicsView* View;

    // Map Tree
    QLineEdit* MapSearchBar;
    QTreeWidget* MapList;

    // Object Tree
    QLineEdit* ObjectSearchBar;
    QTreeWidget* ObjectList;

    // Scenes
    std::vector<RegionTree*> Regions;
    std::vector<SceneRenderer *> ScenesToRender;
    SceneRenderer* RenderedScene = nullptr;
    int ActiveSceneID = -1;

#pragma endregion

public:
    MapTab(int Game, SceneInfo* Scene, size_t NumOfScenes, QWidget* parent = nullptr);
    ~MapTab();

    void RenderMap();
    void UnloadMap();
    void ChangeActiveScene(int NewIndex);
    RegionTree* FindRegionTree(uint8_t Region);
    void FilterTree(QTreeWidget* TreeWidget, const QString& SearchText);
};