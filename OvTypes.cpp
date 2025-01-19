#include "OvTypes.h"
#include "Items.h"
#include "Scenes.h"
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
				case OV_SHOP:
					Item->SceneID = GetSceneShop(Item->ObjectID, OOT_GAME);
					break;
				case OV_NPC:
					Item->SceneID = GetSceneNPC(Item->ObjectID, OOT_GAME);
					break;
				case OV_FISH:
					Item->SceneID = FISHING_POND;
					break;
				case OV_SR:
					Item->SceneID = GetSceneSR(Item->ObjectID);
					break;
				case OV_SCRUB:
					Item->SceneID = GetSceneScrub(Item->ObjectID);
					break;
				case OV_GS:
					Item->SceneID = GetSceneGS(Item->ObjectID);
					break;
				default:
					Item->ObjectID = ((uint32_t)Item->OvType - OV_XFLAG0) << 16 | ((uint32_t)Item->RoomID) << 8 | Item->ObjectID;
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
					Item->SceneID = GetSceneNPC(Item->ObjectID, MM_GAME);
					break;
				case OV_SHOP:
					Item->SceneID = GetSceneShop(Item->ObjectID, MM_GAME);
					break;
				default:
					Item->ObjectID = ((uint32_t)Item->OvType - OV_XFLAG0) << 16 | ((uint32_t)Item->RoomID) << 8 | Item->ObjectID;
					break;
			}

		}
	}
}