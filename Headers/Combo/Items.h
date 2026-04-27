#pragma once

#include "Multi/API.h"
#include "Combo/Objects.h"
#include "UI/Icons.h"
#include <QString>

typedef struct ComboItem
{
	uint8_t GameID;
	uint8_t OvType;
	uint32_t SceneID;
	uint32_t RoomID;
	uint32_t ObjectID;
} ComboItem;


typedef struct ItemInfo
{
	uint32_t ItemID;
	const char* ItemName;
    EGameIcon RenderType;
} ItemInfo;



/*
*   Decode a raw 5 byte combo key into its matching ComboItem fields.
*
*   @param Key      The raw 5 byte key (Game, OvType, Scene, Room, Object).
*   @param Item     The combo item to populate with the decoded values.
*/
void ParseKey(uint8_t Key[5], ComboItem* Item);

/*
*   Find the item information matching the given item ID.
*
*   @param gi       The item ID to search for.
*
*   @return The item information matching the given ID, or a null pointer if none matched.
*/
const ItemInfo * FindItem(uint32_t gi);

/*
*   Find the item information matching the given item name.
*
*   @param Name     The item name to search for (newlines are stripped before matching).
*
*   @return The item information matching the given name, or a null pointer if none matched.
*/
const ItemInfo* FindItemByName(QString Name);
