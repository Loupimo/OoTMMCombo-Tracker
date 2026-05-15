#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
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

    QLabel*                 Tag = nullptr;
    QComboBox*              FromCombo = nullptr;
    QLabel*                 ArrowLabel = nullptr;
    QComboBox*              ToCombo = nullptr;
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
    */
    void PopulateSceneCombos();


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
    *   Swap the current indexes of the From and To combos.
    */
    void OnSwapClicked();
};
