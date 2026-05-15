#pragma once

#include <QFrame>
#include <QLabel>
#include <QVector>
#include "UI/GPSRouteModel.h"


/*
*   Visual card that renders a single GPS Route as a vertical "transit diagram":
*       header (rank / time / label / notes / transitions count) on top,
*       station rows below with a painted vertical line connecting them, plus
*       A / B endpoint circles and numbered intermediate circles.
*
*   The vertical line and the circles are drawn in paintEvent() so they stay
*   centered on the station name labels regardless of font / layout reflow.
*/
class GPSRouteCard : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool best READ IsBest WRITE SetBest)

public:

    #pragma region // Attributes

    Route                   RouteData;
    QVector<QLabel*>        StationLabels;          // The station-name QLabels, one per RouteStep. Used at paint time to locate the Y of each circle.
    QWidget*                Diagram = nullptr;      // The container of the station rows. Its top-left is the origin used to position the vertical line.
    bool                    Best = false;

    #pragma endregion


    #pragma region // Class creation

    /*
    *   Construct the card with the given route data and parent.
    *
    *   @param Data      The route data to display.
    *   @param Parent    The Qt parent widget.
    */
    explicit GPSRouteCard(const Route& Data, QWidget* Parent = nullptr);

    #pragma endregion


    #pragma region // Methods

    /*
    *   Return the current "best" flag of this card, used by the QSS selector
    *   GPSRouteCard[best="true"] to apply the highlighted border.
    *
    *   @return True if this card is the recommended route.
    */
    bool IsBest() const { return this->Best; }


    /*
    *   Set the "best" flag and re-polish the widget so the QSS selector reacts
    *   to the new property value.
    *
    *   @param Value    The new best flag.
    */
    void SetBest(bool Value);

    #pragma endregion


protected:

    #pragma region // Events

    /*
    *   Paint the vertical line connecting all stations and the start / end /
    *   intermediate circles, anchored on the centers of the station-name labels.
    *
    *   @param Event    The Qt paint event (unused).
    */
    void paintEvent(QPaintEvent* Event) override;


    /*
    *   Trigger a repaint when the card is resized so the vertical line and the
    *   circles stay aligned with the labels after layout reflow.
    *
    *   @param Event    The Qt resize event (unused).
    */
    void resizeEvent(QResizeEvent* Event) override;

    #pragma endregion


private:

    #pragma region // Helpers

    /*
    *   Build and attach the header block (rank label, "MEILLEURE" pill on best
    *   cards, big time + label, notes, right-aligned transitions count).
    *
    *   @param ParentLayout    The parent QVBoxLayout to append the header to.
    */
    void BuildHeader(class QVBoxLayout* ParentLayout);


    /*
    *   Build and attach the diagram block: one station row per RouteStep, each
    *   row containing the station name and the "via ..." caption (omitted on the
    *   last station).
    *
    *   @param ParentLayout    The parent QVBoxLayout to append the diagram to.
    */
    void BuildDiagram(class QVBoxLayout* ParentLayout);

    #pragma endregion
};
