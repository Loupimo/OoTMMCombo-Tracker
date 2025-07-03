#include "Combo/Scenes.h"
#include "Combo/NPC.h"
#include "Combo/Objects.h"
#include "Multi/Game.h"

const SceneMetaInfo* GetSceneMetaInfo(uint32_t SceneID, uint32_t Game)
{
	if (Game == OOT_GAME)
	{
		return &OoTScenesMetaInfo[SceneID];
	}
	else
	{
		return &MMScenesMetaInfo[SceneID];
	}
}


uint32_t GetSceneCow(uint32_t ItemID, uint32_t Game)
{
	if (Game == OOT_GAME)
	{
		switch (ItemID)
		{
			case 0x00:
				return KOKIRI_FOREST;

			case 0x01:
				return HYRULE_FIELD;

			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
				return LON_LON_RANCH;

			case 0x06:
				return KAKARIKO_VILLAGE;

			case 0x07:
				return DEATH_MOUNTAIN_TRAIL;

			case 0x08:
				return GERUDO_VALLEY;

			case 0x09:
				return INSIDE_JABU_JABU;

			default:
				return ItemID;
		}
	}
	else
	{
		switch (ItemID)
		{
			case 0x10:
			case 0x11:
			case 0x12:
				return ROMANI_RANCH;

			case 0x13:
			case 0x14:
				return MM_GROTTOS;

			case 0x15:
			case 0x16:
				return GREAT_BAY_COAST;

			case 0x17:
				return BENEATH_THE_WELL;

			default:
				return ItemID;
		}
	}
}


uint32_t GetSceneShop(uint32_t ItemID, uint32_t Game)
{
	uint32_t ShopID = ItemID >> 3;

	if (Game == OOT_GAME)
	{
		switch (ShopID)
		{
			case OoT_Kokiri_Shop:
				return KOKIRI_SHOP;

			case OoT_Bombchu_Shop:
				return BOMBCHU_SHOP;

			case OoT_Zora_Shop:
				return ZORA_SHOP;

			case OoT_Goron_Shop:
				return OOT_GORON_SHOP;

			case OoT_Kakariko_Bazaar:
			case OoT_Market_Bazaar:
				return BAZAAR;

			case OoT_Market_Potion_Shop:
				return MARKET_POTION_SHOP;

			case OoT_Kakariko_Potion_Shop:
				return KAKARIKO_POTION_SHOP;

			default:
				return ShopID;
		}
	}
	else
	{
		if (ItemID <= 0x03)
		{	// Bomb shop
			return BOMB_SHOP;
		}
		else if (ItemID == 0x04)
		{	// Curiosity shop
			return CURIOSITY_SHOP;
		}
		else if (ItemID <= 0x0C)
		{	// Trading post shop
			return TRADING_POST;
		}
		else if (ItemID <= 0x0f)
		{
			return POTION_SHOP;
		}
		else if (ItemID <= 0x12)
		{
			return MM_GORON_SHOP;
		}
		else
		{	// Zora shop
			return ZORA_HALL_ROOMS;
		}
	}
}


uint32_t GetSceneNPC(uint32_t NPC, uint32_t Game)
{
	if (Game == OOT_GAME)
	{
		switch (NPC)
		{
			// Grottos
			case WEIRD_EGG:
			case POCKET_EGG:
			case THEATER_STICKS:
			case THEATER_NUTS:
				return OOT_GROTTOS;

			// Hyrule Castle
			case MALON_EGG:
				return HYRULE_CASTLE;

			// Castle Courtyard
			case ZELDA_LETTER:
			case ZELDA_SONG:
				return CASTLE_COURTYARD;

			// Lost Woods
			case SARIA_OCARINA:
			case LOST_WOODS_TARGET:
			case LOST_WOODS_SKULL_KID:
			case LOST_WOODS_MEMORY:
			case MASK_SELL_SKULL:
			case TRADE_ODD_MUSHROOM:
			case TRADE_POACHER_SAW:
				return OOT_LOST_WOODS;

			// Sacred Forest Meadow
			case SARIA_SONG:
			case SHEIK_FOREST:
				return SACRED_FOREST_MEADOW;

			// Kakariko Village
			case ANJU_BOTTLE:
			case TRADE_POCKET_EGG:
			case TRADE_COJIRO:
			case SHEIK_SHADOW:
			case KAKARIKO_ROOF_MAN:
			case MASK_SELL_KEATON:
				return KAKARIKO_VILLAGE;

			// Kakariko Potion Shop
			case TRADE_ODD_POTION:
				return KAKARIKO_POTION_SHOP;

			// Graveyard
			case MASK_SELL_SPOOKY:
				return GRAVEYARD;

			// Tomb Royal
			case ROYAL_TOMB_SONG:
				return TOMB_ROYAL;

			// Death Mountain Trail
			case TRADE_PRESCRIPTION:
			case TRADE_CLAIM_CHECK:
			case TRADE_BIGGORON_SWORD:
				return DEATH_MOUNTAIN_TRAIL;

			// Goron City
			case DARUNIA_BRACELET:
			case GORON_LINK_TUNIC:
			case GORON_BOMB_BAG:
			case MEDIGORON:
				return GORON_CITY;

			// Death Mountain Crater
			case SHEIK_FIRE:
				return DEATH_MOUNTAIN_CRATER;

			// Zora River
			case BEAN_SELLER:
			case FROGS_STORMS:
			case FROGS_GAME:
				return ZORA_RIVER;

			// Zora Domain
			case ZORA_DIVING_GAME:
			case ZORA_KING_TUNIC:
			case TRADE_EYEBALL_FROG:
				return ZORA_DOMAIN;

			// Lake Hylia
			case RUTO_LETTER:
			case FIRE_ARROW:
				return LAKE_HYLIA;

			// Laboratory
			case LABORATORY_DIVE:
			case TRADE_EYE_DROPS:
				return OOT_LABORATORY;

			// Fishing Pond
			case FISH_CHILD:
			case FISH_ADULT:
				return FISHING_POND;

			// Lon Lon Ranch
			case MALON_SONG:
				return LON_LON_RANCH;

			// Lon Lon Ranch Silo
			case TALON_BOTTLE:
				return RANCH_HOUSE_SILO;

			// Hyrule Field
			case OCARINA_TIME_ITEM:
			case OCARINA_TIME_SONG:
			case MASK_SELL_BUNNY:
				return HYRULE_FIELD;

			// Back Alley
			case DOG_LADY:
				return BACK_ALLEY_HOUSE2;

			// Guard House
			case POE_COLLECTOR:
				return GUARD_HOUSE;

			// Bombchu Bowling
			case BOMBCHU_BOWLING_1:
			case BOMBCHU_BOWLING_2:
				return BOMBCHU_BOWLING_ALLEY;

			// Treasure Shop
			case CHEST_GAME_KEY:
				return OOT_TREASURE_SHOP;

			// Shooting Gallery
			case SHOOTING_GAME_CHILD:
			case SHOOTING_GAME_ADULT:
				return OOT_SHOOTING_GALLERY;

			// Temple of Time
			case ZELDA_LIGHT_ARROW:
			case SHEIK_LIGHT:
			case MASTER_SWORD:
			case LIGHT_MEDALLION:
				return TEMPLE_OF_TIME;

			// Tomb Dampe Windmill
			case OOT_SONG_STORMS:
				return TOMB_DAMPE_WINDMILL;

			// Gerudo Valley
			case TRADE_BROKEN_GORON_SWORD:
				return GERUDO_VALLEY;

			// Gerudo Forteress
			case GERUDO_ARCHERY_1:
			case GERUDO_ARCHERY_2:
				return GERUDO_FORTRESS;

			// Desert Colossus
			case SHEIK_SPIRIT:
				return DESERT_COLOSSUS;

			// House of Skulltulla
			case GS_10:
			case GS_20:
			case GS_30:
			case GS_40:
			case GS_50:
				return HOUSE_OF_SKULLTULA;

			// Great Fairy Upgrades
			case FAIRY_MAGIC_UPGRADE:
			case FAIRY_MAGIC_UPGRADE2:
			case FAIRY_DEFENSE_UPGRADE:
				return GREAT_FAIRY_FOUNTAIN_UPGRADES;

			// Great Fairy Spells
			case FAIRY_SPELL_FIRE:
			case FAIRY_SPELL_WIND:
			case FAIRY_SPELL_LOVE:
				return GREAT_FAIRY_FOUNTAIN_SPELLS;

			// Lair Gohma
			case BLUE_WARP_GOHMA:
				return LAIR_GOHMA;

			// Lair King Dodongo
			case BLUE_WARP_KING_DODONGO:
				return LAIR_KING_DODONGO;

			// Lair Barinade
			case BLUE_WARP_BARINADE:
				return LAIR_BARINADE;

			// Lair Phantom Ganon
			case BLUE_WARP_PHANTOM_GANON:
				return LAIR_PHANTOM_GANON;

			// Lair Volvagia
			case BLUE_WARP_VOLVAGIA:
				return LAIR_VOLVAGIA;

			// Lair Morpha
			case BLUE_WARP_MORPHA:
				return LAIR_MORPHA;

			// Lair Bongo Bongo
			case BLUE_WARP_BONGO_BONGO:
				return LAIR_BONGO_BONGO;

			// Lair Twinrova
			case BLUE_WARP_TWINROVA:
				return LAIR_TWINROVA;

			// Thieves Hideout
			case GERUDO_CARD:
				return THIEVES_HIDEOUT;

			// Ice Cavern
			case SHEIK_WATER:
				return ICE_CAVERN;

			default:
				return NPC;
		}
	}
	else
	{	// Majora's Mask

		switch (NPC)
		{
			// Clock Town South
			case OWL_CLOCK_TOWN:
			case POSTBOX:
			case SCRUB_LAND:
			case TINGLE_MAP_CLOCK_TOWN:
			case TINGLE_MAP_WOODFALL:
			case TINGLE_MAP_SNOWHEAD:
			case TINGLE_MAP_ROMANI_RANCH:
			case TINGLE_MAP_GREAT_BAY:
			case TINGLE_MAP_STONE_TOWER:
			case SONG_HEALING:
				return CLOCK_TOWN_SOUTH;

			// Clock Town North
			case KEATON_HEART_PIECE:
			case MASK_BLAST:
			case BOMBER_NOTEBOOK:
				return CLOCK_TOWN_NORTH;

			// Clock Town East
			case MASK_POSTMAN:
				return CLOCK_TOWN_EAST;

			// Clock Town West
			case ROSA_HEART_PIECE:
			case BANK_1:
			case BANK_2:
			case BANK_3:
				return CLOCK_TOWN_WEST;

			// Laundry Pool
			case STRAY_FAIRY_TOWN:
			case MASK_BREMEN:
				return LAUNDRY_POOL;

			// Fairy Fountain
			case GREAT_FAIRY_TOWN:
			case GREAT_FAIRY_TOWN_ALT:
			case GREAT_FAIRY_SWAMP:
			case GREAT_FAIRY_MOUNTAIN:
			case GREAT_FAIRY_OCEAN:
			case GREAT_FAIRY_VALLEY:
				return MM_FAIRY_FOUNTAIN;

			// Clock Town Tower
			case SKULL_KID_OCARINA:
			case SKULL_KID_SONG:
				return CLOCK_TOWER_ROOFTOP;

			// Post Office
			case POSTMAN_HEART_PIECE:
				return POST_OFFICE;

			// Mayor House
			case MAYOR_HEART_PIECE:
			case MASK_KAFEI:
				return MAYOR_HOUSE;

			// Milk Bar
			case MASK_TROUPE_LEADER:
			case MILK_BAR_MILK:
			case MILK_BAR_CHATEAU:
			case CHATEAU_ROMANI:
				return MILK_BAR;

			// Town Shooting gallery
			case SHOOTING_GAME_TOWN_1:
			case SHOOTING_GAME_TOWN_2:
				return MM_SHOOTING_GALLERY;

			// Treasure Shop
			case CHEST_GAME:
				return MM_TREASURE_SHOP;

			// Honey & Darling
			case HONEY_DARLING_1:
			case HONEY_DARLING_2:
				return HONEY_DARLING;

			// Stock Pot Inn
			case TOILET_HEART_PIECE:
			case GRANDMA_HEART_PIECE_1:
			case GRANDMA_HEART_PIECE_2:
			case ROOM_KEY:
			case LETTER_TO_KAFEI:
			case MASK_COUPLE:
				return STOCK_POT_INN;

			// Curiosity shop
			case PENDANT_OF_MEMORIES:
			case MASK_KEATON:
			case LETTER_TO_MAMA:
				return CURIOSITY_SHOP;

			// Observatory
			case MOON_TEAR:
				return OBSERVATORY;

			// Swordsman school
			case SWORDSMAN_HEART_PIECE:
				return SWORDSMAN_SCHOOL;

			// Deku Playground
			case DEKU_PLAYGROUND_1:
			case DEKU_PLAYGROUND_2:
				return DEKU_PLAYGROUND;

			// Termina Field
			case MASK_KAMARO:
				return TERMINA_FIELD;

			// Grottos
			case SCRUB_TELESCOPE:
			case GOSSIP_HEART_PIECE:
				return MM_GROTTOS;

			// Southern Swamp
			case SCRUB_SHOP_BEANS:
			case OWL_SOUTHERN_SWAMP:
			case SONG_SOARING:
			case SCRUB_SWAMP:
				return SOUTHERN_SWAMP;

			// Shooting gallery Swamp
			case SHOOTING_GAME_SWAMP_1:
			case SHOOTING_GAME_SWAMP_2:
				return SHOOTING_GALLERY_SWAMP;

			// Tourist information
			case KOUME_PICTOGRAPH_BOX:
			case KOUME_HEART_PIECE:
			case TOUR_GUIDE_HEART_PIECE:
				return TOURIST_INFORMATION;

			// Woods of Mystery
			case KOTAKE_RED_POTION:
				return WOODS_MYSTERY;

			// Deku Palace
			case SONG_AWAKENING:
				return DEKU_KING_CHAMBER;

			// Deku Shrine
			case MASK_SCENTS:
				return DEKU_SHRINE;

			// Woodfall
			case OWL_WOODFALL:
				return WOODFALL;

			// Spider House Swamp
			case SPIDER_HOUSE_SWAMP:
				return MM_SPIDER_HOUSE_SWAMP;

			// Mountain Village Winter
			case MASK_DON_GERO:
			case FROG_HEART_PIECE:
			case OWL_MOUNTAIN_VILLAGE:
				return MOUNTAIN_VILLAGE_WINTER;

			// Blacksmith
			case SWORD_RAZOR:
			case SWORD_GILDED:
				return BLACKSMITH;

			// Goron Graveyard
			case MASK_GORON:
				return GORON_GRAVEYARD;

			// Twin Islands Winter
			case GORON_ELDER:
				return TWIN_ISLANDS_WINTER;

			// Goron Shrine
			case GORON_BABY:
				return GORON_SHRINE;

			// Goron Village Winter
			case SCRUB_MOUNTAIN:
			case SCRUB_BOMB_BAG:
			case GORON_KEG:
				return GORON_VILLAGE_WINTER;

			// Snowhead
			case OWL_SNOWHEAD:
				return SNOWHEAD;

			// Goron Racetrack
			case GORON_RACE:
				return GORON_RACETRACK;

			// Milk Road
			case OWL_MILK_ROAD:
				return MILK_ROAD;

			// Romani Ranch
			case SONG_EPONA:
			case ROMANI_BOTTLE:
			case CREMIA_ESCORT:
				return ROMANI_RANCH;

			// Cucco Shack
			case MASK_BUNNY:
				return CUCCO_SHACK;

			// Dog Lady Racetrack
			case DOG_LADY_HEART_PIECE:
				return DOG_RACETRACK;

			// Great Bay Coast
			case MASK_ZORA:
			case FISHERMAN_HEART_PIECE:
			case OWL_GREAT_BAY:
				return GREAT_BAY_COAST;

			// Zora Cape
			case OWL_ZORA_CAPE:
				return ZORA_CAPE;

			// Zora Hall Rooms
			case SCRUB_OCEAN:
			case SCRUB_SHOP_POTION_GREEN:
			case ZORA_EVAN:
				return ZORA_HALL_ROOMS;

			// Zora Hall
			case ZORA_HALL_LIGHTS:
				return ZORA_HALL;

			// Pinnacle Rock
			case SEAHORSE_HEART_PIECE:
				return PINNACLE_ROCK;

			// Laboratory
			case SONG_ZORA:
			case LAB_FISH_HEART_PIECE:
				return MM_LABORATORY;

			// Spider House Ocean
			case SPIDER_HOUSE_OCEAN:
				return MM_SPIDER_HOUSE_OCEAN;

			// Waterfall Rapids
			case BEAVER_1:
			case BEAVER_2:
				return WATERFALL_RAPIDS;

			// Road Ikana
			case MASK_STONE:
				return ROAD_IKANA;

			// Gorman Track
			case GORMAN_MILK:
			case MASK_GARO:
				return GORMAN_TRACK;

			// Beneath the Graveyard
			case MM_SONG_STORMS:
				return BENEATH_THE_GRAVEYARD;

			// Ikana Canyon
			case OWL_IKANA_CANYON:
			case SCRUB_VALLEY:
			case SCRUB_SHOP_POTION_BLUE:
				return IKANA_CANYON;

			// Music Box House
			case MASK_GIBDO:
				return MUSIC_BOX_HOUSE;

			// Ghost Hut
			case GHOST_HUT_HEART_PIECE:
				return GHOST_HUT;

			// Stone Tower
			case OWL_STONE_TOWER:
				return STONE_TOWER;

			// Lair Odolwa
			case SONG_ORDER:
			case REMAINS_ODOLWA:
				return LAIR_ODOLWA;

			// Lair Goht
			case REMAINS_GOHT:
				return LAIR_GOHT;

			// Lair Gyorg
			case REMAINS_GYORG:
				return LAIR_GYORG;

			// Lair Ikana
			case SONG_EMPTINESS:
				return LAIR_IKANA;

			// Lair Twinmold
			case REMAINS_TWINMOLD:
				return LAIR_TWINMOLD;

			// Moon
			case MASK_FIERCE_DEITY:
			case MAJORA:
				return MOON;

			default:
				return NPC;
		}
	}
}


uint32_t GetSceneSR(uint32_t SilverRupee)
{
	if (SilverRupee >= 0x00 && SilverRupee <= 0x04)
	{	// MQ Dodongo Cavern

		return DODONGO_CAVERN;
	}
	else if (SilverRupee >= 0x05 && SilverRupee <= 0x09)
	{	// Bottom of the Well

		return BOTTOM_OF_THE_WELL;
	}
	else if (SilverRupee >= 0x0a && SilverRupee <= 0x18)
	{	// Spirit Temple

		return TEMPLE_SPIRIT;
	}
	else if (SilverRupee >= 0x19 && SilverRupee <= 0x36)
	{	// Shadow Temple

		return TEMPLE_SHADOW;
	}
	else if (SilverRupee >= 0x37 && SilverRupee <= 0x40)
	{	// Ice Cavern

		return ICE_CAVERN;
	}
	else if (SilverRupee >= 0x41 && SilverRupee <= 0x50)
	{	// Gerudo Training Ground

		return GERUDO_TRAINING_GROUND;
	}
	else
	{	// Inside Ganon Castle

		return INSIDE_GANON_CASTLE;
	}
}


uint32_t GetSceneGS(uint32_t GS)
{
	if (GS >= 0x08 && GS <= 0x0b)
	{	// Deku Tree

		return DEKU_TREE;
	}
	else if (GS >= 0x10 && GS <= 0x14)
	{	// Dodongo Cavern

		return DODONGO_CAVERN;
	}
	else if (GS >= 0x18 && GS <= 0x1b)
	{	// Jabu-Jabu

		return INSIDE_JABU_JABU;
	}
	else if (GS >= 0x20 && GS <= 0x24)
	{	// Forest Temple

		return TEMPLE_FOREST;
	}
	else if (GS >= 0x28 && GS <= 0x2c)
	{	// Fire Temple

		return TEMPLE_FIRE;
	}
	else if (GS >= 0x30 && GS <= 0x34)
	{	// Water Temple

		return TEMPLE_WATER;
	}
	else if (GS >= 0x38 && GS <= 0x3c)
	{	// Spirit Temple

		return TEMPLE_SPIRIT;
	}
	else if (GS >= 0x40 && GS <= 0x44)
	{	// Shadow Temple

		return TEMPLE_SHADOW;
	}
	else if (GS >= 0x48 && GS <= 0x4a)
	{	// Bottom of the Well

		return BOTTOM_OF_THE_WELL;
	}
	else if (GS >= 0x50 && GS <= 0x52)
	{	// Ice Cavern

		return ICE_CAVERN;
	}
	else if (GS == 0x58 || GS == 0x59 || GS == 0x79)
	{	// Grottos

		return OOT_GROTTOS;
	}
	else if (GS >= 0x60 && GS <= 0x63)
	{	// Lon Lon Ranch

		return LON_LON_RANCH;
	}
	else if (GS >= 0x68 && GS <= 0x6a)
	{	// Kokiri Forest

		return KOKIRI_FOREST;
	}
	else if (GS >= 0x70 && GS <= 0x72)
	{	// Lost Woods

		return OOT_LOST_WOODS;
	}
	else if (GS == 0x73)
	{	// Sacred Forest Meadow

		return SACRED_FOREST_MEADOW;
	}
	else if (GS == 0x78)
	{	// Ganon Castle Exterior

		return GANON_CASTLE_EXTERIOR;
	}
	else if (GS == 0x7a)
	{	// Hyrule Castle

		return HYRULE_CASTLE;
	}
	else if (GS == 0x7b)
	{	// Guard House

		return GUARD_HOUSE;
	}
	else if (GS == 0x80 || GS == 0x87)
	{	// Death Mountain Crater

		return DEATH_MOUNTAIN_CRATER;
	}
	else if (GS >= 0x81 && GS <= 0x84)
	{	// Death Mountain Trail

		return DEATH_MOUNTAIN_TRAIL;
	}
	else if (GS == 0x85 || GS == 0x86)
	{	// Goron City

		return GORON_CITY;
	}
	else if (GS == 0x88 || GS == 0x8f)
	{	// Graveyard

		return GRAVEYARD;
	}
	else if (GS >= 0x89 && GS <= 0x8e)
	{	// Kakariko Village

		return KAKARIKO_VILLAGE;
	}
	else if (GS == 0x90 || GS == 0x91 || GS == 0x93 || GS == 0x94)
	{	// Zora River

		return ZORA_RIVER;
	}
	else if (GS == 0x92 || GS == 0x95 || GS == 0x97)
	{	// Zora Fountain

		return ZORA_FOUNTAIN;
	}
	else if (GS == 0x96)
	{	// Zora Domain

		return ZORA_DOMAIN;
	}
	else if ((GS >= 0x98 && GS <= 0x9A) || GS == 0x9c)
	{	// Lake Hylia

		return LAKE_HYLIA;
	}
	else if (GS == 0x9b)
	{	// Laboratory

		return OOT_LABORATORY;
	}
	else if (GS >= 0xa0 && GS <= 0xa3)
	{	// Gerudo Valley

		return GERUDO_VALLEY;
	}
	else if (GS >= 0xa8 && GS <= 0xa9)
	{	// Gerudo Fortress

		return GERUDO_FORTRESS;
	}
	else if (GS == 0xb0 || GS == 0xb2 || GS == 0xb3)
	{	// Desert Colossus

		return DESERT_COLOSSUS;
	}
	else
	{	// Haunted Wasteland

		return HAUNTED_WASTELAND;
	}
}


uint32_t GetSceneScrub(uint32_t Scrub)
{
	if (Scrub >= 0x00 && Scrub <= 0x02)
	{	// Lost Woods

		return OOT_LOST_WOODS;
	}
	else if ((Scrub >= 0x03 && Scrub <= 0x0d) || (Scrub >= 0x0f && Scrub <= 0x1a))
	{	// Grottos

		return OOT_GROTTOS;
	}
	else if (Scrub == 0x0e)
	{	// Death Mountain Crater

		return DEATH_MOUNTAIN_CRATER;
	}
	else if (Scrub == 0x1b)
	{	// MQ Deku Tree

		return DEKU_TREE;
	}
	else if (Scrub >= 0x1c && Scrub <= 0x1f)
	{	// Dodongo Cavern

		return DODONGO_CAVERN;
	}
	else if (Scrub == 0x20)
	{	// Jabu-Jabu

		return INSIDE_JABU_JABU;
	}
	else
	{	// Inside Ganon Castle

		return INSIDE_GANON_CASTLE;
	}
}