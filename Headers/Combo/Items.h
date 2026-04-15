#pragma once

#include "Multi/API.h"
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
} ItemInfo;



void ParseKey(uint8_t Key[5], ComboItem* Item);
const ItemInfo * FindItem(uint32_t gi);
const ItemInfo* FindItemByName(QString Name);