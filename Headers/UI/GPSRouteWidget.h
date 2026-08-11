#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QVector>
#include "UI/GPSRouteModel.h"


class GPSRouteCard;


/*
*   Top-level GPS Route container hosted as the 3rd sub-tab of EntranceTab.
*       - Top bar: "GPS" tag, From combo, arrow, To combo, "Inverser" button, summary label.
*       - Below: a horizontal row of GPSRouteCard widgets, one per Route returned by SampleRoutes().
*
*   This iteration is purely visual: there is no real pathfinder yet; selection
*   changes simply re-call SampleRoutes(From, To) and repopulate the cards.
*/
class GPSRouteWidget : public QWidget
{
    Q_OBJECT

public:

    #pragma region // Attributes

    QVBoxLayout*            MainLayout = nullptr;
    QHBoxLayout*            TopBar = nullptr;
    QHBoxLayout*            CardsRow = nullptr;
    QWidget*                CardsHost = nullptr;        // Container of the route cards. Layout is CardsRow.
    QScrollArea*            CardsScroll = nullptr;      // Scroll viewport wrapping CardsHost so long routes don't grow the window.

    QLabel*                 Tag = nullptr;
    QComboBox*              FromCombo = nullptr;
    QComboBox*              FromEntranceCombo = nullptr;    // Departure entrance within FromCombo's scene ("Any" by default).
    QLabel*                 ArrowLabel = nullptr;
    QComboBox*              ToCombo = nullptr;
    QComboBox*              ToEntranceCombo = nullptr;      // Arrival entrance within ToCombo's scene ("Any" by default).
    QPushButton*            SwapButton = nullptr;
    QLabel*                 Summary = nullptr;
    QLabel*                 Placeholder = nullptr;      // Shown when there are no routes to display.

    QVector<GPSRouteCard*>  Cards;

    #pragma endregion


    #pragma region // Class creation

    /*
    *   Construct the GPS route widget, populate the scene combos from the OoT
    *   and MM scene meta tables, and trigger an initial route render.
    *
    *   @param Parent    The Qt parent widget.
    */
    explicit GPSRouteWidget(QWidget* Parent = nullptr);

    #pragma endregion


    #pragma region // Methods

    /*
    *   Replace the current cards with one card per route in the given list. Also
    *   refreshes the summary label with the new route count.
    *
    *   @param Routes    The routes to display. If empty, switches to the empty placeholder.
    */
    void SetRoutes(const QVector<Route>& Routes);


    /*
    *   Clear the cards and show the "no path found" placeholder instead.
    */
    void SetEmpty();


    /*
    *   Clear the cards and show the given message as the placeholder. Used to
    *   report pre-check failures from the pathfinder ("Cannot leave Start Area",
    *   "Destination Unreachable", ...).
    *
    *   @param Message    The user-facing message to display.
    */
    void SetMessage(const QString& Message);


    /*
    *   Programmatically set the GPS starting scene without triggering a route recompute.
    *   Signals on FromCombo are blocked so the change does not fire OnSelectionChanged; the
    *   From-entrance combo is repopulated manually so its choices stay consistent with the
    *   newly selected scene. When EntranceID is provided and matches one of the newly loaded
    *   entrance items, that entrance is also selected — otherwise the combo keeps its default
    *   "Any entrance" value. No-op if the (Game, SceneID) pair is not present in FromCombo.
    *
    *   @param Game          The game the scene belongs to (OOT_GAME or MM_GAME).
    *   @param SceneID       The scene ID to select.
    *   @param EntranceID    Optional specific entrance to preselect (UINT32_MAX = keep "Any").
    */
    void SetFromScene(int Game, uint32_t SceneID, uint32_t EntranceID = UINT32_MAX);

    #pragma endregion


private:

    #pragma region // Helpers

    /*
    *   Build the top bar (tag, From combo, arrow, To combo, swap button, summary).
    */
    void BuildTopBar();


    /*
    *   Build the host widget that contains the row of route cards plus the
    *   empty-state placeholder, attached to the main layout.
    */
    void BuildCardsHost();


    /*
    *   Populate the two scene combos with all OoT and MM scenes that expose a
    *   non-empty name in their SceneMetaInfo entry. Each entry is prefixed by
    *   the game tag ("[OoT]" / "[MM]") so duplicates across games remain distinct.
    *   Scenes that share a display name within a game (e.g. the many "Generic Grotto")
    *   are further disambiguated by their region, then by scene ID as a last resort.
    */
    void PopulateSceneCombos();


    /*
    *   Fill an entrance combo with the entrances physically located in (Game, Scene): a
    *   leading "Any entrance" item (data UINT32_MAX) followed by one item per entrance,
    *   labelled by the scene it connects to and carrying the entrance ID as item data.
    *   Signals are blocked during the refill and the selection is reset to "Any".
    *
    *   @param Combo    The entrance combo to (re)populate.
    *   @param Game     OOT_GAME or MM_GAME.
    *   @param Scene    The scene whose entrances should be listed.
    */
    void PopulateEntranceCombo(QComboBox* Combo, int Game, uint32_t Scene);


    /*
    *   Repopulate both entrance combos from the scenes currently selected in the From / To
    *   scene combos. Used after the scene combos are first filled and after a swap.
    */
    void RefreshEntranceCombos();


    /*
    *   Turn a scene combo into a searchable field: make it editable (search only, no insert)
    *   and attach a case-insensitive "contains" completer so the user can type part of a scene
    *   name to filter the long list instead of scrolling through it. The typed text is snapped
    *   back to the current selection when it does not match any scene.
    *
    *   @param Combo    The combo to make searchable.
    */
    void MakeComboSearchable(QComboBox* Combo);


    /*
    *   Remove every card currently attached to the cards row and delete them.
    */
    void ClearCards();

    #pragma endregion


private slots:

    /*
    *   React to a change in the From / To combos: rebuild the cards via
    *   SampleRoutes(From, To). If From and To are the same scene, switch to the
    *   empty placeholder instead.
    */
    void OnSelectionChanged();


    /*
    *   React to a change of the From scene combo: repopulate the From entrance combo for the
    *   new scene (resetting it to "Any"), then recompute the routes.
    */
    void OnFromSceneChanged();


    /*
    *   React to a change of the To scene combo: repopulate the To entrance combo for the new
    *   scene (resetting it to "Any"), then recompute the routes.
    */
    void OnToSceneChanged();


    /*
    *   Swap the current indexes of the From and To combos.
    */
    void OnSwapClicked();
};
