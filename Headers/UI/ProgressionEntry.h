#pragma once

#include "UI/Icons.h"

/*
*   Declarative description of a single progression entry.
*   The Icon field both selects the displayed pixmap (via IconsMetaInfo) and
*   serves as the primary lookup key when an item is collected (Item->RenderType).
*   When several entries share the same Icon (e.g. ocarina songs), the LookupKey
*   acts as a disambiguator: it is matched against the normalized item name.
*   When an item's RenderType cannot be matched in the icon hash (typically
*   RenderType=none), the LookupKey is also used as a fallback name search.
*/
typedef struct ProgEntry
{
    EGameIcon Icon;             // Display + primary lookup key.
    const char* DisplayName;    // Label shown under the icon.
    //const char* LookupKey;      // Lowercase substring matched against the normalized item name.
    QSet<uint32_t> LookupKeys;
    bool IsCounter;             // Whether the widget shows a counter badge.
} ProgEntry;


/*
*   Declarative description of a section (group of entries) inside a page.
*/
typedef struct ProgSection
{
    const char* Title;          // Section header text.
    const ProgEntry* Entries;   // Pointer to the static entries array.
    size_t Count;               // Number of entries in the array.
} ProgSection;

#pragma region OoT Page

const size_t NumOoTSections = 4;
extern const ProgSection OoTSections[NumOoTSections];

#pragma endregion OoT Page

#pragma region MM Page

const size_t NumMMSections = 5;
extern const ProgSection MMSections[NumMMSections];

#pragma endregion MM Page

#pragma region Souls

const size_t NumSoulsSections = 8;
extern const ProgSection SoulsSections[NumSoulsSections];

#pragma endregion Souls

#pragma region Collectibles

const size_t NumCollectiblesSections = 5;
extern const ProgSection CollectiblesSections[NumCollectiblesSections];

#pragma endregion Collectibles

