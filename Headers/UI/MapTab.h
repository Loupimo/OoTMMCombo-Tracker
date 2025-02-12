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
    //std::vector<SceneRenderer *> ScenesToRender;
    //SceneRenderer* RenderedScene = nullptr;
    //int ActiveSceneID = -1;
    SceneItemTree* RenderedScene = nullptr;
    QHash<int, SceneItemTree*> Scenes;

#pragma endregion

public:
    MapTab(int Game, SceneInfo* Scene, size_t NumOfScenes, QWidget* parent = nullptr);
    ~MapTab();

    void RenderMap();
    void UnloadMap();
    void ChangeActiveScene (QTreeWidgetItem* Current, QTreeWidgetItem* Previous);
    //void ChangeActiveScene(int NewIndex);
    RegionTree* FindRegionTree(uint8_t Region);
    void ChangeObjectState(QTreeWidgetItem* Item, int Column);
    void FilterTree(QTreeWidget* TreeWidget, const QString& SearchText);


    /*
    *   Update the matching scene object.
    *
    *   @param Object       The object in which the item has been found.
    *   @param ItemFound    The item that has been found.
    *
    *   @warning This should be executed by the main thread only at it can modify the GUI elements.
    */
    void ItemFound(ObjectInfo* Object, const ItemInfo* ItemFound);
};