#include "Combo/Xflags.h"
#include "Combo/Items.h"
#include "Combo/OvTypes.h"

#pragma region Attributes

/* True when the loaded ROM predates the compact-XflagID rework (stable <= v32.3). */
static bool UsesLegacyXflagsFlag = false;

#pragma endregion

#pragma region Xflag resolution

void SetUsesLegacyXflags(bool Legacy)
{
    UsesLegacyXflagsFlag = Legacy;
}

bool UsesLegacyXflags(void)
{
    return UsesLegacyXflagsFlag;
}

bool ResolveXflagItem(ComboItem* Item)
{
    if (Item == nullptr)
    {
        return false;
    }

    if (UsesLegacyXflagsFlag)
    {   // Legacy ROMs (<= v32.3) already carry the full scene / room / actor inside the key.

        return false;
    }

    if (Item->OvType < OV_XFLAG0)
    {   // Not an xflag; nothing to tag.

        return false;
    }

    /* In the compact system the query only holds the XflagID, split across room (high) and id (low).
       Tagging it lets FindObject resolve the object directly (FindObjectByXflagID). */
    Item->XflagID = (uint16_t)(((uint32_t)Item->RoomID << 8) | (uint32_t)(Item->ObjectID & 0xFF));

    return true;
}

#pragma endregion
