#pragma once

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPixmap>
#include <QString>

#include <vector>
#include <cstdint>

#include "UI/ObjectRenderer.h"
#include "Combo/Entrances.h"

class EntranceGameTabView;
struct GlobalEntranceRow;
class GlobalEntranceTableModel;
class EntranceRenderer;
class EntranceItemTree;
class EntranceLinkItemTree;


/*
*   The graphical pixmap item that represents one link side of an entrance on the scene map.
*   Skeleton for future graphical integration: the final icon resources are not yet available.
*/
class EntrancePixmapItem : public QGraphicsPixmapItem
{
public:

    EntranceRenderer* Owner = nullptr;              // The renderer that owns this graphical item.
    EntranceLinkItemTree* ItemOwner = nullptr;      // The link item tree that owns this graphical item.

    /*
    *   Construct a graphical item using the given pixmap.
    *
    *   @param Pixmap      The pixmap that represents this link side.
    *   @param PaOwner     The renderer that owns this item.
    *   @param PaItem      The link item tree that owns this graphical item.
    */
    EntrancePixmapItem(const QPixmap& Pixmap, EntranceRenderer* PaOwner, EntranceLinkItemTree* PaItem);

protected:

    /*
    *   Forward the click to the owning link item tree so the same action is triggered as a tree click.
    *
    *   @param event    The click event that triggered this function.
    */
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
};


/*
*   The tree item that displays one link side (in or out) of an entrance.
*   Reads its display text from the matching GlobalEntranceRow of the model so it always stays in sync with the table.
*/
class EntranceLinkItemTree : public CommonBaseItemTree
{
public:

    EntranceItemTree* EntranceItem;                 // The owning entrance item tree.
    bool IsInLink;                                  // True for the "in" side, false for the "out" side.
    EntrancePixmapItem* GraphItem = nullptr;        // The optional graphical marker on the scene map.

    /*
    *   Construct the link item tree for the given side and refresh its displayed text.
    *
    *   @param PaEntrance    The owning entrance item tree.
    *   @param InIsInLink    True for the "in" side, false for the "out" side.
    *   @param Parent        The parent tree item to attach this item to.
    */
    EntranceLinkItemTree(EntranceItemTree* PaEntrance, bool InIsInLink, QTreeWidgetItem* Parent = nullptr);

    /*
    *   Refresh the displayed text from the InLinkName / OutLinkName of the backing model row.
    */
    void RefreshText();

    /*
    *   Return the scene-space position (X, Y, Z) of this link side from the entrance meta info.
    *
    *   @return A 3-integer array pointer for the position, or nullptr if no meta info is found.
    */
    const int* GetPosition() const;

    /*
    *   Center the scene view on the position of this link side when the user clicks on it.
    */
    void PerformAction() override;

    /*
    *   Always return 1 so the tree filter never hides this leaf as "no objects available".
    *
    *   @return Always 1.
    */
    int GetTotalObjectAvailable() override;
};


/*
*   The tree item that wraps a single entrance of a scene.
*   Holds 1 link child for one-way entrances and 2 link children for normal entrances.
*   Looks its data up in the model's m_rows on demand to avoid duplication.
*/
class EntranceItemTree : public CommonBaseItemTree
{
public:

    EntranceRenderer* RendererOwner;                // The renderer that owns this item.
    uint32_t SceneID;                               // The scene ID this entrance belongs to.
    uint32_t EntranceID;                            // The entrance ID this item represents.
    EntranceLinkItemTree* InItem = nullptr;         // The "in" side child item, if any.
    EntranceLinkItemTree* OutItem = nullptr;        // The "out" side child item, if any.

    /*
    *   Construct the entrance item tree and create its link children based on the entrance type.
    *
    *   @param Row         The model row that describes this entrance.
    *   @param PaOwner     The renderer that owns this item.
    *   @param Parent      The parent tree item to attach this item to.
    */
    EntranceItemTree(GlobalEntranceRow* Row, EntranceRenderer* PaOwner, QTreeWidgetItem* Parent = nullptr);

    /*
    *   Find the matching row in the model's m_rows by scene ID and entrance ID.
    *
    *   @return The matching row pointer, or nullptr if not found.
    */
    GlobalEntranceRow* GetRow() const;

    /*
    *   Get the meta info of the wrapped entrance for the owning game.
    *
    *   @return The matching entrance meta info, or nullptr if not found.
    */
    const EntranceMetaInfo* GetMetaInfo() const;

    /*
    *   Refresh the displayed entrance name and its link children from the backing model row.
    */
    void RefreshText();

    /*
    *   Center the scene view on the first available link side when the user clicks on the entrance parent.
    */
    void PerformAction() override;

    /*
    *   Return the number of link children so the filter only shows entrances with at least one side.
    *
    *   @return The number of link children (1 or 2).
    */
    int GetTotalObjectAvailable() override;
};


/*
*   The renderer that owns the entrance tree items currently displayed for one scene.
*   Mirrors the ObjectRenderer role: owns the graphical scene, the view used for centering and the list of tree items.
*/
class EntranceRenderer
{
public:

    EntranceGameTabView* Owner;                     // The game tab view that owns this renderer.
    QGraphicsScene* Scene = nullptr;                // The graphics scene used to render the map (and future pixmaps).
    QGraphicsView* View = nullptr;                  // The graphics view used to center on a position.
    std::vector<EntranceItemTree*> Entrances;       // The list of entrance items currently registered.

    /*
    *   Construct the entrance renderer owned by the given game tab view.
    *
    *   @param PaOwner    The owning game tab view.
    */
    EntranceRenderer(EntranceGameTabView* PaOwner);

    /*
    *   Set the graphics scene and view the renderer targets for centering and future graphical rendering.
    *
    *   @param PaScene   The graphics scene.
    *   @param PaView    The graphics view.
    */
    void SetTarget(QGraphicsScene* PaScene, QGraphicsView* PaView);

    /*
    *   Forget every currently registered entrance item.
    *   Does not delete the items themselves (their parent tree widget is expected to own them).
    */
    void Clear();

    /*
    *   Register an entrance item so the renderer can refresh or look it up later.
    *
    *   @param Item    The entrance item to register.
    */
    void RegisterEntrance(EntranceItemTree* Item);

    /*
    *   Find the registered entrance item matching the given scene and entrance IDs.
    *
    *   @param SceneID       The scene ID to match.
    *   @param EntranceID    The entrance ID to match.
    *
    *   @return The matching entrance item, or nullptr if not found.
    */
    EntranceItemTree* FindEntrance(uint32_t SceneID, uint32_t EntranceID);

    /*
    *   Center the scene view on the given position.
    *
    *   @param Position    A 3-integer array with X, Y, Z coordinates (only X and Y are used for centering).
    */
    void CenterViewOn(const int* Position);

    /*
    *   Get the game ID of the owning game tab view.
    *
    *   @return The game ID, or -1 if no owner.
    */
    int GetGameID() const;

    /*
    *   Get the global entrance table model from the owning game tab view.
    *
    *   @return The global entrance table model, or nullptr if not available.
    */
    GlobalEntranceTableModel* GetModel() const;
};
