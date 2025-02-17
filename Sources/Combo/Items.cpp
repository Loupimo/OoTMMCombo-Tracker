#include "Combo/Items.h"

void ParseKey(uint8_t Key[5], ComboItem* Item)
{
    uint8_t index = 0;

    Item->GameID = Key[index];
    Item->OvType = Key[++index];
    Item->SceneID = Key[++index];
    Item->RoomID = Key[++index];
    Item->ObjectID = Key[++index];
}

const ItemInfo * FindItem(uint32_t gi)
{
    const ItemInfo * currItem = nullptr;

    for (size_t i = 0; i < NUM_ITEM; i++)
    {
        currItem = &ItemList[i];

        if (currItem->ItemID == gi)
        {
            return currItem;
        }
    }

    return currItem;
}

const ItemInfo* FindItemByName(QString Name)
{
    Name = Name.replace("\n", "");
    size_t len = Name.length() + 1;
    char* tmpObjName = (char*)malloc(sizeof(char) * len);
    if (tmpObjName)
    {
        memcpy_s(tmpObjName, len, Name.toStdString().c_str(), len);
        tmpObjName[len - 1] = '\0';

        for (size_t i = 0; i < NUM_ITEM; i++)
        {
            if (strcmp(tmpObjName, ItemList[i].ItemName) == 0)
            {
                free(tmpObjName);
                return &ItemList[i];
            }
        }

        // No item found

        ItemInfo* item = new ItemInfo();
        item->ItemID = -1;
        item->ItemName = tmpObjName;

        return item;

    }
    return nullptr;
}