#pragma once

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsView>
#include <QPixmap>
#include <QString>

#include <vector>
#include <cstdint>

#include "UI/ObjectRenderer.h"
#include "Combo/Entrances.h"

class EntranceGameTabView;
struct GlobalEntranceRow;
struct SceneEntranceMetaInf;
class GlobalEntranceTableModel;
class EntranceRenderer;
class EntranceItemTree;
class EntranceLinkItemTree;
class EntranceLabelItem;
class QGraphicsSceneHoverEvent;


enum TextPlacement
{
    Default = 0,
    Up = 1,
    Down = 2,
    Left = 3,
    Right = 4,
    NoText = 255
};


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

    /*
    *   Enter the highlighted state on the owning link so the paired label is highlighted too.
    *
    *   @param event    The hover event that triggered this function.
    */
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

    /*
    *   Leave the highlighted state on the owning link so the paired label is restored too.
    *
    *   @param event    The hover event that triggered this function.
    */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
};


/*
*   The graphical text item that displays the link name on top of the scene map.
*   Shares the same click / hover pipeline as EntrancePixmapItem so hovering or clicking either
*   one of the pair produces the same visual + behavioral feedback.
*/
class EntranceLabelItem : public QGraphicsSimpleTextItem
{
public:

    EntranceRenderer* Owner = nullptr;              // The renderer that owns this graphical item.
    EntranceLinkItemTree* ItemOwner = nullptr;      // The link item tree that owns this graphical item.

    /*
    *   Construct the label item with the given text and accept hover / mouse events.
    *
    *   @param Text       The initial text to display.
    *   @param PaOwner    The renderer that owns this item.
    *   @param PaItem     The link item tree that owns this graphical item.
    */
    EntranceLabelItem(const QString& Text, EntranceRenderer* PaOwner, EntranceLinkItemTree* PaItem);

protected:

    /*
    *   Forward the click to the owning link item tree as if the paired arrow had been clicked so
    *   the user can navigate to the destination scene by clicking either the arrow or its label.
    *
    *   @param event    The click event that triggered this function.
    */
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    /*
    *   Enter the highlighted state on the owning link so the paired arrow is highlighted too.
    *
    *   @param event    The hover event that triggered this function.
    */
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

    /*
    *   Leave the highlighted state on the owning link so the paired arrow is restored too.
    *
    *   @param event    The hover event that triggered this function.
    */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
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
    EntranceLabelItem* TextItem = nullptr;          // The optional name label painted next to the marker.

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
    *   Propagates the update to the on-map label (if any) so the map reflects newly discovered
    *   names without waiting for the next scene re-render.
    */
    void RefreshText();

    /*
    *   Sync the optional overlay label (text + on-map position) with the current InLinkName /
    *   OutLinkName of the backing row. No-op if no label is currently attached (e.g. the scene
    *   is not the one being rendered).
    */
    void UpdateOverlayLabel();

    /*
    *   Return the scene-space position (X, Y, Z) of this link side from the entrance meta info.
    *
    *   @return A 3-integer array pointer for the position, or nullptr if no meta info is found.
    */
    const int* GetPosition() const;

    /*
    *   Dispatch the click depending on the entry point:
    *       - tree click (CalledFromGraph == false): center the scene view on this link side,
    *       - map arrow click (CalledFromGraph == true): navigate to the destination scene.
    *   Both entry points share the same tree item so the name/state stays coherent with the model.
    */
    void PerformAction() override;

    /*
    *   Always return 1 so the tree filter never hides this leaf as "no objects available".
    *
    *   @return Always 1.
    */
    int GetTotalObjectAvailable() override;

    /*
    *   Resolve this link's target (destination entrance + game) via the scene meta info and ask the
    *   owning EntranceTab to focus the matching scene. Reads InLink / InLinkGame for the "in" side,
    *   OutLink / OutLinkGame for the "out" side. Silently returns if the link is undiscovered or if
    *   the target meta info / game tab cannot be resolved.
    */
    void NavigateToTarget();

    /*
    *   Tell whether this link side has been discovered yet. Reads the live scene meta info written
    *   by EntranceHelper, so the answer always reflects the current save state.
    *
    *   @return True if the destination entrance ID and game are both set, false otherwise.
    */
    bool IsTargetKnown() const;

    /*
    *   Focus this leaf in the entrance tree widget: expand all ancestors, set it as the current
    *   selection and scroll it into view. Used as the fallback action when the user clicks an
    *   undiscovered marker on the map (selecting the leaf is more useful than silently doing
    *   nothing because there is no destination scene to navigate to).
    */
    void SelectInTree();

    /*
    *   Apply or remove the highlighted visual state on both the on-map arrow and the name label so
    *   hovering either one of the pair makes both stand out.
    *
    *   @param Highlighted    True to highlight the pair, false to restore the default appearance.
    */
    void SetHighlighted(bool Highlighted);
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
    *   Center and zoom the scene view on the entrance position. The position is read from
    *   InPosition for Normal / OneWayIn entrances, OutPosition for OneWayOut entrances.
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
    std::vector<QGraphicsItem*> OverlayItems;       // Every text label + arrow pixmap painted on top of the current scene map, owned by the renderer for cleanup.

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
    *   Forget every registered entrance item and remove every overlay graphics item (name labels and
    *   arrow pixmaps) from the scene. Tree items are not deleted: they are owned by the tree widget
    *   and will be recreated on the next PopulateEntranceList call.
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
    *   Reset the view transform to a fixed zoom level and center the view on the given position.
    *   Used when the user clicks on an entrance parent in the tree, to make the entrance pair
    *   stand out without forcing the user to wheel-zoom afterwards.
    *
    *   @param Position    A 3-integer array with X, Y, Z coordinates (only X and Y are used).
    */
    void CenterAndZoomViewOn(const int* Position);

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

    /*
    *   Paint every in / out marker (name label + clickable arrow pixmap) on top of the current scene
    *   map for the given scene's entrances. Relies on the matching EntranceItemTree already being
    *   registered via RegisterEntrance, so PopulateEntranceList must run before this.
    *
    *   @param Scene    The scene whose entrance markers should be painted.
    */
    void RenderSceneOverlay(SceneEntranceMetaInf* Scene);

private:

    /*
    *   Create and register the label + arrow marker for one link side of an entrance on the map.
    *   Rotates the arrow by ArrowRot for the in side and ArrowRot + 180 for the out side so a
    *   normal entrance's two sides visually point in opposite directions.
    *
    *   @param Link    The link tree item that owns the marker (click target, name source).
    *   @param IsIn    True for the "in" side, false for the "out" side.
    *   @param Meta    The entrance meta info providing position + rotation.
    */
    void AddLinkMarker(EntranceLinkItemTree* Link, bool IsIn, const EntranceMetaInfo* Meta);

public:

    /*
    *   Place the given label around the arrow at (CenterX, CenterY). The Placement value selects
    *   the strategy: Default follows the arrow tip direction (the In / Out arrow pixmaps point to
    *   the right at rotation 0, so Normal entrances get their labels on opposite sides of the pair
    *   because their rotations differ by 180 deg), Up / Down / Left / Right pin the label to a
    *   fixed cardinal side regardless of the arrow rotation, and NoText hides the label entirely.
    *
    *   @param Label       The label item to position. No-op if null.
    *   @param CenterX     The X coordinate of the arrow center.
    *   @param CenterY     The Y coordinate of the arrow center.
    *   @param RotDeg      The arrow rotation in degrees (0 = pointing right).
    *   @param Placement   The TextPlacement value controlling the anchoring strategy.
    */
    static void PlaceLabelAroundArrow(QGraphicsSimpleTextItem* Label, qreal CenterX, qreal CenterY, qreal RotDeg, int Placement);

private:

    /*
    *   Build and cache the placeholder arrow pixmap used until real resources land. The pixmap is
    *   centered on its own origin so callers only need setPos(x, y) to place the arrow's center at
    *   (x, y) before applying rotation.
    *
    *   @return A reference to the cached placeholder arrow pixmap.
    */
    static const QPixmap& PlaceholderArrowPixmap();
};
