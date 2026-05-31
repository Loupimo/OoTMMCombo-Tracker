#pragma once

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QButtonGroup>
#include <QCheckBox>
#include <QHash>
#include <QVector>
#include <QString>
#include <QPair>
#include <QGroupBox>
#include <QGridLayout>
#include <initializer_list>
#include "UI/Settings.h"
#include "UI/OoTMMComboTracker.h"

class OoTMMComboTracker;

/*
*   Modal window exposing the ROM build parameters defined in Settings::ROMSettings.
*   Provides a vertical navigation list on the left and one editor page per parameter
*   group on the right (General, Keys, NPC, Breakable, Special, Progressive Items,
*   Shared Items, World Items, MQ / JP Layouts).
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

    QHash<QString, QWidget*> ParamWidgets;          // Map from setting key (FilterSettings or ItemSettings) to the editor widget bound to it.

    QHash<QString, QCheckBox*> KeyRingChecksOoT;    // Per-dungeon Ocarina of Time small key ring toggles (World Items page).
    QHash<QString, QCheckBox*> KeyRingChecksMM;     // Per-dungeon Majora's Mask small key ring toggles (World Items page).
    QHash<QString, QCheckBox*> SilverPouchChecks;   // Per-area Ocarina of Time silver rupee pouch toggles (World Items page).
    QHash<QString, QCheckBox*> OwlStatueChecks;     // Per-location Majora's Mask pre-activated owl statue toggles (World Items page).

    QHash<uint32_t, QCheckBox*> MQLayoutChecks;     // Scene id -> Master Quest layout toggle (Layouts page).
    QCheckBox* JPLayoutDekuPalace = nullptr;        // Single Majora's Mask JP Deku Palace layout toggle (Layouts page).

    QPushButton* LoadSpoilerButton = nullptr;       // Spoiler log loader at the bottom of the navigation column.
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
    *   Build the General page (game / mode / teams selectors and Skip Zelda toggle).
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
    *   Build the Progressive Items page (non-shared entries from ItemSettings).
    *
    *   @return The built Progressive Items page widget.
    */
    QWidget* BuildProgressiveItemsPage();

    /*
    *   Build the Shared Items page (every entry from ItemSettings whose key starts with "shared").
    *
    *   @return The built Shared Items page widget.
    */
    QWidget* BuildSharedItemsPage();

    /*
    *   Build the Songs page (every song-related entry from ItemSettings, individual and shared).
    *
    *   @return The built Songs page widget.
    */
    QWidget* BuildSongsPage();

    /*
    *   Build the World Items page (Key Rings, Silver Pouches and Pre-Activated Owl Statues).
    *
    *   @return The built World Items page widget.
    */
    QWidget* BuildWorldItemsPage();

    /*
    *   Build the MQ / JP Layouts page (per scene Master Quest toggles + JP Deku Palace toggle).
    *
    *   @return The built Layouts page widget.
    */
    QWidget* BuildLayoutsPage();

    /*
    *   Build a parameter row composed of a game badge, a label and the matching editor widget,
    *   then add it to the given grid layout at the next available row. The setting is searched
    *   in both Settings::FilterSettings and Settings::ItemSettings.
    *
    *   @param Layout      The grid layout to append the row to.
    *   @param Key         The setting key (used to lookup type / value).
    *   @param GameLabel   The badge text ("OoT", "MM" or empty for shared parameters).
    */
    void AddParamRow(class QGridLayout* Layout, const QString& Key, const QString& GameLabel);

    /*
    *   Build a titled "bubble" group box containing a 3-column grid (badge | label | editor)
    *   populated with the given list of (settingKey, gameLabel) pairs. Centralises the page
    *   layout so every category box has matching paddings, spacings and column stretches.
    *
    *   @param Parent   The widget parent for memory ownership.
    *   @param Title    The group box title.
    *   @param Params   Ordered list of (settingKey, gameLabel) pairs to render as rows.
    *
    *   @return The built QGroupBox, ready to be added to a parent layout.
    */
    QGroupBox* MakeParamGroup(QWidget* Parent, const QString& Title, std::initializer_list<QPair<QString, QString>> Params);

    /*
    *   Build the colored badge label for the given game tag.
    *
    *   @param GameLabel   The game label ("OoT", "MM" or empty).
    *
    *   @return The built badge label, or nullptr if GameLabel is empty.
    */
    QLabel* MakeGameBadge(const QString& GameLabel);

    /*
    *   Locate a parameter by key in either FilterSettings or ItemSettings.
    *
    *   @param Key    The setting key to lookup.
    *
    *   @return Pointer to the matching Parameter, or nullptr if no entry exists.
    */
    Parameter* FindParameter(const QString& Key);

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

    /*
    *   Apply the World Items page selections (Key Rings, Silver Pouches, Owl Statues)
    *   on top of the disabled / starting item id sets after the regular settings pass.
    */
    void ApplyWorldItemSelections();

    /*
    *   Apply the MQ / JP layout selections to the matching scenes ActiveLayout.
    */
    void ApplyLayoutSelections();

    /*
    *   Open a file dialog and forward the chosen spoiler log to OoTMMComboTracker::LoadGameSpoiler,
    *   then reload the dialog widgets to reflect the freshly parsed ROMSettings.
    */
    void OnLoadSpoiler();

#pragma endregion

};
