#pragma once

#include <cstdint>

struct ComboItem;

#pragma region Xflag resolution

/*
*   Record whether the loaded ROM predates the compact-XflagID rework (<= v32.3). Published by the
*   spoiler loader so the item pipeline (which has no Settings handle) can branch. Dev and unknown
*   builds are treated as current (new system).
*
*   @param Legacy   True for stable builds <= v32.3, false otherwise.
*/
void SetUsesLegacyXflags(bool Legacy);

/*
*   Tell whether the loaded ROM uses the legacy xflag system (full identity inside the key).
*
*   @return True when the legacy system is active, false for the compact-XflagID system.
*/
bool UsesLegacyXflags(void);

/*
*   Tag a ComboItem that carries a compact XflagID (new xflag ROMs) with that id, so a later
*   FindObject resolves the object directly (see FindObjectByXflagID). Legacy ROMs (<= v32.3) keep
*   the full identity in the key and need no tagging, so this is a no-op for them, for non-xflag
*   items, and it leaves Item->XflagID at 0xFFFF in those cases.
*
*   On entry the item is expected as decoded from the override key: OvType == OV_XFLAG (0x10),
*   RoomID == (XflagID >> 8), ObjectID == (XflagID & 0xff), SceneID == 0.
*
*   @param Item     The combo item to tag in place.
*
*   @return True if the item was an xflag that got tagged, false otherwise.
*/
bool ResolveXflagItem(ComboItem* Item);

#pragma endregion
