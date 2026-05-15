#include "UI/GPSRouteModel.h"


#pragma region // Helpers

const char* ViaLabel(TransitionVia Via)
{
    switch (Via)
    {
        case Via_Walk: return "Walk";
        case Via_Owl:  return "Owl";
        case Via_Song: return "Song";
        case Via_Warp: return "Warp";
    }
    return "?";
}


QChar ViaGlyph(TransitionVia Via)
{
    switch (Via)
    {
        case Via_Walk: return QChar(0x2192);    // ->
        case Via_Owl:  return QChar(0x2303);    // up arrow
        case Via_Song: return QChar(0x266A);    // music note
        case Via_Warp: return QChar(0x2726);    // four-pointed star
    }
    return QChar('?');
}


QVector<Route> SampleRoutes(const QString& From, const QString& To)
{
    QVector<Route> Routes;

    // r1 - Rapide (best route)
    {
        Route R;
        R.Rank = "ROUTE 1";
        R.TotalTime = "1:25";
        R.Label = "rapide";
        R.Notes = "2 transitions · trajet direct";
        R.Accent = QColor("#3ddc84");
        R.IsBest = true;
        R.Steps.append({ From,           Via_Walk, 40 });
        R.Steps.append({ "Hyrule Field", Via_Walk, 45 });
        R.Steps.append({ To,             Via_Walk,  0 });
        Routes.append(R);
    }

    // r2 - Equilibree
    {
        Route R;
        R.Rank = "ROUTE 2";
        R.TotalTime = "2:10";
        R.Label = "équilibrée";
        R.Notes = "3 transitions · détour modeste";
        R.Accent = QColor("#e8a82d");
        R.IsBest = false;
        R.Steps.append({ From,             Via_Walk, 35 });
        R.Steps.append({ "Market",         Via_Walk, 50 });
        R.Steps.append({ "Tektite Grotto", Via_Owl,  45 });
        R.Steps.append({ To,               Via_Walk,  0 });
        Routes.append(R);
    }

    // r3 - Detour
    {
        Route R;
        R.Rank = "ROUTE 3";
        R.TotalTime = "3:50";
        R.Label = "détour";
        R.Notes = "4 transitions · panoramique";
        R.Accent = QColor("#ff5252");
        R.IsBest = false;
        R.Steps.append({ From,           Via_Walk, 60 });
        R.Steps.append({ "Zora River",   Via_Walk, 45 });
        R.Steps.append({ "Hyrule Field", Via_Song, 80 });
        R.Steps.append({ "Open Grotto",  Via_Walk, 45 });
        R.Steps.append({ To,             Via_Walk,  0 });
        Routes.append(R);
    }

    return Routes;
}

#pragma endregion
