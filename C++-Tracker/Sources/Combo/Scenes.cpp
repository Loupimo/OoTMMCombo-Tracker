#include "Combo/Scenes.h"
#include "Combo/NPC.h"
#include "Combo/Objects.h"
#include "Multi/Game.h"

#pragma region Scenes

SceneMetaInfo OoTScenesMetaInfo[OOT_NUM_SCENES] =
{
    { "Deku Tree", "", "./Resources/OoT/Dungeons/Deku_Tree/DEK_Map.jpg", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot },
    { "Dodongo's Cavern", "", "./Resources/OoT/Dungeons/Cavern/CAV_Map.jpg", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot },
    { "Inside Jabu-Jabu", "", "./Resources/OoT/Dungeons/Jabu-Jabu/JAB_Map.jpg", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot },
    { "Forest Temple", "", "./Resources/OoT/Dungeons/Forest/FOT_Map.jpg", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot },
    { "Fire Temple", "", "./Resources/OoT/Dungeons/Fire/FIT_Map.jpg", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot },
    { "Water Temple", "", "./Resources/OoT/Dungeons/Water/WT_Map.jpg", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot },
    { "Spirit Temple", "", "./Resources/OoT/Dungeons/Spirit/SPT_Map.jpg", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot },
    { "Shadow Temple", "", "./Resources/OoT/Dungeons/Shadow/SHT_Map.jpg", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot },
    { "Bottom of the Well", "", "./Resources/OoT/Dungeons/Well/WEL_Map.jpg", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot },
    { "Ice Cavern", "./Resources/OoT/Dungeons/Ice/Ice.jpg", "./Resources/OoT/Dungeons/Ice/Ice_Map.jpg", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot },
    { "Ganon Tower", "", "./Resources/OoT/Ganon/GT_Map.jpg", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot },
    { "Gerudo Training Ground", "./Resources/OoT/Dungeons/GTG/GTG.jpg", "./Resources/OoT/Dungeons/GTG/GTG_Map.jpg", (uint8_t)OoTRegions::Gerudo_Fortress, false, GameLayout::oot },
    { "Thieves' Hideout", "./Resources/OoT/Gerudo_Fortress/Hideout.jpg", "./Resources/OoT/Gerudo_Fortress/Hideout_Map.jpg", (uint8_t)OoTRegions::Gerudo_Fortress, false, GameLayout::oot },
    { "Inside Ganon Castle", "./Resources/OoT/Ganon/GT_1F.jpg", "./Resources/OoT/Ganon/GT_1F_Map.jpg", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot },
    { "Ganon Tower - Collapsing", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Inside Ganon Castle - Collapsing", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Treasure Shop", "./Resources/OoT/Market/Treasure_Game.jpg", "./Resources/OoT/Market/Treasure_Game_Map.jpg", (uint8_t)OoTRegions::Market, false, GameLayout::oot },
    { "Gohma's Lair", "", "./Resources/OoT/Dungeons/Deku_Tree/Gohma_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "King Dodongo's Lair", "", "./Resources/OoT/Dungeons/Cavern/King_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Barinade's Lair", "", "./Resources/OoT/Dungeons/Jabu-Jabu/Barinade_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Phantom Ganon's Lair", "", "./Resources/OoT/Dungeons/Forest/Phantom_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Volvagia's Lair", "", "./Resources/OoT/Dungeons/Fire/Volvagia_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Morpha's Lair", "", "./Resources/OoT/Dungeons/Water/Morpha_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Twinrova's Lair", "", "./Resources/OoT/Dungeons/Spirit/Twinrova_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Bongo-Bongo's Lair", "", "./Resources/OoT/Dungeons/Shadow/Bongo_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Ganondorf's Lair", "", "./Resources/OoT/Ganon/Ganondorf_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Tower Collapse - Exterior", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Market Entrance - Child Day", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Market Entrance - Child Night", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Market Entrance - Adult", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Back Alley - Day", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Back Alley - Night", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Market - Day", "./Resources/OoT/Market/Market_Day.jpg", "", (uint8_t)OoTRegions::Market, false, GameLayout::oot },
    { "Market - Night", "./Resources/OoT/Market/Market_Night.jpg", "", (uint8_t)OoTRegions::Market, false, GameLayout::oot },
    { "Market - Adult", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Temple of Time Exterior Child Day", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Temple of Time Exterior Child Night", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Temple of Time Exterior Adult", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Know-It-All Brothers's House", "./Resources/OoT/Kokiri_Forest/Know_It_All.jpg", "./Resources/OoT/Kokiri_Forest/Know_It_All_Map.jpg", (uint8_t)OoTRegions::Kokiri_Forest, false, GameLayout::oot },
    { "Twins's House", "./Resources/OoT/Kokiri_Forest/Twins_House.jpg", "./Resources/OoT/Kokiri_Forest/Twins_House_Map.jpg", (uint8_t)OoTRegions::Kokiri_Forest, false, GameLayout::oot },
    { "Mido's House", "./Resources/OoT/Kokiri_Forest/Mido_House.jpg", "./Resources/OoT/Kokiri_Forest/Mido_House_Map.jpg", (uint8_t)OoTRegions::Kokiri_Forest, false, GameLayout::oot },
    { "Saria's House", "./Resources/OoT/Kokiri_Forest/Saria_House.jpg", "./Resources/OoT/Kokiri_Forest/Saria_House_Map.jpg", (uint8_t)OoTRegions::Kokiri_Forest, false, GameLayout::oot },
    { "Carpenters Boss's House", "", "./Resources/OoT/Kakariko/Carpenter_House_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Back Alley's House", "./Resources/OoT/Market/Back_Alley_House.jpg", "./Resources/OoT/Market/Back_Alley_House_Map.jpg", (uint8_t)OoTRegions::Market, false, GameLayout::oot },
    { "Bazaar", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Kokiri Shop", "./Resources/OoT/Kokiri_Forest/Shop.jpg", "./Resources/OoT/Kokiri_Forest/Shop_Map.jpg", (uint8_t)OoTRegions::Kokiri_Forest, false, GameLayout::oot },
    { "Goron Shop", "./Resources/OoT/Goron_City/Shop.jpg", "./Resources/OoT/Goron_City/Shop_Map.jpg", (uint8_t)OoTRegions::Goron_City, false, GameLayout::oot },
    { "Zora Shop", "./Resources/OoT/Zora_Domain/Shop.jpg", "./Resources/OoT/Goron_City/Shop_Map.jpg", (uint8_t)OoTRegions::Zora_Domain, false, GameLayout::oot },
    { "Kakariko Potion Shop", "./Resources/OoT/Kakariko/Potion_Shop.jpg", "./Resources/OoT/Kakariko/Potion_Shop_Map.jpg", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot },
    { "Market Potion Shop", "./Resources/OoT/Market/Potion_Shop.jpg", "./Resources/OoT/Market/Potion_Shop_Map.jpg", (uint8_t)OoTRegions::Market, false, GameLayout::oot },
    { "Bombchu Shop", "./Resources/OoT/Market/Bombchu_Shop.jpg", "./Resources/OoT/Market/Bombchu_Shop_Map.jpg", (uint8_t)OoTRegions::Market, false, GameLayout::oot },
    { "Happy Mask Shop", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Link's House", "./Resources/OoT/Kokiri_Forest/Link_House.jpg", "./Resources/OoT/Kokiri_Forest/Link_House_Map.jpg", (uint8_t)OoTRegions::Kokiri_Forest, true, GameLayout::oot },
    { "Dog Lady's House", "./Resources/OoT/Market/Dog_Lady_House.jpg", "./Resources/OoT/Market/Dog_Lady_House_Map.jpg", (uint8_t)OoTRegions::Market, false, GameLayout::oot },
    { "Stable", "./Resources/OoT/Ranch/Stable.jpg", "./Resources/OoT/Ranch/Stable_Map.jpg", (uint8_t)OoTRegions::Lon_Lon_Ranch, false, GameLayout::oot },
    { "Impa's House", "./Resources/OoT/Kakariko/Impa_House.jpg", "./Resources/OoT/Kakariko/Impa_House_Map.jpg", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot },
    { "Laboratory", "./Resources/OoT/Lake_Hylia/Laboratory.jpg", "./Resources/OoT/Lake_Hylia/Laboratory_Map.jpg", (uint8_t)OoTRegions::Lake_Hylia, false, GameLayout::oot },
    { "Carpenter's Tent", "", "./Resources/OoT/Gerudo_Valley/Carpenter_Tent_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Dampe's House", "", "./Resources/OoT/Graveyard/Dampe_House_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Great Fairy Fountain - Upgrades", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Fairy Fountain", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Great Fairy Fountain - Spells", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Grottos", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Redead's Tomb", "./Resources/OoT/Graveyard/Redead_Tomb.jpg", "./Resources/OoT/Graveyard/Redead_Tomb_Map.jpg", (uint8_t)OoTRegions::Graveyard, false, GameLayout::oot },
    { "Fairy's Fountain Tomb", "./Resources/OoT/Graveyard/Fairy_Fountain_Tomb.jpg", "./Resources/OoT/Graveyard/Fairy_Fountain_Tomb_Map.jpg", (uint8_t)OoTRegions::Graveyard, false, GameLayout::oot },
    { "Royal's Family Tomb", "./Resources/OoT/Graveyard/Royal_Tomb.jpg", "./Resources/OoT/Graveyard/Royal_Tomb_Map.jpg", (uint8_t)OoTRegions::Graveyard, false, GameLayout::oot },
    { "Shooting Gallery", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Temple of Time", "./Resources/OoT/Market/Temple_of_Time.jpg", "./Resources/OoT/Market/Temple_of_Time_Map.jpg", (uint8_t)OoTRegions::Market, false, GameLayout::oot },
    { "Chamber of the Sages", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Castle Maze - Day", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Castle Maze - Night", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Cutscene Map", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Dampe's Tomb", "./Resources/OoT/Graveyard/Dampe_Tomb.jpg", "./Resources/OoT/Graveyard/Dampe_Tomb_Map.jpg", (uint8_t)OoTRegions::Graveyard, false, GameLayout::oot },
    { "Fishing Pond", "./Resources/OoT/Lake_Hylia/Pond.jpg", "./Resources/OoT/Lake_Hylia/Pond_Map.jpg", (uint8_t)OoTRegions::Lake_Hylia, true, GameLayout::oot },
    { "Castle Courtyard", "./Resources/OoT/Hyrule/Castle_Courtyard.jpg", "./Resources/OoT/Hyrule/Castle_Courtyard_Map.jpg", (uint8_t)OoTRegions::Castle, false, GameLayout::oot },
    { "Bombchu Bowling", "./Resources/OoT/Market/Bombchu_Bowling.jpg", "./Resources/OoT/Market/Bombchu_Bowling_Map.jpg", (uint8_t)OoTRegions::Market, false, GameLayout::oot },
    { "Talon's House", "./Resources/OoT/Ranch/Ranch_House_Silo.jpg", "./Resources/OoT/Ranch/Ranch_House_Silo_Map.jpg", (uint8_t)OoTRegions::Lon_Lon_Ranch, true, GameLayout::oot },
    { "Pot's House", "./Resources/OoT/Market/Pot_House.jpg", "./Resources/OoT/Market/Pot_House_Map.jpg", (uint8_t)OoTRegions::Market, true, GameLayout::oot },
    { "Granny Potion Shop", "./Resources/OoT/Kakariko/Granny_Potion_Shop.jpg", "./Resources/OoT/Kakariko/Granny_Potion_Shop_Map.jpg", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot },
    { "Ganon Battle Arena", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "House of Skulltula", "./Resources/OoT/Kakariko/House_Skulltula.jpg", "./Resources/OoT/Kakariko/House_Skulltula_Map.jpg", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot },
    { "Hyrule Field", "./Resources/OoT/Hyrule/Hyrule_Field.jpg", "./Resources/OoT/Hyrule/Hyrule_Field_Map.jpg", (uint8_t)OoTRegions::Hyrule, true, GameLayout::oot },
    { "Kakariko's Village", "./Resources/OoT/Kakariko/Kakariko.jpg", "./Resources/OoT/Kakariko/Kakariko_Map.jpg", (uint8_t)OoTRegions::Kakariko, true, GameLayout::oot },
    { "Graveyard", "./Resources/OoT/Graveyard/Graveyard.jpg", "./Resources/OoT/Graveyard/Graveyard_Map.jpg", (uint8_t)OoTRegions::Graveyard, true, GameLayout::oot },
    { "Zora's River", "./Resources/OoT/Zora_River/Zora_River.jpg", "./Resources/OoT/Zora_River/Zora_River_Map.jpg", (uint8_t)OoTRegions::Zora_River, true, GameLayout::oot },
    { "Kokiri Forest", "./Resources/OoT/Kokiri_Forest/Kokiri_Forest.jpg", "./Resources/OoT/Kokiri_Forest/Kokiri_Forest_Map.jpg", (uint8_t)OoTRegions::Kokiri_Forest, true, GameLayout::oot },
    { "Sacred Forest Meadow", "./Resources/OoT/Sacred_Forest_Meadow/Sacred_Forest_Meadow.jpg", "./Resources/OoT/Sacred_Forest_Meadow/Sacred_Forest_Meadow_Map.jpg", (uint8_t)OoTRegions::Sacred_Forest_Meadow, true, GameLayout::oot },
    { "Lake Hylia", "./Resources/OoT/Lake_Hylia/Lake_Hylia.jpg", "./Resources/OoT/Lake_Hylia/Lake_Hylia_Map.jpg", (uint8_t)OoTRegions::Lake_Hylia, true, GameLayout::oot },
    { "Zora's Domain", "./Resources/OoT/Zora_Domain/Zora_Domain.jpg", "./Resources/OoT/Zora_Domain/Zora_Domain_Map.jpg", (uint8_t)OoTRegions::Zora_Domain, true, GameLayout::oot },
    { "Zora's Fountain", "./Resources/OoT/Zora_Fountain/Zora_Fountain.jpg", "./Resources/OoT/Zora_Fountain/Zora_Fountain_Map.jpg", (uint8_t)OoTRegions::Zora_Fountain, true, GameLayout::oot },
    { "Gerudo Valley", "./Resources/OoT/Gerudo_Valley/Valley.jpg", "./Resources/OoT/Gerudo_Valley/Valley_Map.jpg", (uint8_t)OoTRegions::Gerudo_Valley, true, GameLayout::oot },
    { "Lost Woods", "./Resources/OoT/Lost_Woods/Lost_Woods.jpg", "./Resources/OoT/Lost_Woods/Lost_Woods_Map.jpg", (uint8_t)OoTRegions::Lost_Woods, true, GameLayout::oot },
    { "Desert Colossus", "./Resources/OoT/Desert_Colossus/Desert_Colossus.jpg", "./Resources/OoT/Desert_Colossus/Desert_Colossus_Map.jpg", (uint8_t)OoTRegions::Desert_Colossus, true, GameLayout::oot },
    { "Gerudo's Fortress", "./Resources/OoT/Gerudo_Fortress/Fortress.jpg", "./Resources/OoT/Gerudo_Fortress/Fortress_Map.jpg", (uint8_t)OoTRegions::Gerudo_Fortress, true, GameLayout::oot },
    { "Haunted Wasteland", "./Resources/OoT/Haunted_Wasteland/Haunted_Wasteland.jpg", "./Resources/OoT/Haunted_Wasteland/Haunted_Wasteland_Map.jpg", (uint8_t)OoTRegions::Haunted_Wasteland, false, GameLayout::oot },
    { "Hyrule Castle", "./Resources/OoT/Hyrule/Hyrule_Castle.jpg", "./Resources/OoT/Hyrule/Hyrule_Castle_Map.jpg", (uint8_t)OoTRegions::Castle, false, GameLayout::oot },
    { "Death Mountain Trail", "", "./Resources/OoT/Death_Mountain_Trail/DMT_Map.jpg", (uint8_t)OoTRegions::Death_Mountain_Trail, true, GameLayout::oot },
    { "Death Mountain Crater", "./Resources/OoT/Death_Mountain_Crater/Crater.jpg", "./Resources/OoT/Death_Mountain_Crater/Crater_Map.jpg", (uint8_t)OoTRegions::Death_Mountain_Crater, true, GameLayout::oot },
    { "Goron City", "./Resources/OoT/Goron_City/Goron_City.jpg", "./Resources/OoT/Goron_City/Goron_City_Map.jpg", (uint8_t)OoTRegions::Goron_City, true, GameLayout::oot },
    { "Lon Lon's Ranch", "./Resources/OoT/Ranch/Lon_Lon_Ranch.jpg", "./Resources/OoT/Ranch/Lon_Lon_Ranch_Map.jpg", (uint8_t)OoTRegions::Lon_Lon_Ranch, true, GameLayout::oot },
    { "Ganon Castle Exterior", "./Resources/OoT/Ganon/Exterior.jpg", "./Resources/OoT/Ganon/Exterior_Map.jpg", (uint8_t)OoTRegions::Castle, false, GameLayout::oot },
    { "Song of Storms Grotto", "./Resources/OoT/Grottos/Open.jpg", "./Resources/OoT/Grottos/Open_Map.jpg", (uint8_t)OoTRegions::Kokiri_Forest, false, GameLayout::oot },
    { "Deku Scrubs Grotto", "./Resources/OoT/Grottos/Double_Scrubs.jpg", "./Resources/OoT/Grottos/Double_Scrubs_Map.jpg", (uint8_t)OoTRegions::Lost_Woods, false, GameLayout::oot },
    { "Generic Grotto", "./Resources/OoT/Grottos/Open.jpg", "./Resources/OoT/Grottos/Open_Map.jpg", (uint8_t)OoTRegions::Lost_Woods, false, GameLayout::oot },
    { "Deku's Theater", "./Resources/OoT/Lost_Woods/Theater.jpg", "./Resources/OoT/Lost_Woods/Theater_Map.jpg", (uint8_t)OoTRegions::Lost_Woods, false, GameLayout::oot },
    { "Wolfos Grotto", "./Resources/OoT/Sacred_Forest_Meadow/Wolfos.jpg", "./Resources/OoT/Sacred_Forest_Meadow/Wolfos_Map.jpg", (uint8_t)OoTRegions::Sacred_Forest_Meadow, false, GameLayout::oot },
    { "Song of Storms Grotto", "./Resources/OoT/Grottos/Double_Scrubs.jpg", "./Resources/OoT/Grottos/Double_Scrubs_Map.jpg", (uint8_t)OoTRegions::Sacred_Forest_Meadow, false, GameLayout::oot },
    { "Redead Grotto", "./Resources/OoT/Kakariko/Redead.jpg", "./Resources/OoT/Kakariko/Redead_Map.jpg", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot },
    { "Open Grotto", "./Resources/OoT/Grottos/Open.jpg", "./Resources/OoT/Grottos/Open_Map.jpg", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot },
    { "Song of Storms Grotto", "./Resources/OoT/Grottos/Open.jpg", "./Resources/OoT/Grottos/Open_Map.jpg", (uint8_t)OoTRegions::Death_Mountain_Trail, false, GameLayout::oot },
    { "Cow Grotto", "./Resources/OoT/Grottos/Cow.jpg", "./Resources/OoT/Grottos/Cow_Map.jpg", (uint8_t)OoTRegions::Death_Mountain_Trail, false, GameLayout::oot },
    { "Deku Scrubs Grotto", "./Resources/OoT/Grottos/Triple_Scrubs.jpg", "./Resources/OoT/Grottos/Triple_Scrubs_Map.jpg", (uint8_t)OoTRegions::Goron_City, false, GameLayout::oot },
    { "Generic Grotto", "./Resources/OoT/Grottos/Open.jpg", "./Resources/OoT/Grottos/Open_Map.jpg", (uint8_t)OoTRegions::Death_Mountain_Crater, false, GameLayout::oot },
    { "Deku Scrubs Grotto", "./Resources/OoT/Grottos/Triple_Scrubs.jpg", "./Resources/OoT/Grottos/Triple_Scrubs_Map.jpg", (uint8_t)OoTRegions::Death_Mountain_Crater, false, GameLayout::oot },
    { "Song of Storms Grotto", "./Resources/OoT/Grottos/Double_Scrubs.jpg", "./Resources/OoT/Grottos/Double_Scrubs_Map.jpg", (uint8_t)OoTRegions::Zora_River, false, GameLayout::oot },
    { "Generic Grotto", "./Resources/OoT/Grottos/Open.jpg", "./Resources/OoT/Grottos/Open_Map.jpg", (uint8_t)OoTRegions::Zora_River, false, GameLayout::oot },
    { "Deku Scrubs Grotto", "./Resources/OoT/Grottos/Triple_Scrubs.jpg", "./Resources/OoT/Grottos/Triple_Scrubs_Map.jpg", (uint8_t)OoTRegions::Lake_Hylia, false, GameLayout::oot },
    { "Deku Scrubs Grotto", "./Resources/OoT/Grottos/Triple_Scrubs.jpg", "./Resources/OoT/Grottos/Triple_Scrubs_Map.jpg", (uint8_t)OoTRegions::Lon_Lon_Ranch, false, GameLayout::oot },
    { "Deku Scrubs Grotto", "./Resources/OoT/Grottos/Cow.jpg", "./Resources/OoT/Grottos/Field_Scrub_Map.jpg", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot },
    { "Southeast Grotto", "./Resources/OoT/Grottos/Open.jpg", "./Resources/OoT/Grottos/Open_Map.jpg", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot },
    { "Open Grotto", "./Resources/OoT/Grottos/Open.jpg", "./Resources/OoT/Grottos/Open_Map.jpg", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot },
    { "Market Side Grotto", "./Resources/OoT/Grottos/Open.jpg", "./Resources/OoT/Grottos/Open_Map.jpg", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot },
    { "Tektite Grotto", "./Resources/OoT/Hyrule/Field_Tektite.jpg", "./Resources/OoT/Hyrule/Field_Tektite_Map.jpg", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot },
    { "Kakariko Side Grotto", "./Resources/OoT/Hyrule/Field_Kakariko.jpg", "./Resources/OoT/Hyrule/Field_Kakariko_Map.jpg", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot },
    { "Gerudo Side Grotto", "./Resources/OoT/Hyrule/Field_Gerudo.jpg", "./Resources/OoT/Hyrule/Field_Gerudo_Map.jpg", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot },
    { "Song of Storms Grotto", "./Resources/OoT/Hyrule/Castle_Storms.jpg", "./Resources/OoT/Hyrule/Castle_Storms_Map.jpg", (uint8_t)OoTRegions::Castle, false, GameLayout::oot },
    { "Song of Storms Grotto", "./Resources/OoT/Grottos/Double_Scrubs.jpg", "./Resources/OoT/Grottos/Double_Scrubs_Map.jpg", (uint8_t)OoTRegions::Gerudo_Valley, false, GameLayout::oot },
    { "Octorok Grotto", "./Resources/OoT/Gerudo_Valley/Octorok.jpg", "./Resources/OoT/Gerudo_Valley/Octorok_Map.jpg", (uint8_t)OoTRegions::Gerudo_Valley, false, GameLayout::oot },
    { "Deku Scrubs Grotto", "./Resources/OoT/Grottos/Double_Scrubs.jpg", "./Resources/OoT/Grottos/Double_Scrubs_Map.jpg", (uint8_t)OoTRegions::Desert_Colossus, false, GameLayout::oot },
    { "Meadow Fairy Fountain", "./Resources/OoT/Fairy/Fountain.jpg", "./Resources/OoT/Fairy/Fountain_Map.jpg", (uint8_t)OoTRegions::Sacred_Forest_Meadow, false, GameLayout::oot },
    { "River Fairy Fountain", "./Resources/OoT/Fairy/Fountain.jpg", "./Resources/OoT/Fairy/Fountain_Map.jpg", (uint8_t)OoTRegions::Zora_River, false, GameLayout::oot },
    { "Domain Fairy Fountain", "./Resources/OoT/Fairy/Fountain.jpg", "./Resources/OoT/Fairy/Fountain_Map.jpg", (uint8_t)OoTRegions::Zora_Domain, false, GameLayout::oot },
    { "Field Fairy Fountain", "./Resources/OoT/Fairy/Fountain.jpg", "./Resources/OoT/Fairy/Fountain_Map.jpg", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot },
    { "Fortress Fairy Fountain", "./Resources/OoT/Fairy/Fountain.jpg", "./Resources/OoT/Fairy/Fountain_Map.jpg", (uint8_t)OoTRegions::Gerudo_Fortress, false, GameLayout::oot },
    { "Castle Great Fairy Fountain", "./Resources/OoT/Fairy/Great_Fairy.jpg", "./Resources/OoT/Fairy/Great_Fairy_Map.jpg", (uint8_t)OoTRegions::Castle, true, GameLayout::oot },
    { "Great Fairy Fountain", "", "./Resources/OoT/Fairy/Great_Fairy_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Zora Great Fairy Fountain", "./Resources/OoT/Fairy/Great_Fairy.jpg", "./Resources/OoT/Fairy/Great_Fairy_Map.jpg", (uint8_t)OoTRegions::Zora_Fountain, false, GameLayout::oot },
    { "Desert Great Fairy Fountain", "./Resources/OoT/Fairy/Great_Fairy.jpg", "./Resources/OoT/Fairy/Great_Fairy_Map.jpg", (uint8_t)OoTRegions::Desert_Colossus, false, GameLayout::oot },
    { "Trail Great Fairy Fountain", "./Resources/OoT/Fairy/Great_Fairy.jpg", "./Resources/OoT/Fairy/Great_Fairy_Map.jpg", (uint8_t)OoTRegions::Death_Mountain_Trail, false, GameLayout::oot },
    { "Crater Great Fairy Fountain", "./Resources/OoT/Fairy/Great_Fairy.jpg", "./Resources/OoT/Fairy/Great_Fairy_Map.jpg", (uint8_t)OoTRegions::Death_Mountain_Crater, false, GameLayout::oot },
    { "Great Fairy Fountain", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Kakariko Bazaar", "./Resources/OoT/Kakariko/Bazaar.jpg", "./Resources/OoT/Market/Bazaar_Map.jpg", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot },
    { "Market Bazaar", "./Resources/OoT/Market/Bazaar.jpg", "./Resources/OoT/Market/Bazaar_Map.jpg", (uint8_t)OoTRegions::Market, false, GameLayout::oot },
    { "Kakariko Shooting Gallery", "./Resources/OoT/Kakariko/Shooting.jpg", "./Resources/OoT/Market/Shooting_Map.jpg", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot },
    { "Market Shooting Gallery", "./Resources/OoT/Market/Shooting.jpg", "./Resources/OoT/Market/Shooting_Map.jpg", (uint8_t)OoTRegions::Market, false, GameLayout::oot },
    { "Silo", "./Resources/OoT/Ranch/Silo.jpg", "./Resources/OoT/Ranch/Silo_Map.jpg", (uint8_t)OoTRegions::Lon_Lon_Ranch, false, GameLayout::oot },
    { "Windmill", "./Resources/OoT/Kakariko/Windmill.jpg", "./Resources/OoT/Kakariko/Windmill_Map.jpg", (uint8_t)OoTRegions::Kakariko, true, GameLayout::oot },
    { "Market Entrance", "", "./Resources/OoT/Market/Market_Entrance_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Temple of Time Entryway", "./Resources/OoT/Market/Temple_Entry.jpg", "./Resources/OoT/Market/Temple_of_Time_Entryway_Map.jpg", (uint8_t)OoTRegions::Market, false, GameLayout::oot },
    { "Back Alley", "", "./Resources/OoT/Market/Back_Alley_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Warp Songs", "", "./Resources/OoT/Songs/Song_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Inside Eggs", "", "", (uint8_t)OoTRegions::None, false, GameLayout::oot },
    { "Market", "", "./Resources/OoT/Market/Market_Map.jpg", (uint8_t)OoTRegions::None, false, GameLayout::oot }
};


SceneMetaInfo MMScenesMetaInfo[MM_NUM_SCENES] =
{
    { "Southern Swamp - Clear", " ", " ", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Fairy Fountain", "./Resources/MM/Snowhead/Fairy.jpg", "./Resources/MM/Snowhead/Fairy_Map.jpg", (uint8_t)MMRegions::Snowhead, false, GameLayout::mm },
    { "Fairy Fountain", "./Resources/MM/Zora_Cape/Fairy.jpg", "./Resources/MM/Snowhead/Fairy_Map.jpg", (uint8_t)MMRegions::Zora_Cape, false, GameLayout::mm },
    { "Fairy Fountain", "./Resources/MM/Woodfall/Fairy.jpg", "./Resources/MM/Snowhead/Fairy_Map.jpg", (uint8_t)MMRegions::Woodfall, false, GameLayout::mm },
    { "Fairy Fountain", "./Resources/MM/Clock_Town/Fairy.jpg", "./Resources/MM/Snowhead/Fairy_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Fairy Fountain", "./Resources/MM/Ikana_Canyon/Fairy.jpg", "./Resources/MM/Snowhead/Fairy_Map.jpg", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm },
    { "Path to Snowhead, Spring", "", "", (uint8_t)MMRegions::None, true, GameLayout::mm },
    { "Grottos", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Cutscene Map", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Path to Mountain Village, Spring", "", "", (uint8_t)MMRegions::None, true, GameLayout::mm },
    { "Potion Shop", "./Resources/MM/Southern_Swamp/Potion_Shop.jpg", "./Resources/MM/Southern_Swamp/Potion_Shop_Map.jpg", (uint8_t)MMRegions::Southern_Swamp, false, GameLayout::mm },
    { "Majora's Lair", "./Resources/MM/Moon/Majora.jpg", "./Resources/MM/Moon/Majora_Map.jpg", (uint8_t)MMRegions::Moon, false, GameLayout::mm },
    { "Beneath the Graveyard", "./Resources/MM/Graveyard/Beneath_Graveyard.jpg", "./Resources/MM/Graveyard/Beneath_Graveyard_Map.jpg", (uint8_t)MMRegions::Graveyard, false, GameLayout::mm },
    { "Curiosity Shop", "./Resources/MM/Clock_Town/Curiosity_Shop.jpg", "./Resources/MM/Clock_Town/Curiosity_Shop_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Beneath the Graveyard, Night 1", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Beneath the Graveyard, Night 2", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Ranch Barn", "./Resources/MM/Ranch/Stable.jpg", "./Resources/MM/Ranch/Stable_Map.jpg", (uint8_t)MMRegions::Ranch, false, GameLayout::mm },
    { "Honey & Darling", "./Resources/MM/Clock_Town/Honey_Darling.jpg", "./Resources/MM/Clock_Town/Honey_Darling_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Town Hall", "./Resources/MM/Clock_Town/Town_Hall.jpg", "./Resources/MM/Clock_Town/Town_Hall_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Ikana Canyon", "./Resources/MM/Ikana_Canyon/Ikana_Canyon.jpg", "./Resources/MM/Ikana_Canyon/Ikana_Canyon_Map.jpg", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm },
    { "Pirate's Fortress, Exterior", "./Resources/MM/Fortress/Exterior.jpg", "./Resources/MM/Fortress/Exterior_Map.jpg", (uint8_t)MMRegions::Fortress, false, GameLayout::mm },
    { "Milk Bar", "./Resources/MM/Clock_Town/Milk_Bar.jpg", "./Resources/MM/Clock_Town/Milk_Bar_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Stone Tower Temple", "", "./Resources/MM/Dungeons/Stone_Tower/STT_Map.jpg", (uint8_t)MMRegions::Dungeons, false, GameLayout::mm },
    { "Treasure Shop", "./Resources/MM/Clock_Town/Treasure.jpg", "./Resources/MM/Clock_Town/Treasure_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Stone Tower Temple, Inverted", "", "./Resources/MM/Dungeons/Stone_Tower_Inverted/STIT_Map.jpg", (uint8_t)MMRegions::Dungeons, false, GameLayout::mm },
    { "Clock Tower Rooftop", "./Resources/MM/Clock_Town/Tower_Rooftop.jpg", "./Resources/MM/Clock_Town/Tower_Rooftop_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Opening", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Woodfall Temple", "", "./Resources/MM/Dungeons/Woodfall/WFT_Map.jpg", (uint8_t)MMRegions::Dungeons, false, GameLayout::mm },
    { "Path to Mountain Village", "./Resources/MM/Path_to_Mountain_Village/Path.jpg", "./Resources/MM/Path_to_Mountain_Village/Path_Map.jpg", (uint8_t)MMRegions::Path_to_Mountain_Village, true, GameLayout::mm },
    { "Ancient Ikana Castle", "", "./Resources/MM/Ikana_Canyon/Exterior_Map.jpg", (uint8_t)MMRegions::Dungeons, false, GameLayout::mm },
    { "Deku's Playground", "./Resources/MM/Clock_Town/Deku_Playground.jpg", "./Resources/MM/Clock_Town/Deku_Playground_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Odolwa's Lair", "", "./Resources/MM/Dungeons/Woodfall/Odolwa_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Shooting Gallery", "./Resources/MM/Clock_Town/Shooting.jpg", "./Resources/MM/Clock_Town/Shooting_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Snowhead Temple", "", "./Resources/MM/Dungeons/Snowhead/SHT_Map.jpg", (uint8_t)MMRegions::Dungeons, false, GameLayout::mm },
    { "Milk Road", "./Resources/MM/Milk_Road/Milk_Road.jpg", "./Resources/MM/Milk_Road/Milk_Road_Map.jpg", (uint8_t)MMRegions::Milk_Road, false, GameLayout::mm },
    { "Pirate's Fortress, Interior", "./Resources/MM/Fortress/Interior.jpg", "./Resources/MM/Fortress/Interior_Map.jpg", (uint8_t)MMRegions::Fortress, false, GameLayout::mm },
    { "Shooting Gallery", "./Resources/MM/Road_to_Southern_Swamp/Shooting.jpg", "./Resources/MM/Road_to_Southern_Swamp/Shooting_Map.jpg", (uint8_t)MMRegions::Southern_Swamp_Road, false, GameLayout::mm },
    { "Pinnacle Rock", "./Resources/MM/Great_Bay_Coast/Rock.jpg", "./Resources/MM/Great_Bay_Coast/Rock_Map.jpg", (uint8_t)MMRegions::Great_Bay_Coast, false, GameLayout::mm },
    { "Fairy Fountain", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Spider's House", "./Resources/MM/Southern_Swamp/Spider_House.jpg", "./Resources/MM/Southern_Swamp/Spider_House_Map.jpg", (uint8_t)MMRegions::Southern_Swamp, false, GameLayout::mm },
    { "Spider's House", "./Resources/MM/Great_Bay_Coast/Spider_House.jpg", "./Resources/MM/Great_Bay_Coast/Spider_House_Map.jpg", (uint8_t)MMRegions::Great_Bay_Coast, false, GameLayout::mm },
    { "Observatory", "./Resources/MM/Clock_Town/Observatory.jpg", "./Resources/MM/Clock_Town/Observatory_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Deku's Trial", "./Resources/MM/Moon/Deku.jpg", "./Resources/MM/Moon/Deku_Map.jpg", (uint8_t)MMRegions::Moon, false, GameLayout::mm },
    { "Deku's Palace", "./Resources/MM/Deku_Palace/Outside.jpg", "./Resources/MM/Deku_Palace/Outside_Map.jpg", (uint8_t)MMRegions::Deku_Palace, false, GameLayout::mm },
    { "Blacksmith", "./Resources/MM/Mountain_Village/Blacksmith.jpg", "./Resources/MM/Mountain_Village/Blacksmith_Map.jpg", (uint8_t)MMRegions::Mountain_Village, false, GameLayout::mm },
    { "Termina Field", "./Resources/MM/Termina/Field.jpg", "./Resources/MM/Termina/Field_Map.jpg", (uint8_t)MMRegions::Termina, false, GameLayout::mm },
    { "Post Office", "./Resources/MM/Clock_Town/Post_Office.jpg", "./Resources/MM/Clock_Town/Post_Office_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Laboratory", "./Resources/MM/Great_Bay_Coast/Laboratory.jpg", "./Resources/MM/Great_Bay_Coast/Laboratory_Map.jpg", (uint8_t)MMRegions::Great_Bay_Coast, false, GameLayout::mm },
    { "Dampe's House", "./Resources/MM/Graveyard/Dampe.jpg", "./Resources/MM/Graveyard/Dampe_Map.jpg", (uint8_t)MMRegions::Graveyard, false, GameLayout::mm },
    { "Inside Ancient Ikana Castle", "", "./Resources/MM/Ikana_Canyon/Interior_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Goron's Shrine", "./Resources/MM/Goron_Village/City.jpg", "./Resources/MM/Goron_Village/City_Map.jpg", (uint8_t)MMRegions::Goron_Village, false, GameLayout::mm },
    { "Zora's Theater", "./Resources/MM/Zora_Hall/Zora_Hall.jpg", "./Resources/MM/Zora_Hall/Zora_Hall_Map.jpg", (uint8_t)MMRegions::Zora_Hall, false, GameLayout::mm },
    { "Trading Post", "./Resources/MM/Clock_Town/Trading_Post.jpg", "./Resources/MM/Clock_Town/Trading_Post_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Romani's Ranch", "./Resources/MM/Ranch/Romani_Ranch.jpg", "./Resources/MM/Ranch/Romani_Ranch_Map.jpg", (uint8_t)MMRegions::Ranch, false, GameLayout::mm },
    { "Twinmold Lair", "", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Twinmold_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Great Bay Coast", "./Resources/MM/Great_Bay_Coast/Great_Bay.jpg", "./Resources/MM/Great_Bay_Coast/Great_Bay_Map.jpg", (uint8_t)MMRegions::Great_Bay_Coast, false, GameLayout::mm },
    { "Zora Cape", "./Resources/MM/Zora_Cape/Cape.jpg", "./Resources/MM/Zora_Cape/Cape_Map.jpg", (uint8_t)MMRegions::Zora_Cape, false, GameLayout::mm },
    { "Lottery", "./Resources/MM/Clock_Town/Lottery.jpg", "./Resources/MM/Clock_Town/Lottery_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Snowhead, Spring", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Pirate's Fortress, Entrance", "./Resources/MM/Fortress/Entrance.jpg", "./Resources/MM/Fortress/Entrance_Map.jpg", (uint8_t)MMRegions::Fortress, false, GameLayout::mm },
    { "Fisherman's Hut", "", "./Resources/MM/Great_Bay_Coast/Fisherman_Hut_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Goron Shop", "./Resources/MM/Goron_Village/Shop.jpg", "./Resources/MM/Goron_Village/Shop_Map.jpg", (uint8_t)MMRegions::Goron_Village, false, GameLayout::mm },
    { "Deku King's Chamber", "./Resources/MM/Deku_Palace/King.jpg", "./Resources/MM/Deku_Palace/King_Map.jpg", (uint8_t)MMRegions::Deku_Palace, false, GameLayout::mm },
    { "Goron's Trial", "./Resources/MM/Moon/Goron.jpg", "./Resources/MM/Moon/Goron_Map.jpg", (uint8_t)MMRegions::Moon, false, GameLayout::mm },
    { "Road to Southern Swamp", "./Resources/MM/Road_to_Southern_Swamp/Road.jpg", "./Resources/MM/Road_to_Southern_Swamp/Road_Map.jpg", (uint8_t)MMRegions::Southern_Swamp_Road, false, GameLayout::mm },
    { "Dog's Racetrack", "./Resources/MM/Ranch/Dog_Race.jpg", "./Resources/MM/Ranch/Dog_Race_Map.jpg", (uint8_t)MMRegions::Ranch, false, GameLayout::mm },
    { "Cucco's Shack", "./Resources/MM/Ranch/Shack.jpg", "./Resources/MM/Ranch/Shack_Map.jpg", (uint8_t)MMRegions::Ranch, false, GameLayout::mm },
    { "Ikana Graveyard", "./Resources/MM/Graveyard/Graveyard.jpg", "./Resources/MM/Graveyard/Graveyard_Map.jpg", (uint8_t)MMRegions::Graveyard, false, GameLayout::mm },
    { "Goht's Lair", "", "./Resources/MM/Dungeons/Snowhead/Goht_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Southern Swamp", "./Resources/MM/Southern_Swamp/Swamp.jpg", "./Resources/MM/Southern_Swamp/Swamp_Map.jpg", (uint8_t)MMRegions::Southern_Swamp, false, GameLayout::mm },
    { "Woodfall", "./Resources/MM/Woodfall/Woodfall.jpg", "./Resources/MM/Woodfall/Woodfall_Map.jpg", (uint8_t)MMRegions::Woodfall, false, GameLayout::mm },
    { "Zora's Trial", "./Resources/MM/Moon/Zora.jpg", "./Resources/MM/Moon/Zora_Map.jpg", (uint8_t)MMRegions::Moon, false, GameLayout::mm },
    { "Goron's Village, Spring", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Great Bay Temple", "", "./Resources/MM/Dungeons/Great_Bay/GBT_Map.jpg", (uint8_t)MMRegions::Dungeons, false, GameLayout::mm },
    { "Waterfall Rapids", "./Resources/MM/Zora_Cape/Waterfall_Rapids.jpg", "./Resources/MM/Zora_Cape/Waterfall_Rapids_Map.jpg", (uint8_t)MMRegions::Zora_Cape, false, GameLayout::mm },
    { "Beneath the Well", "./Resources/MM/Ikana_Canyon/Beneath_Well.jpg", "./Resources/MM/Ikana_Canyon/Beneath_Well_Map.jpg", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm },
    { "Zora's Hall Rooms", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Goron's Village, Winter", "./Resources/MM/Goron_Village/Village.jpg", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Goron's Graveyard", "./Resources/MM/Mountain_Village/Graveyard.jpg", "./Resources/MM/Mountain_Village/Graveyard_Map.jpg", (uint8_t)MMRegions::Mountain_Village, false, GameLayout::mm },
    { "Sakon's Hideout", "./Resources/MM/Ikana_Canyon/Sakon_Hideout.jpg", "./Resources/MM/Ikana_Canyon/Sakon_Hideout_Map.jpg", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm },
    { "Mountain Village, Winter", "", "", (uint8_t)MMRegions::None, true, GameLayout::mm },
    { "Ghost Hut", "./Resources/MM/Ikana_Canyon/Ghost_Hut.jpg", "./Resources/MM/Ikana_Canyon/Ghost_Hut_Map.jpg", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm },
    { "Deku's Shrine", "./Resources/MM/Deku_Palace/Shrine.jpg", "./Resources/MM/Deku_Palace/Shrine_Map.jpg", (uint8_t)MMRegions::Deku_Palace, false, GameLayout::mm },
    { "Road to Ikana", "./Resources/MM/Road_to_Ikana/Road.jpg", "./Resources/MM/Road_to_Ikana/Road_Map.jpg", (uint8_t)MMRegions::Road_to_Ikana, false, GameLayout::mm },
    { "Swordsman's School", "./Resources/MM/Clock_Town/Swordsman_School.jpg", "./Resources/MM/Clock_Town/Swordsman_School_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Music Box House", "./Resources/MM/Ikana_Canyon/Music_Box_House.jpg", "./Resources/MM/Ikana_Canyon/Music_Box_House_Map.jpg", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm },
    { "Ikana's Lair", "", "./Resources/MM/Ikana_Canyon/Igos_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Tourist Information", "./Resources/MM/Southern_Swamp/Tourist_Information.jpg", "./Resources/MM/Southern_Swamp/Tourist_Information_Map.jpg", (uint8_t)MMRegions::Southern_Swamp, false, GameLayout::mm },
    { "Stone Tower", "./Resources/MM/Stone_Tower/Tower.jpg", "./Resources/MM/Stone_Tower/Tower_Map.jpg", (uint8_t)MMRegions::Stone_Tower, false, GameLayout::mm },
    { "Stone Tower, Inverted", "./Resources/MM/Stone_Tower/Tower_Inverted.jpg", "./Resources/MM/Stone_Tower/Tower_Inverted_Map.jpg", (uint8_t)MMRegions::Stone_Tower, false, GameLayout::mm },
    { "Mountain Village, Spring", "", "", (uint8_t)MMRegions::None, true, GameLayout::mm },
    { "Path to Snowhead", "./Resources/MM/Path_to_Snowhead/Path.jpg", "./Resources/MM/Path_to_Snowhead/Path_Map.jpg", (uint8_t)MMRegions::Path_to_Snowhead, true, GameLayout::mm },
    { "Snowhead", "./Resources/MM/Snowhead/Snowhead.jpg", "./Resources/MM/Snowhead/Snowhead_Map.jpg", (uint8_t)MMRegions::Snowhead, true, GameLayout::mm },
    { "Twin Islands, Winter", "", "", (uint8_t)MMRegions::None, true, GameLayout::mm },
    { "Twin Islands, Spring", "", "", (uint8_t)MMRegions::None, true, GameLayout::mm },
    { "Gyorg's Lair", "", "./Resources/MM/Dungeons/Great_Bay/Gyorg_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Secret's Shrine", "./Resources/MM/Ikana_Canyon/Shrine.jpg", "./Resources/MM/Ikana_Canyon/Shrine_Map.jpg", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm },
    { "Stock Pot Inn", "./Resources/MM/Clock_Town/Stock_Pot_Inn.jpg", "./Resources/MM/Clock_Town/Stock_Pot_Inn_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Great Bay Cutscene", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Clock Tower, Interior", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Woods of Mystery", "./Resources/MM/Woods_of_Mystery/Woods_Mystery.jpg", "./Resources/MM/Woods_of_Mystery/Woods_Mystery_Map.jpg", (uint8_t)MMRegions::Woods_of_Mystery, false, GameLayout::mm },
    { "Lost Woods", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Link's Trial", "./Resources/MM/Moon/Link.jpg", "./Resources/MM/Moon/Link_Map.jpg", (uint8_t)MMRegions::Moon, false, GameLayout::mm },
    { "Moon", "./Resources/MM/Moon/Moon.jpg", "./Resources/MM/Moon/Moon_Map.jpg", (uint8_t)MMRegions::Moon, false, GameLayout::mm },
    { "Bomb Shop", "./Resources/MM/Clock_Town/Bomb_Shop.jpg", "./Resources/MM/Clock_Town/Bomb_Shop_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Giant Chamber", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Gorman's Track", "./Resources/MM/Milk_Road/Gorman_Track.jpg", "./Resources/MM/Milk_Road/Gorman_Track_Map.jpg", (uint8_t)MMRegions::Milk_Road, false, GameLayout::mm },
    { "Goron's Racetrack", "./Resources/MM/Twin_Islands/Goron_Racetrack.jpg", "./Resources/MM/Twin_Islands/Goron_Racetrack_Map.jpg", (uint8_t)MMRegions::Twin_Islands, false, GameLayout::mm },
    { "Clock Town, East", "./Resources/MM/Clock_Town/East.jpg", "./Resources/MM/Clock_Town/East_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Clock Town, West", "./Resources/MM/Clock_Town/West.jpg", "./Resources/MM/Clock_Town/West_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Clock Town, North", "./Resources/MM/Clock_Town/North.jpg", "./Resources/MM/Clock_Town/North_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Clock Town, South", "./Resources/MM/Clock_Town/South.jpg", "./Resources/MM/Clock_Town/South_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Laundry Pool", "./Resources/MM/Clock_Town/Pool.jpg", "./Resources/MM/Clock_Town/Pool_Map.jpg", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm },
    { "Extra", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Lone Peak Shrine", "./Resources/MM/Goron_Village/Lone_Peak_Shrine.jpg", "./Resources/MM/Goron_Village/Lone_Peak_Shrine_Map.jpg", (uint8_t)MMRegions::Goron_Village, false, GameLayout::mm },
    { "Zora Shop", "./Resources/MM/Zora_Hall/Zora_Shop.jpg", "./Resources/MM/Zora_Hall/Zora_Shop_Map.jpg", (uint8_t)MMRegions::Zora_Hall, false, GameLayout::mm },
    { "Evans' Room", "./Resources/MM/Zora_Hall/Evan_Room.jpg", "./Resources/MM/Zora_Hall/Evan_Room_Map.jpg", (uint8_t)MMRegions::Zora_Hall, false, GameLayout::mm },
    { "Japas' Room", "", "./Resources/MM/Zora_Hall/Japas_Room_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Tijo's Room", "", "./Resources/MM/Zora_Hall/Tijo_Room_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Lulu's Room", "./Resources/MM/Zora_Hall/Lulu_Room.jpg", "./Resources/MM/Zora_Hall/Lulu_Room_Map.jpg", (uint8_t)MMRegions::Zora_Hall, false, GameLayout::mm },
    { "Stable", "", "./Resources/MM/Ranch/Stable_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Sewers", "", "", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Warp Owls", "", "./Resources/MM/Songs/Owls_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Dodongo Grotto", "./Resources/MM/Termina/Dodongo.jpg", "./Resources/MM/Termina/Dodongo_Map.jpg", (uint8_t)MMRegions::Termina, false, GameLayout::mm },
    { "Gossip Stones Grotto, Ocean", "./Resources/MM/Termina/Ocean_Gossip.jpg", "./Resources/MM/Termina/Gossip_Map.jpg", (uint8_t)MMRegions::Termina, false, GameLayout::mm },
    { "Gossip Stones Grotto, Canyon", "./Resources/MM/Termina/Canyon_Gossip.jpg", "./Resources/MM/Termina/Gossip_Map.jpg", (uint8_t)MMRegions::Termina, false, GameLayout::mm },
    { "Gossip Stones Grotto, Swamp", "", "./Resources/MM/Termina/Gossip_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Gossip Stones Grotto, Mountain", "", "./Resources/MM/Termina/Gossip_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Bio Baba Grotto", "./Resources/MM/Termina/Bio_Baba.jpg", "./Resources/MM/Termina/Bio_Baba_Map.jpg", (uint8_t)MMRegions::Termina, false, GameLayout::mm },
    { "Peahat Grotto", "./Resources/MM/Termina/Peahat.jpg", "./Resources/MM/Termina/Peahat_Map.jpg", (uint8_t)MMRegions::Termina, false, GameLayout::mm },
    { "Deku Scrub Grotto", "./Resources/MM/Termina/Scrub.jpg", "./Resources/MM/Termina/Scrub_Map.jpg", (uint8_t)MMRegions::Termina, false, GameLayout::mm },
    { "Tall Grass Grotto", "./Resources/MM/Grottos/Generic.jpg", "./Resources/MM/Grottos/Generic_Map.jpg", (uint8_t)MMRegions::Termina, false, GameLayout::mm },
    { "Cow Grotto", "./Resources/MM/Grottos/Cow.jpg", "./Resources/MM/Grottos/Cow_Map.jpg", (uint8_t)MMRegions::Termina, false, GameLayout::mm },
    { "Pillar Grotto", "./Resources/MM/Grottos/Generic.jpg", "./Resources/MM/Grottos/Generic_Map.jpg", (uint8_t)MMRegions::Termina, false, GameLayout::mm },
    { "Open Grotto", "./Resources/MM/Grottos/Generic.jpg", "./Resources/MM/Grottos/Generic_Map.jpg", (uint8_t)MMRegions::Great_Bay_Coast, false, GameLayout::mm },
    { "Cow Grotto", "./Resources/MM/Grottos/Cow.jpg", "./Resources/MM/Grottos/Cow_Map.jpg", (uint8_t)MMRegions::Great_Bay_Coast, false, GameLayout::mm },
    { "Generic Grotto", "./Resources/MM/Grottos/Generic.jpg", "./Resources/MM/Grottos/Generic_Map.jpg", (uint8_t)MMRegions::Zora_Cape, false, GameLayout::mm },
    { "Generic Grotto", "./Resources/MM/Grottos/Generic.jpg", "./Resources/MM/Grottos/Generic_Map.jpg", (uint8_t)MMRegions::Graveyard, false, GameLayout::mm },
    { "Open Grotto", "./Resources/MM/Grottos/Generic.jpg", "./Resources/MM/Grottos/Generic_Map.jpg", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm },
    { "Generic Grotto", "./Resources/MM/Grottos/Generic.jpg", "./Resources/MM/Grottos/Generic_Map.jpg", (uint8_t)MMRegions::Road_to_Ikana, false, GameLayout::mm },
    { "Frozen Grotto", "./Resources/MM/Twin_Islands/Frozen.jpg", "./Resources/MM/Twin_Islands/Frozen_Map.jpg", (uint8_t)MMRegions::Twin_Islands, false, GameLayout::mm },
    { "Ramp Grotto", "./Resources/MM/Grottos/Generic.jpg", "./Resources/MM/Grottos/Generic_Map.jpg", (uint8_t)MMRegions::Twin_Islands, false, GameLayout::mm },
    { "Generic Grotto", "./Resources/MM/Grottos/Generic.jpg", "./Resources/MM/Grottos/Generic_Map.jpg", (uint8_t)MMRegions::Path_to_Snowhead, false, GameLayout::mm },
    { "Generic Grotto", "./Resources/MM/Grottos/Generic.jpg", "./Resources/MM/Grottos/Generic_Map.jpg", (uint8_t)MMRegions::Mountain_Village, false, GameLayout::mm },
    { "Open Grotto", "./Resources/MM/Grottos/Generic.jpg", "./Resources/MM/Grottos/Generic_Map.jpg", (uint8_t)MMRegions::Southern_Swamp_Road, false, GameLayout::mm },
    { "Open Grotto", "./Resources/MM/Grottos/Generic.jpg", "./Resources/MM/Grottos/Generic_Map.jpg", (uint8_t)MMRegions::Southern_Swamp, false, GameLayout::mm },
    { "Open Grotto", "./Resources/MM/Grottos/Generic.jpg", "./Resources/MM/Grottos/Generic_Map.jpg", (uint8_t)MMRegions::Woods_of_Mystery, false, GameLayout::mm },
    { "Beans Grotto", "./Resources/MM/Deku_Palace/Beans.jpg", "./Resources/MM/Deku_Palace/Beans_Map.jpg", (uint8_t)MMRegions::Deku_Palace, false, GameLayout::mm },
    { "Generic Grotto", "./Resources/MM/Deku_Palace/JP_Grotto_2.jpg", "./Resources/MM/Deku_Palace/JP_Grotto_2_Map.jpg", (uint8_t)MMRegions::Deku_Palace, false, GameLayout::mm },
    { "Climb Grotto", "", "./Resources/MM/Deku_Palace/JP_Grotto_1_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm },
    { "Mountain Village", "./Resources/MM/Mountain_Village/Village.jpg", "./Resources/MM/Mountain_Village/Village_Map.jpg", (uint8_t)MMRegions::Mountain_Village, true, GameLayout::mm, "./Resources/MM/Mountain_Village/Village_Spring.jpg" },
    { "Twin Islands", "./Resources/MM/Twin_Islands/Twins.jpg", "./Resources/MM/Twin_Islands/Twins_Map.jpg", (uint8_t)MMRegions::Twin_Islands, true, GameLayout::mm },
    { "Goron's Village", "./Resources/MM/Goron_Village/Village.jpg", "./Resources/MM/Goron_Village/Village_Map.jpg", (uint8_t)MMRegions::Goron_Village, true, GameLayout::mm },
    { "Spring Water Cave", "", "./Resources/MM/Ikana_Canyon/Water_Cave_Map.jpg", (uint8_t)MMRegions::None, false, GameLayout::mm }
};

#pragma endregion

SceneMetaInfo* GetSceneMetaInfo(uint32_t SceneID, uint32_t Game)
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
				return OOT_KOKIRI_FOREST;

			case 0x01:
				return OOT_HYRULE_FIELD;

			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
				return OOT_LON_LON_RANCH;

			case 0x06:
				return OOT_KAKARIKO_VILLAGE;

			case 0x07:
				return OOT_DEATH_MOUNTAIN_TRAIL;

			case 0x08:
				return OOT_GERUDO_VALLEY;

			case 0x09:
				return OOT_INSIDE_JABU_JABU;

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
				return MM_ROMANI_RANCH;

			case 0x13:
			case 0x14:
				return MM_GROTTOS;

			case 0x15:
			case 0x16:
				return MM_GREAT_BAY_COAST;

			case 0x17:
				return MM_BENEATH_THE_WELL;

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
				return OOT_KOKIRI_SHOP;

			case OoT_Bombchu_Shop:
				return OOT_BOMBCHU_SHOP;

			case OoT_Zora_Shop:
				return OOT_ZORA_SHOP;

			case OoT_Goron_Shop:
				return OOT_GORON_SHOP;

			case OoT_Kakariko_Bazaar:
			case OoT_Market_Bazaar:
				return OOT_BAZAAR;

			case OoT_Market_Potion_Shop:
				return OOT_MARKET_POTION_SHOP;

			case OoT_Kakariko_Potion_Shop:
				return OOT_KAKARIKO_POTION_SHOP;

			default:
				return ShopID;
		}
	}
	else
	{
		if (ItemID <= 0x03)
		{	// Bomb shop
			return MM_BOMB_SHOP;
		}
		else if (ItemID == 0x04)
		{	// Curiosity shop
			return MM_CURIOSITY_SHOP;
		}
		else if (ItemID <= 0x0C)
		{	// Trading post shop
			return MM_TRADING_POST;
		}
		else if (ItemID <= 0x0f)
		{
			return MM_POTION_SHOP;
		}
		else if (ItemID <= 0x12)
		{
			return MM_GORON_SHOP;
		}
		else
		{	// Zora shop
			return MM_ZORA_HALL_ROOMS;
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
				return OOT_HYRULE_CASTLE;

			// Castle Courtyard
			case ZELDA_LETTER:
			case ZELDA_SONG:
				return OOT_CASTLE_COURTYARD;

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
				return OOT_SACRED_FOREST_MEADOW;

			// Kakariko Village
			case ANJU_BOTTLE:
			case TRADE_POCKET_EGG:
			case TRADE_COJIRO:
			case SHEIK_SHADOW:
			case KAKARIKO_ROOF_MAN:
			case MASK_SELL_KEATON:
				return OOT_KAKARIKO_VILLAGE;

			// Kakariko Potion Shop
			case TRADE_ODD_POTION:
            case WITCH_BLUE_POTION:
				return OOT_KAKARIKO_POTION_SHOP;

			// Graveyard
			case MASK_SELL_SPOOKY:
				return OOT_GRAVEYARD;

			// Tomb Royal
			case ROYAL_TOMB_SONG:
				return OOT_TOMB_ROYAL;

			// Death Mountain Trail
			case TRADE_PRESCRIPTION:
			case TRADE_CLAIM_CHECK:
			case TRADE_BIGGORON_SWORD:
				return OOT_DEATH_MOUNTAIN_TRAIL;

			// Goron City
			case DARUNIA_BRACELET:
			case GORON_LINK_TUNIC:
			case GORON_BOMB_BAG:
			case MEDIGORON:
				return OOT_GORON_CITY;

			// Death Mountain Crater
			case SHEIK_FIRE:
				return OOT_DEATH_MOUNTAIN_CRATER;

			// Zora River
			case BEAN_SELLER:
			case FROGS_STORMS:
			case FROGS_GAME:
			case FROGS_ZL:
			case FROGS_EPONA:
			case FROGS_SARIA:
			case FROGS_SUNS:
			case FROGS_SOT:
				return OOT_ZORA_RIVER;

			// Zora Domain
			case ZORA_DIVING_GAME:
			case ZORA_KING_TUNIC:
			case TRADE_EYEBALL_FROG:
				return OOT_ZORA_DOMAIN;

			// Lake Hylia
			case RUTO_LETTER:
			case FIRE_ARROW:
				return OOT_LAKE_HYLIA;

			// Laboratory
			case LABORATORY_DIVE:
			case TRADE_EYE_DROPS:
				return OOT_LABORATORY;

			// Fishing Pond
			case FISH_CHILD:
			case FISH_ADULT:
				return OOT_FISHING_POND;

			// Lon Lon Ranch
			case MALON_SONG:
				return OOT_LON_LON_RANCH;

			// Lon Lon Ranch Silo
			case TALON_MILK:
			case TALON_BOTTLE:
				return OOT_RANCH_HOUSE_SILO;

			// Hyrule Field
			case OCARINA_TIME_ITEM:
			case OCARINA_TIME_SONG:
			case MASK_SELL_BUNNY:
				return OOT_HYRULE_FIELD;

			// Back Alley
			case DOG_LADY:
				return OOT_BACK_ALLEY_HOUSE2;

			// Guard House
			case POE_COLLECTOR:
				return OOT_GUARD_HOUSE;

			// Bombchu Bowling
			case BOMBCHU_BOWLING_1:
			case BOMBCHU_BOWLING_2:
				return OOT_BOMBCHU_BOWLING_ALLEY;

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
				return OOT_TEMPLE_OF_TIME;

			// Tomb Dampe Windmill
			case OOT_SONG_STORMS:
				return OOT_TOMB_DAMPE_WINDMILL;

			// Gerudo Valley
			case TRADE_BROKEN_GORON_SWORD:
				return OOT_GERUDO_VALLEY;

			// Gerudo Forteress
			case GERUDO_ARCHERY_1:
			case GERUDO_ARCHERY_2:
				return OOT_GERUDO_FORTRESS;

			// Haunted Wasteland
			case CARPET_MERCHANT:
				return OOT_HAUNTED_WASTELAND;

			// Desert Colossus
			case SHEIK_SPIRIT:
				return OOT_DESERT_COLOSSUS;

			// House of Skulltulla
			case GS_10:
			case GS_20:
			case GS_30:
			case GS_40:
			case GS_50:
            case GS_100:
				return OOT_HOUSE_OF_SKULLTULA;

			// Great Fairy Upgrades
			case FAIRY_MAGIC_UPGRADE:
			case FAIRY_MAGIC_UPGRADE2:
			case FAIRY_DEFENSE_UPGRADE:
				return OOT_GREAT_FAIRY_FOUNTAIN_UPGRADES;

			// Great Fairy Spells
			case FAIRY_SPELL_FIRE:
			case FAIRY_SPELL_WIND:
			case FAIRY_SPELL_LOVE:
				return OOT_GREAT_FAIRY_FOUNTAIN_SPELLS;

			// Lair Gohma
			case BLUE_WARP_GOHMA:
				return OOT_LAIR_GOHMA;

			// Lair King Dodongo
			case BLUE_WARP_KING_DODONGO:
				return OOT_LAIR_KING_DODONGO;

			// Lair Barinade
			case BLUE_WARP_BARINADE:
				return OOT_LAIR_BARINADE;

			// Lair Phantom Ganon
			case BLUE_WARP_PHANTOM_GANON:
				return OOT_LAIR_PHANTOM_GANON;

			// Lair Volvagia
			case BLUE_WARP_VOLVAGIA:
				return OOT_LAIR_VOLVAGIA;

			// Lair Morpha
			case BLUE_WARP_MORPHA:
				return OOT_LAIR_MORPHA;

			// Lair Bongo Bongo
			case BLUE_WARP_BONGO_BONGO:
				return OOT_LAIR_BONGO_BONGO;

			// Lair Twinrova
			case BLUE_WARP_TWINROVA:
				return OOT_LAIR_TWINROVA;

			// Thieves Hideout
			case GERUDO_CARD:
				return OOT_THIEVES_HIDEOUT;

			// Ice Cavern
			case SHEIK_WATER:
				return OOT_ICE_CAVERN;

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
				return MM_CLOCK_TOWN_SOUTH;

			// Clock Town North
			case KEATON_HEART_PIECE:
			case MASK_BLAST:
			case BOMBER_NOTEBOOK:
				return MM_CLOCK_TOWN_NORTH;

			// Clock Town East
			case MASK_POSTMAN:
				return MM_CLOCK_TOWN_EAST;

			// Clock Town West
			case ROSA_HEART_PIECE:
			case BANK_1:
			case BANK_2:
			case BANK_3:
				return MM_CLOCK_TOWN_WEST;

			// Laundry Pool
			case STRAY_FAIRY_TOWN:
			case MASK_BREMEN:
				return MM_LAUNDRY_POOL;

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
				return MM_CLOCK_TOWER_ROOFTOP;

			// Post Office
			case POSTMAN_HEART_PIECE:
				return MM_POST_OFFICE;

			// Mayor House
			case MAYOR_HEART_PIECE:
			case MASK_KAFEI:
				return MM_MAYOR_HOUSE;

			// Milk Bar
			case MASK_TROUPE_LEADER:
			case MILK_BAR_MILK:
			case MILK_BAR_CHATEAU:
			case CHATEAU_ROMANI:
				return MM_MILK_BAR;

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
				return MM_HONEY_DARLING;

			// Stock Pot Inn
			case TOILET_HEART_PIECE:
			case GRANDMA_HEART_PIECE_1:
			case GRANDMA_HEART_PIECE_2:
			case ROOM_KEY:
			case LETTER_TO_KAFEI:
			case MASK_COUPLE:
				return MM_STOCK_POT_INN;

			// Curiosity shop
			case PENDANT_OF_MEMORIES:
			case MASK_KEATON:
			case LETTER_TO_MAMA:
				return MM_CURIOSITY_SHOP;

			// Observatory
			case MOON_TEAR:
				return MM_OBSERVATORY;

			// Swordsman school
			case SWORDSMAN_HEART_PIECE:
				return MM_SWORDSMAN_SCHOOL;

			// Deku Playground
			case DEKU_PLAYGROUND_1:
			case DEKU_PLAYGROUND_2:
				return MM_DEKU_PLAYGROUND;

			// Termina Field
			case MASK_KAMARO:
				return MM_TERMINA_FIELD;

			// Grottos
			case SCRUB_TELESCOPE:
			case GOSSIP_HEART_PIECE:
				return MM_GROTTOS;

			// Southern Swamp
			case SCRUB_SHOP_BEANS:
			case OWL_SOUTHERN_SWAMP:
			case SONG_SOARING:
			case SCRUB_SWAMP:
				return MM_SOUTHERN_SWAMP;

			// Shooting gallery Swamp
			case SHOOTING_GAME_SWAMP_1:
			case SHOOTING_GAME_SWAMP_2:
				return MM_SHOOTING_GALLERY_SWAMP;

			// Tourist information
			case KOUME_PICTOGRAPH_BOX:
			case KOUME_HEART_PIECE:
			case TOUR_GUIDE_HEART_PIECE:
				return MM_TOURIST_INFORMATION;

			// Woods of Mystery
			case KOTAKE_RED_POTION:
				return MM_WOODS_MYSTERY;

			// Deku Palace
			case SONG_AWAKENING:
				return MM_DEKU_KING_CHAMBER;

			// Deku Shrine
			case MASK_SCENTS:
				return MM_DEKU_SHRINE;

			// Woodfall
			case OWL_WOODFALL:
				return MM_WOODFALL;

			// Spider House Swamp
			case SPIDER_HOUSE_SWAMP:
				return MM_SPIDER_HOUSE_SWAMP;

			// Mountain Village Winter
			case MASK_DON_GERO:
			case FROG_HEART_PIECE:
			case OWL_MOUNTAIN_VILLAGE:
				return MM_MOUNTAIN_VILLAGE_WINTER;

			// Blacksmith
			case SWORD_RAZOR:
			case SWORD_GILDED:
				return MM_BLACKSMITH;

			// Goron Graveyard
			case MASK_GORON:
				return MM_GORON_GRAVEYARD;

			// Twin Islands Winter
			case GORON_ELDER:
				return MM_TWIN_ISLANDS_WINTER;

			// Goron Shrine
			case GORON_BABY:
				return MM_GORON_SHRINE;

			// Goron Village Winter
			case SCRUB_MOUNTAIN:
			case SCRUB_BOMB_BAG:
			case GORON_KEG:
				return MM_GORON_VILLAGE_WINTER;

			// Snowhead
			case OWL_SNOWHEAD:
				return MM_SNOWHEAD;

			// Goron Racetrack
			case GORON_RACE:
				return MM_GORON_RACETRACK;

			// Milk Road
			case OWL_MILK_ROAD:
				return MM_MILK_ROAD;

			// Romani Ranch
			case SONG_EPONA:
			case ROMANI_BOTTLE:
			case CREMIA_ESCORT:
				return MM_ROMANI_RANCH;

			// Cucco Shack
			case MASK_BUNNY:
				return MM_CUCCO_SHACK;

			// Dog Lady Racetrack
			case DOG_LADY_HEART_PIECE:
				return MM_DOG_RACETRACK;

			// Great Bay Coast
			case MASK_ZORA:
			case FISHERMAN_HEART_PIECE:
			case OWL_GREAT_BAY:
				return MM_GREAT_BAY_COAST;

			// Zora Cape
			case OWL_ZORA_CAPE:
				return MM_ZORA_CAPE;

			// Zora Hall Rooms
			case SCRUB_OCEAN:
			case SCRUB_SHOP_POTION_GREEN:
			case ZORA_EVAN:
				return MM_ZORA_HALL_ROOMS;

			// Zora Hall
			case ZORA_HALL_LIGHTS:
				return MM_ZORA_HALL;

			// Pinnacle Rock
			case SEAHORSE_HEART_PIECE:
				return MM_PINNACLE_ROCK;

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
				return MM_WATERFALL_RAPIDS;

			// Road Ikana
			case MASK_STONE:
				return MM_ROAD_IKANA;

			// Gorman Track
			case GORMAN_MILK:
			case MASK_GARO:
				return MM_GORMAN_TRACK;

			// Beneath the Graveyard
			case MM_SONG_STORMS:
				return MM_BENEATH_THE_GRAVEYARD;

			// Ikana Canyon
			case OWL_IKANA_CANYON:
			case SCRUB_VALLEY:
			case SCRUB_SHOP_POTION_BLUE:
				return MM_IKANA_CANYON;

			// Music Box House
			case MASK_GIBDO:
				return MM_MUSIC_BOX_HOUSE;

			// Ghost Hut
			case GHOST_HUT_HEART_PIECE:
				return MM_GHOST_HUT;

			// Stone Tower
			case OWL_STONE_TOWER:
				return MM_STONE_TOWER;

			// Lair Odolwa
			case SONG_ORDER:
			case REMAINS_ODOLWA:
				return MM_LAIR_ODOLWA;

			// Lair Goht
			case REMAINS_GOHT:
				return MM_LAIR_GOHT;

			// Lair Gyorg
			case REMAINS_GYORG:
				return MM_LAIR_GYORG;

			// Lair Ikana
			case SONG_EMPTINESS:
				return MM_LAIR_IKANA;

			// Lair Twinmold
			case REMAINS_TWINMOLD:
				return MM_LAIR_TWINMOLD;

			// Moon
			case MASK_FIERCE_DEITY:
			case MAJORA:
				return MM_MOON;

			default:
				return NPC;
		}
	}
}


uint32_t GetSceneSR(uint32_t SilverRupee)
{
	if (SilverRupee >= 0x00 && SilverRupee <= 0x04)
	{	// MQ Dodongo Cavern

		return OOT_DODONGO_CAVERN;
	}
	else if (SilverRupee >= 0x05 && SilverRupee <= 0x09)
	{	// Bottom of the Well

		return OOT_BOTTOM_OF_THE_WELL;
	}
	else if (SilverRupee >= 0x0a && SilverRupee <= 0x18)
	{	// Spirit Temple

		return OOT_TEMPLE_SPIRIT;
	}
	else if (SilverRupee >= 0x19 && SilverRupee <= 0x36)
	{	// Shadow Temple

		return OOT_TEMPLE_SHADOW;
	}
	else if (SilverRupee >= 0x37 && SilverRupee <= 0x40)
	{	// Ice Cavern

		return OOT_ICE_CAVERN;
	}
	else if (SilverRupee >= 0x41 && SilverRupee <= 0x50)
	{	// Gerudo Training Ground

		return OOT_GERUDO_TRAINING_GROUND;
	}
	else
	{	// Inside Ganon Castle

		return OOT_INSIDE_GANON_CASTLE;
	}
}


uint32_t GetSceneGS(uint32_t GS)
{
	if (GS >= 0x08 && GS <= 0x0b)
	{	// Deku Tree

		return OOT_DEKU_TREE;
	}
	else if (GS >= 0x10 && GS <= 0x14)
	{	// Dodongo Cavern

		return OOT_DODONGO_CAVERN;
	}
	else if (GS >= 0x18 && GS <= 0x1b)
	{	// Jabu-Jabu

		return OOT_INSIDE_JABU_JABU;
	}
	else if (GS >= 0x20 && GS <= 0x24)
	{	// Forest Temple

		return OOT_TEMPLE_FOREST;
	}
	else if (GS >= 0x28 && GS <= 0x2c)
	{	// Fire Temple

		return OOT_TEMPLE_FIRE;
	}
	else if (GS >= 0x30 && GS <= 0x34)
	{	// Water Temple

		return OOT_TEMPLE_WATER;
	}
	else if (GS >= 0x38 && GS <= 0x3c)
	{	// Spirit Temple

		return OOT_TEMPLE_SPIRIT;
	}
	else if (GS >= 0x40 && GS <= 0x44)
	{	// Shadow Temple

		return OOT_TEMPLE_SHADOW;
	}
	else if (GS >= 0x48 && GS <= 0x4a)
	{	// Bottom of the Well

		return OOT_BOTTOM_OF_THE_WELL;
	}
	else if (GS >= 0x50 && GS <= 0x52)
	{	// Ice Cavern

		return OOT_ICE_CAVERN;
	}
	else if (GS == 0x58 || GS == 0x59 || GS == 0x79)
	{	// Grottos

		return OOT_GROTTOS;
	}
	else if (GS >= 0x60 && GS <= 0x63)
	{	// Lon Lon Ranch

		return OOT_LON_LON_RANCH;
	}
	else if (GS >= 0x68 && GS <= 0x6a)
	{	// Kokiri Forest

		return OOT_KOKIRI_FOREST;
	}
	else if (GS >= 0x70 && GS <= 0x72)
	{	// Lost Woods

		return OOT_LOST_WOODS;
	}
	else if (GS == 0x73)
	{	// Sacred Forest Meadow

		return OOT_SACRED_FOREST_MEADOW;
	}
	else if (GS == 0x78)
	{	// Ganon Castle Exterior

		return OOT_GANON_CASTLE_EXTERIOR;
	}
	else if (GS == 0x7a)
	{	// Hyrule Castle

		return OOT_HYRULE_CASTLE;
	}
	else if (GS == 0x7b)
	{	// Guard House

		return OOT_GUARD_HOUSE;
	}
	else if (GS == 0x80 || GS == 0x87)
	{	// Death Mountain Crater

		return OOT_DEATH_MOUNTAIN_CRATER;
	}
	else if (GS >= 0x81 && GS <= 0x84)
	{	// Death Mountain Trail

		return OOT_DEATH_MOUNTAIN_TRAIL;
	}
	else if (GS == 0x85 || GS == 0x86)
	{	// Goron City

		return OOT_GORON_CITY;
	}
	else if (GS == 0x88 || GS == 0x8f)
	{	// Graveyard

		return OOT_GRAVEYARD;
	}
	else if (GS >= 0x89 && GS <= 0x8e)
	{	// Kakariko Village

		return OOT_KAKARIKO_VILLAGE;
	}
	else if (GS == 0x90 || GS == 0x91 || GS == 0x93 || GS == 0x94)
	{	// Zora River

		return OOT_ZORA_RIVER;
	}
	else if (GS == 0x92 || GS == 0x95 || GS == 0x97)
	{	// Zora Fountain

		return OOT_ZORA_FOUNTAIN;
	}
	else if (GS == 0x96)
	{	// Zora Domain

		return OOT_ZORA_DOMAIN;
	}
	else if ((GS >= 0x98 && GS <= 0x9A) || GS == 0x9c)
	{	// Lake Hylia

		return OOT_LAKE_HYLIA;
	}
	else if (GS == 0x9b)
	{	// Laboratory

		return OOT_LABORATORY;
	}
	else if (GS >= 0xa0 && GS <= 0xa3)
	{	// Gerudo Valley

		return OOT_GERUDO_VALLEY;
	}
	else if (GS >= 0xa8 && GS <= 0xa9)
	{	// Gerudo Fortress

		return OOT_GERUDO_FORTRESS;
	}
	else if (GS == 0xb0 || GS == 0xb2 || GS == 0xb3)
	{	// Desert Colossus

		return OOT_DESERT_COLOSSUS;
	}
	else
	{	// Haunted Wasteland

		return OOT_HAUNTED_WASTELAND;
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

		return OOT_DEATH_MOUNTAIN_CRATER;
	}
	else if (Scrub == 0x1b)
	{	// MQ Deku Tree

		return OOT_DEKU_TREE;
	}
	else if (Scrub >= 0x1c && Scrub <= 0x1f)
	{	// Dodongo Cavern

		return OOT_DODONGO_CAVERN;
	}
	else if (Scrub == 0x20)
	{	// Jabu-Jabu

		return OOT_INSIDE_JABU_JABU;
	}
	else
	{	// Inside Ganon Castle

		return OOT_INSIDE_GANON_CASTLE;
	}
}

const char* GetSceneName(int Game, uint32_t SceneID)
{
	if (Game == OOT_GAME)
	{
		return OoTScenesMetaInfo[SceneID].Name;
	}
	else
	{
		return MMScenesMetaInfo[SceneID].Name;
	}
}

const char* GetSceneMiniMap(int Game, uint32_t SceneID)
{
    if (Game == OOT_GAME)
    {
        return OoTScenesMetaInfo[SceneID].MiniMapPath;
    }
    else
    {
        return MMScenesMetaInfo[SceneID].MiniMapPath;
    }
}
