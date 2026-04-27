#pragma once

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QButtonGroup>
#include <QHash>
#include <QVector>
#include <QString>
#include <QGridLayout>
#include "UI/Settings.h"
#include "UI/OoTMMComboTracker.h"

class OoTMMComboTracker;

/*
*   Modal window exposing the ROM build parameters defined in Settings::ROMSettings.
*   Provides a vertical navigation list on the left and one editor page per parameter
*   group on the right (General, Keys, NPC, Breakable, Special).
*/
class SettingsTab : public QDialog
{
    Q_OBJECT

#pragma region Attributes

private:

    OoTMMComboTracker* WinOwner = nullptr;          // The owning main window, used to apply the settings.

    QListWidget* NavList = nullptr;                 // The left side category list.
    QStackedWidget* Pages = nullptr;                // The right side page stack.

    QButtonGroup* GameGroup = nullptr;              // OoT / MM / OoTMM selector for the General page.
    QButtonGroup* ModeGroup = nullptr;              // Single / Coop / Multi selector for the General page.
    QSpinBox* TeamsSpin = nullptr;                  // Number of multiplayer teams selector.

    QHash<QString, QWidget*> ParamWidgets;          // Map from setting key to the editor widget bound to it.

    QPushButton* ApplyButton = nullptr;             // Apply button at the bottom of the navigation column.
    QPushButton* CancelButton = nullptr;            // Cancel button at the bottom of the navigation column.

#pragma endregion

#pragma region Class creation

public:

    /*
    *   Constructs the settings window for the given owning main window.
    *
    *   @param Owner    The main window that owns the Settings instance to edit.
    *   @param Parent   The Qt parent widget (defaults to Owner).
    */
    explicit SettingsTab(OoTMMComboTracker* Owner, QWidget* Parent = nullptr);

#pragma endregion

#pragma region Pages building

private:

    /*
    *   Build the General page (game / mode / teams selectors).
    *
    *   @return The built General page widget.
    */
    QWidget* BuildGeneralPage();

    /*
    *   Build the Keys & Dungeons page.
    *
    *   @return The built Keys page widget.
    */
    QWidget* BuildKeysPage();

    /*
    *   Build the NPC & Shops page.
    *
    *   @return The built NPC page widget.
    */
    QWidget* BuildNpcPage();

    /*
    *   Build the Breakables page.
    *
    *   @return The built Breakables page widget.
    */
    QWidget* BuildBreakablePage();

    /*
    *   Build the Special page.
    *
    *   @return The built Special page widget.
    */
    QWidget* BuildSpecialPage();

    /*
    *   Build a parameter row composed of a game badge, a label and the matching editor widget,
    *   then add it to the given grid layout at the next available row.
    *
    *   @param Layout      The grid layout to append the row to.
    *   @param Key         The setting key in ROMSettings (used to lookup type / value).
    *   @param GameLabel   The badge text ("OoT", "MM" or empty for shared parameters).
    */
    void AddParamRow(class QGridLayout* Layout, const QString& Key, const QString& GameLabel);

    /*
    *   Build the colored badge label for the given game tag.
    *
    *   @param GameLabel   The game label ("OoT", "MM" or empty).
    *
    *   @return The built badge label, or nullptr if GameLabel is empty.
    */
    QLabel* MakeGameBadge(const QString& GameLabel);

#pragma endregion

#pragma region Apply / Cancel

private:

    /*
    *   Reload all editor widgets values from the owner's current ROMSettings.
    */
    void LoadFromSettings();

    /*
    *   Write the editor widgets values back into the owner's ROMSettings and trigger
    *   the matching OoTMMComboTracker::ApplySettings() refresh.
    */
    void OnApply();

#pragma endregion

};
