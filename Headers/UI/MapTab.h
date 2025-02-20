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

class ToggleSwitch : public QWidget {
    Q_OBJECT

public:

    MapTab* Owner;

private:
    QPushButton* button;
    QLabel* circle;
    QFrame* background;
    QPropertyAnimation* moveAnimation;
    QPropertyAnimation* colorAnimation;

public:
    explicit ToggleSwitch(MapTab* Owner, QWidget* parent = nullptr) : QWidget(parent) {
        
        this->Owner = Owner;
        setFixedSize(70, 30);

        // Création du fond
        background = new QFrame(this);
        background->setStyleSheet("background-color: #ccc; border-radius: 15px;");
        background->setGeometry(0, 0, 70, 30);

        // Création du cercle mobile (le "point" du switch)
        circle = new QLabel(this);
        circle->setFixedSize(26, 26);
        circle->setStyleSheet("background-color: white; border-radius: 13px;");
        circle->move(2, 2);

        // Animation du déplacement
        moveAnimation = new QPropertyAnimation(circle, "pos");
        moveAnimation->setDuration(200);
        moveAnimation->setEasingCurve(QEasingCurve::InOutQuad);

        // Animation du fond
        colorAnimation = new QPropertyAnimation(this, "backgroundColor");
        colorAnimation->setDuration(200);
        colorAnimation->setEasingCurve(QEasingCurve::InOutQuad);

        // Bouton invisible qui détecte les clics
        button = new QPushButton(this);
        button->setCheckable(true);
        button->setStyleSheet("background: transparent;");
        button->setGeometry(0, 0, 70, 30);

        // Connexion du bouton à l'animation
        connect(button, &QPushButton::toggled, this, &ToggleSwitch::animateSwitch);
    }

    ~ToggleSwitch()
    {
        delete moveAnimation;
        delete colorAnimation;
        delete button;
        delete circle;
        delete background;
    }

    bool GetContext()
    {
        return this->button->isChecked();
    }

    void UpdateContext(ObjectContext Context);

private slots:
    void animateSwitch(bool checked);

protected:
    void setBackgroundColor(QColor color) {
        background->setStyleSheet(QString("background-color: %1; border-radius: 15px;").arg(color.name()));
    }

    QColor getBackgroundColor() const {
        return background->palette().color(QPalette::Window);
    }

    Q_PROPERTY(QColor backgroundColor READ getBackgroundColor WRITE setBackgroundColor)

};


class MapView : public QGraphicsView
{
    Q_OBJECT

public:

    MapTab* Owner;

public:
    explicit MapView(MapTab* Owner, QWidget* parent = nullptr) : QGraphicsView(parent)
    {
        this->Owner = Owner;
        setRenderHint(QPainter::Antialiasing);
        setRenderHint(QPainter::SmoothPixmapTransform);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);  // Zoom centré sur la souris
        setDragMode(QGraphicsView::ScrollHandDrag);  // Permet de déplacer la vue avec la souris
    }

    void UpdateContext(ObjectContext Context);

protected:
    void wheelEvent(QWheelEvent* event) override
    {
        const double scaleFactor = 1.15; // Facteur de zoom
        if (event->angleDelta().y() > 0)
            scale(scaleFactor, scaleFactor);  // Zoom avant
        else
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);  // Zoom arrière
    }
};


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
    ToggleSwitch* SwitchButton;

    // Map Tree
    QLineEdit* MapSearchBar;
    QTreeWidget* MapList;

    // Object Tree
    QLineEdit* ObjectSearchBar;
    QTreeWidget* ObjectList;

    // Scenes
    std::vector<RegionTree*> Regions;
    SceneItemTree* RenderedScene = nullptr;
    QHash<int, SceneItemTree*> Scenes;          // All the scenes available

    // Flags
    CommonBaseItemTree* PrevSelected = nullptr; // The previously selected object in the list
    bool SelectionUpdated = false;              // Tells if the object has already been updated. Used to avoid duplicates between selection changed and click signals


#pragma endregion

public:
    MapTab(GameTab* Owner, int Game, SceneInfo* Scene, size_t NumOfScenes, QWidget* parent = nullptr);
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

    void RefreshScenesObjectCounts();

    void UpdateContext(ObjectContext Context);
    void ContextSwitch(bool NewState);
    void UpdateObjectSelection();
};