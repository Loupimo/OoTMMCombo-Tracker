#include "OvTypes.h"
#include "Items.h"
#include "game.h"

void CorrectComboItem(ComboItem* Item)
{
	if (Item)
	{
		if (Item->GameID == OOT_GAME)
		{
			switch (Item->OvType)
			{
				case OV_SF:
				case OV_NONE:
					break;
				case OV_CHEST:
				case OV_COLLECTIBLE:
				case OV_COW:
					//Item->ObjectID >>= 3;
					break;
				case OV_NPC:
				case OV_SHOP:
				case OV_SCRUB:
				case OV_SR:
				case OV_FISH:
					//BITMAP8_SET(gSharedCustomSave.oot.npc, id);
					break;
				case OV_GS:
					//BITMAP32_SET(gOotSave.info.gsFlags, id - 8);
					break;
				default:

					Item->ObjectID = ((uint32_t)Item->OvType - OV_XFLAG0) << 16 | ((uint32_t)Item->RoomID) << 8 | Item->ObjectID;
					//setXflagsMarkOot(play, ovType - OV_XFLAG0, sceneId, roomId, id);
					break;
			}
		}
		else
		{	// Majora's mask
			switch (Item->OvType)
			{
				case OV_SCRUB:
				case OV_SR:
				case OV_FISH:
				case OV_GS:
				case OV_NONE:
					break;
				case OV_CHEST:
				case OV_COLLECTIBLE:
					break;
				case OV_SF:
					//setStrayFairyMarkMm(play, sceneId, id);
					break;
				case OV_COW:
					break;
				case OV_NPC:
				case OV_SHOP:
					//BITMAP8_SET(gSharedCustomSave.mm.shops, id);
					break;
				default:

					Item->ObjectID = ((uint32_t)Item->OvType - OV_XFLAG0) << 16 | ((uint32_t)Item->RoomID) << 8 | Item->ObjectID;
					//setXflagsMarkMm(play, ovType - OV_XFLAG0, sceneId, roomId, id);
					break;
			}

		}
	}
}