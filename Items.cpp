#include "Items.h"

void ParseKey(uint8_t Key[5], ComboItem* Item)
{
    uint8_t index = 0;

    Item->GameID = Key[index];
    Item->OvType = Key[++index];
    Item->SceneID = Key[++index];
    Item->RoomID = Key[++index];
    Item->ObjectID = Key[++index];
}

ItemInfo FindItem(uint32_t gi)
{
    ItemInfo currItem;

    for (size_t i = 0; i < NUM_ITEM; i++)
    {
        currItem = ItemList[i];

        if (currItem.ItemID == gi)
        {
            return currItem;
        }
    }

    return currItem;
}