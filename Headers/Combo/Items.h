#pragma once

#include "Multi/API.h"
#include "Combo/Objects.h"
#include <QString>

typedef struct ComboItem
{
	uint8_t GameID;
	uint8_t OvType;
	uint32_t SceneID;
	uint32_t RoomID;
	uint32_t ObjectID;
} ComboItem;


/*
*   Render reference for an item icon. An item can either point to a specific
*   icon (entry of SpecificIconsMetaInfo via ObjectIconMap) or to a common one
*   (entry of IconsMetaInfo via ObjectType). Implicit constructors let item
*   tables keep the natural { ..., ObjectIconMap::xxx } / { ..., ObjectType::xxx }
*   initializer syntax without forcing callers to choose a tag manually.
*/
typedef struct ItemRenderIcon
{
    bool UseSpecific;               // True = use SpecificIcon (ObjectIconMap); false = use CommonIcon (ObjectType).
    ObjectIconMap SpecificIcon;     // The specific icon to render, indexed in SpecificIconsMetaInfo.
    ObjectType CommonIcon;          // The common icon to render, indexed in IconsMetaInfo.

    constexpr ItemRenderIcon() : UseSpecific(false), SpecificIcon(ObjectIconMap::type), CommonIcon(ObjectType::none) {}
    constexpr ItemRenderIcon(ObjectIconMap Icon) : UseSpecific(true), SpecificIcon(Icon), CommonIcon(ObjectType::none) {}
    constexpr ItemRenderIcon(ObjectType Icon) : UseSpecific(false), SpecificIcon(ObjectIconMap::type), CommonIcon(Icon) {}
} ItemRenderIcon;


typedef struct ItemInfo
{
	uint32_t ItemID;
	const char* ItemName;
    ItemRenderIcon RenderType;
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
