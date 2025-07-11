#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QGraphicsScene>
#include <QPushButton>
#include <QTreeWidget>
#include <QLineEdit>
#include <QLabel>
#include <QSplitter>
#include <QPropertyAnimation>
#include "RegionTab.h"
#include "ui_OoTMMComboTracker.h"
#include "SceneRenderer.h"

class GameTab;
class MapTab;

/*
*   This class is responsible of handling the context switch option.
*/
class ContextSwitchButton : public QWidget {
    Q_OBJECT

public:

    MapTab* Owner;                      // A reference to the map tab that owns this view

private:
    QPushButton* Button;                // The push button that holds the context state
    QLabel* Circle;                     // The moving circle of the button
    QFrame* Background;                 // The background style of the button
    QPropertyAnimation* MoveAnimation;  // The cursor movement animation
    QPropertyAnimation* ColorAnimation; // The button background fading animation

public:

    /*
    *   Constructs the button with the given parameter.
    *
    *   @param Owner    The map tab that owns this object.
    *   @param Parent   The potential parent to attach this class to.
    */
    ContextSwitchButton(MapTab* Owner, QWidget* parent = nullptr);

    /*
    *   The default destructor.
    */
    ~ContextSwitchButton();

    /*
    *   Get the current context switch button position.
    *
    *   @return True if the context is Adult or Spring, false is context is Child or Winter.
    */
    bool GetContext();

    /*
    *   Update the switch position based on the given context.
    *
    *   @param Context    The context to put the switch to.
    */
    void UpdateContext(ObjectContext Context);

    /*
    *   Animate the switch button regarding its current position and the desired state.
    *
    *   @param Checked    The final switch state : True means ends on rigth side, false on left side.
    */
    void AnimateSwitch(bool Checked);

protected:

    /*
    *   Set the button background color.
    *
    *   @param Color    The new background color to use.
    */
    void SetBackgroundColor(QColor Color);

    /*
    *   Get the current button background color.
    *
    *   @return The current button background color.
    */
    QColor GetBackgroundColor() const;

    Q_PROPERTY(QColor backgroundColor READ GetBackgroundColor WRITE SetBackgroundColor)
};


/*
*   This class is responsible of handling the view in which the scene is rendered.
*/
class MapView : public QGraphicsView
{
    Q_OBJECT

public:

    MapTab* Owner;  // A reference to the map tab that owns this view

public:

    /*
    *   Constructs the map view with the given parameter.
    *
    *   @param Owner    The map tab that owns this object.
    *   @param Parent   The potential parent to attach this class to.
    */
    MapView(MapTab* Owner, QWidget* Parent = nullptr);

    /*
    *   Switch the current map context with the desired one.
    *
    *   @param Context  The context used to set up the switch.
    */
    void UpdateContext(ObjectContext Context);

protected:

    /*
    *   Zoom or unzoom the camera on weel movement.
    * 
    *   @param event    The actual weel event that triggered this function.
    */
    void wheelEvent(QWheelEvent* event) override;
};


/*
*   This class is responsible of holding all elements related to scene, region, object and graphical layout for a specific game.
*/
class MapTab : public QWidget
{
    Q_OBJECT

#pragma region Attributes

public:

    // Containers
    QWidget* MapContainer;
    QWidget* ObjectContainer;
    QWidget* SwitchContainer;

    // Layouts
    QHBoxLayout* MainLayout;
    QHBoxLayout* SwitchLayout;
    QVBoxLayout* MapTreeLayout;
    QVBoxLayout* ObjectTreeLayout;
    QSplitter* LayoutSplitter;

    // Main View
    MapView* View;

    // Switch Button
    QLabel* LeftIcon;
    QLabel* RightIcon;
    ContextSwitchButton* SwitchButton;

    // Map Tree
    QLineEdit* MapSearchBar;
    QTreeWidget* MapList;
    QPushButton* MapTreeToggleButton;
    bool IsMapExpanded = false;

    // Object Tree
    QLineEdit* ObjectSearchBar;
    QTreeWidget* ObjectList;
    QPushButton* ObjectTreeToggleButton;
    bool IsObjectExpanded = true;

    // Scenes
    std::vector<RegionTree*> Regions;           // The list of all available regions
    SceneItemTree* RenderedScene = nullptr;     // The currently rendered scene
    QHash<int, SceneItemTree*> Scenes;          // All the scenes available

    // Flags
    CommonBaseItemTree* PrevSelected = nullptr; // The previously selected object in the list
    bool SelectionUpdated = false;              // Tells if the object has already been updated. Used to avoid duplicates between selection changed and click signals


#pragma endregion

#pragma region Class creation / loading

public:

    /*
    *   Build all game map that match given game / scenes.
    */
    MapTab(GameTab* Owner, int Game, SceneInfo* Scene, size_t NumOfScenes, QWidget* parent = nullptr);

    /*
    *   Default destructor.
    */
    ~MapTab();

    /*
    *   Find the desired region.
    *
    *   @param Region    The region ID to match.
    *
    *   @return The matching region if found, nullptr otherwise.
    */
    RegionTree* FindRegionTree(uint8_t Region);

    /*
    *   Refresh the object counts of all scenes.
    */
    void RefreshScenesObjectCounts();

    /*
    *   Hide or unhide tree elements regarding the given string.
    *
    *   @param TreeWidget      The tree to filter.
    *   @param SearchText      The text to match.
    */
    void FilterTree(QTreeWidget* TreeWidget, const QString& SearchText);

    void OnToggleExpandCollapse(QTreeWidget* TreeWidget, bool Expand);

#pragma endregion

#pragma region Context

public:

    /*
    *   Update the context of the switch button.
    *
    *   @param Context      The context in which the button should be put.
    */
    void UpdateContext(ObjectContext Context);

    /*
    *   Refresh the context of the current rendered scene regarding the given state.
    *
    *   @param NewState      The context in which the scene should be.
    */
    void ContextSwitch(bool NewState);

#pragma endregion

#pragma region Map Tree / Scene Rendering

public:

    /*
    *   Render the current active scene.
    */
    void RenderMap();

    /*
    *   Unload the current rendered scene.
    */
    void UnloadMap();

    /*
    *   Change the current active scene.
    *
    *   @param Current    The new scene to render.
    *   @param Previous   The previously rendered scene.
    */
    void ChangeActiveScene(QTreeWidgetItem* Current, QTreeWidgetItem* Previous);

#pragma endregion

#pragma region Object Tree / Object changed

public:

    /*
    *   Update the matching scene object.
    *
    *   @param Object       The object in which the item has been found.
    *   @param ItemFound    The item that has been found.
    *
    *   @warning This should be executed by the main thread only at it can modify the GUI elements.
    */
    void ItemFound(ObjectInfo* Object, const ItemInfo* ItemFound);

    /*
    *   Perform the action on the clicked object.
    *
    *   @param Item      The clicked object.
    *   @param Column    The item column (not used).
    */
    void ObjectClicked(QTreeWidgetItem* Item, int Column);

    /*
    *   Update the object list selected item.
    *
    *   @warning This function does a lot of things that modify some flags that could have potential side effect on other function.
    */
    void UpdateObjectSelection();

#pragma endregion

};