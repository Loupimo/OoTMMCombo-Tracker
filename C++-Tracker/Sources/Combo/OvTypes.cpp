#include "Combo/OvTypes.h"
#include "Combo/Items.h"
#include "Combo/Scenes.h"
#include "Multi/Game.h"

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
					break;
				case OV_COW:
					Item->SceneID = GetSceneCow(Item->ObjectID, OOT_GAME);
					break;
				case OV_SHOP:
					Item->SceneID = GetSceneShop(Item->ObjectID, OOT_GAME);
					break;
				case OV_NPC:
					// On a stable ROM the GS reward NPCs are reported one ID higher; translate to
					// the tracker's internal numbering so both the scene lookup and FindObject match.
					Item->ObjectID = ResolveRawOoTNpcID(Item->ObjectID);
					Item->SceneID = GetSceneNPC(Item->ObjectID, OOT_GAME);
					break;
				case OV_FISH:
					Item->SceneID = OOT_FISHING_POND;
					break;
				case OV_SR:
					Item->SceneID = GetSceneSR(Item->ObjectID);
					break;
				case OV_SCRUB:
					Item->SceneID = GetSceneScrub(Item->ObjectID);
					break;
				case OV_GS:
					// The game reports the raw GS flag, but the pool (and GetSceneGS)
					// key every GS on flag+8, so shift before resolving the scene / object.
					// Otherwise flag F marks the GS stamped with object_id F (8 slots too
					// early), e.g. Zora River GS Tree (flag 0x89) -> Kakariko GS Bazaar.
					Item->ObjectID += 8;
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
					Item->SceneID = GetSceneCow(Item->ObjectID, MM_GAME);
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