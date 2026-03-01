#include "Combo/Scenes.h"
#include "Combo/NPC.h"
#include "Combo/Objects.h"
#include "Multi/Game.h"

#pragma region Scenes

SceneMetaInfo OoTScenesMetaInfo[OOT_NUM_SCENES] =
{
	{ "Deku Tree", "", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot},
	{ "Dodongo's Cavern", "", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot},
	{ "Inside Jabu-Jabu", "", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot},
	{ "Forest Temple", "", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot},
	{ "Fire Temple", "", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot},
	{ "Water Temple", "", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot},
	{ "Spirit Temple", "", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot},
	{ "Shadow Temple", "", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot},
	{ "Bottom of the Well", "", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot},
	{ "Ice Cavern", "./Resources/OoT/Dungeons/Ice/ICE.jpg", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot},
	{ "Ganon Tower", "", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot},
	{ "Gerudo Training Ground", "./Resources/OoT/Dungeons/GTG/GTG.jpg", (uint8_t)OoTRegions::Gerudo_Fortress, false, GameLayout::oot},
	{ "Thieves Hideout", "./Resources/OoT/Gerudo_Fortress/Hideout.png", (uint8_t)OoTRegions::Gerudo_Fortress, false, GameLayout::oot},
	{ "Inside Ganon Castle", "", (uint8_t)OoTRegions::Dungeons, false, GameLayout::oot},
	{ "Ganon Tower - Collapsing", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Inside Ganon Castle - Collapsing", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Treasure Shop", "./Resources/OoT/Market/Treasure_Game.png", (uint8_t)OoTRegions::Market, false, GameLayout::oot},
	{ "Gohma's Lair", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "King Dodongo's Lair", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Barinade's Lair", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Phantom Ganon's Lair", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Volvagia's Lair", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Morpha's Lair", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Twinrova's Lair", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Bongo-Bongo's Lair", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Ganondorf's Lair", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Tower Collapse - Exterior", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Market Entrance - Child Day", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Market Entrance - Child Night", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Market Entrance - Adult", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Back Alley - Day", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Back Alley - Night", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Market - Day", "./Resources/OoT/Market/Market_Day.png", (uint8_t)OoTRegions::Market, false, GameLayout::oot},
	{ "Market - Night", "./Resources/OoT/Market/Market_Night.png", (uint8_t)OoTRegions::Market, false, GameLayout::oot},
	{ "Market - Adult", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Temple of Time Exterior Child Day", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Temple of Time Exterior Child Night", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Temple of Time Exterior Adult", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Know-It-All Brothers's House", "./Resources/OoT/Kokiri_Forest/Know_It_All.png", (uint8_t)OoTRegions::Kokiri_Forest, false, GameLayout::oot},
	{ "Twins's House", "./Resources/OoT/Kokiri_Forest/Twins_House.png", (uint8_t)OoTRegions::Kokiri_Forest, false, GameLayout::oot},
	{ "Mido's House", "./Resources/OoT/Kokiri_Forest/Mido_House.png", (uint8_t)OoTRegions::Kokiri_Forest, false, GameLayout::oot},
	{ "Saria's House", "./Resources/OoT/Kokiri_Forest/Saria_House.png", (uint8_t)OoTRegions::Kokiri_Forest, false, GameLayout::oot},
	{ "Carpenters Boss's House", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Back Alley's House", "./Resources/OoT/Market/Back_Alley_House.png", (uint8_t)OoTRegions::Market, false, GameLayout::oot},
	{ "Bazaar", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Shop", "./Resources/OoT/Kokiri_Forest/Shop.png", (uint8_t)OoTRegions::Kokiri_Forest, false, GameLayout::oot},
	{ "Shop", "./Resources/OoT/Goron_City/Shop.png", (uint8_t)OoTRegions::Goron_City, false, GameLayout::oot},
	{ "Shop", "./Resources/OoT/Zora_Domain/Shop.png", (uint8_t)OoTRegions::Zora_Domain, false, GameLayout::oot},
	{ "Potion Shop", "./Resources/OoT/Kakariko/Potion_Shop.png", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot},
	{ "Potion Shop", "./Resources/OoT/Market/Potion_Shop.png", (uint8_t)OoTRegions::Market, false, GameLayout::oot},
	{ "Bombchu Shop", "./Resources/OoT/Market/Bombchu_Shop.png", (uint8_t)OoTRegions::Market, false, GameLayout::oot},
	{ "Happy Mask Shop", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Link's House", "./Resources/OoT/Kokiri_Forest/Link_House.png", (uint8_t)OoTRegions::Kokiri_Forest, true, GameLayout::oot},
	{ "Dog Lady's House", "./Resources/OoT/Market/Dog_Lady_House.png", (uint8_t)OoTRegions::Market, false, GameLayout::oot},
	{ "Stable", "./Resources/OoT/Ranch/Stable.png", (uint8_t)OoTRegions::Lon_Lon_Ranch, false, GameLayout::oot},
	{ "Impa's House", "./Resources/OoT/Kakariko/Impa_House.png", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot},
	{ "Laboratory", "./Resources/OoT/Lake_Hylia/Laboratory.png", (uint8_t)OoTRegions::Lake_Hylia, false, GameLayout::oot},
	{ "Carpenter's Tent", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Gravekeeper's Hut", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Great Fairy Fountain - Upgrades", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Fairy Fountain", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Great Fairy Fountain - Spells", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Grottos", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Redead's Tomb", "./Resources/OoT/Graveyard/Redead_Tomb.png", (uint8_t)OoTRegions::Graveyard, false, GameLayout::oot},
	{ "Fairy's Fountain Tomb", "./Resources/OoT/Graveyard/Fairy_Fountain_Tomb.png", (uint8_t)OoTRegions::Graveyard, false, GameLayout::oot},
	{ "Royal's Family Tomb", "./Resources/OoT/Graveyard/Royal_Tomb.png", (uint8_t)OoTRegions::Graveyard, false, GameLayout::oot},
	{ "Shooting Gallery", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Temple of Time", "./Resources/OoT/Market/Temple_of_Time.png", (uint8_t)OoTRegions::Market, false, GameLayout::oot},
	{ "Chamber of the Sages", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Castle Maze - Day", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Castle Maze - Night", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Cutscene Map", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Dampe's Tomb", "./Resources/OoT/Graveyard/Dampe_Tomb.png", (uint8_t)OoTRegions::Graveyard, false, GameLayout::oot},
	{ "Fishing Pond", "./Resources/OoT/Lake_Hylia/Pond.png", (uint8_t)OoTRegions::Lake_Hylia, true, GameLayout::oot},
	{ "Castle Courtyard", "./Resources/OoT/Hyrule/Castle_Courtyard.png", (uint8_t)OoTRegions::Castle, false, GameLayout::oot},
	{ "Bombchu Bowling", "./Resources/OoT/Market/Bombchu_Bowling.png", (uint8_t)OoTRegions::Market, false, GameLayout::oot},
	{ "Talon's House", "./Resources/OoT/Ranch/Ranch_House_Silo.png", (uint8_t)OoTRegions::Lon_Lon_Ranch, true, GameLayout::oot},
	{ "Pot's House", "./Resources/OoT/Market/Pot_House.png", (uint8_t)OoTRegions::Market, true, GameLayout::oot},
	{ "Granny Potion Shop", "./Resources/OoT/Kakariko/Granny_Potion_Shop.png", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot},
	{ "Ganon Battle Arena", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "House of Skulltula", "./Resources/OoT/Kakariko/House_Skulltula.png", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot},
	{ "Hyrule Field", "./Resources/OoT/Hyrule/Hyrule_Field.png", (uint8_t)OoTRegions::Hyrule, true, GameLayout::oot},
	{ "Kakariko's Village", "./Resources/OoT/Kakariko/Kakariko.png", (uint8_t)OoTRegions::Kakariko, true, GameLayout::oot},
	{ "Graveyard", "./Resources/OoT/Graveyard/Graveyard.png", (uint8_t)OoTRegions::Graveyard, true, GameLayout::oot},
	{ "Zora's River", "./Resources/OoT/Zora_River/Zora_River.png", (uint8_t)OoTRegions::Zora_River, true, GameLayout::oot},
	{ "Kokiri Forest", "./Resources/OoT/Kokiri_Forest/Kokiri_Forest.png", (uint8_t)OoTRegions::Kokiri_Forest, true, GameLayout::oot},
	{ "Sacred Forest Meadow", "./Resources/OoT/Sacred_Forest_Meadow/Sacred_Forest_Meadow.png", (uint8_t)OoTRegions::Sacred_Forest_Meadow, true, GameLayout::oot},
	{ "Lake Hylia", "./Resources/OoT/Lake_Hylia/Lake_Hylia.png", (uint8_t)OoTRegions::Lake_Hylia, true, GameLayout::oot},
	{ "Zora's Domain", "./Resources/OoT/Zora_Domain/Zora_Domain.png", (uint8_t)OoTRegions::Zora_Domain, true, GameLayout::oot},
	{ "Zora's Fountain", "./Resources/OoT/Zora_Fountain/Zora_Fountain.png", (uint8_t)OoTRegions::Zora_Fountain, true, GameLayout::oot},
	{ "Gerudo Valley", "./Resources/OoT/Gerudo_Valley/Valley.png", (uint8_t)OoTRegions::Gerudo_Valley, true, GameLayout::oot},
	{ "Lost Woods", "./Resources/OoT/Lost_Woods/Lost_Woods.png", (uint8_t)OoTRegions::Lost_Woods, true, GameLayout::oot},
	{ "Desert Colossus", "./Resources/OoT/Desert_Colossus/Desert_Colossus.png", (uint8_t)OoTRegions::Desert_Colossus, true, GameLayout::oot},
	{ "Gerudo's Fortress", "./Resources/OoT/Gerudo_Fortress/Fortress.png", (uint8_t)OoTRegions::Gerudo_Fortress, true, GameLayout::oot},
	{ "Haunted Wasteland", "./Resources/OoT/Haunted_Wasteland/Haunted_Wasteland.png", (uint8_t)OoTRegions::Haunted_Wasteland, false, GameLayout::oot},
	{ "Hyrule Castle", "./Resources/OoT/Hyrule/Hyrule_Castle.png", (uint8_t)OoTRegions::Castle, false, GameLayout::oot},
	{ "Death Mountain Trail", "", (uint8_t)OoTRegions::Death_Mountain_Trail, true, GameLayout::oot},
	{ "Death Mountain Crater", "./Resources/OoT/Death_Mountain_Crater/Crater.png", (uint8_t)OoTRegions::Death_Mountain_Crater, true, GameLayout::oot},
	{ "Goron City", "./Resources/OoT/Goron_City/Goron_City.png", (uint8_t)OoTRegions::Goron_City, true, GameLayout::oot},
	{ "Lon Lon's Ranch", "./Resources/OoT/Ranch/Lon_Lon_Ranch.png", (uint8_t)OoTRegions::Lon_Lon_Ranch, true, GameLayout::oot},
	{ "Ganon Castle Exterior", "./Resources/OoT/Ganon/Exterior.png", (uint8_t)OoTRegions::Castle, false, GameLayout::oot},
	{ "Song of Storms Grotto", "./Resources/OoT/Kokiri_Forest/Storms.png", (uint8_t)OoTRegions::Kokiri_Forest, false, GameLayout::oot},
	{ "Deku Scrubs Grotto", "./Resources/OoT/Lost_Woods/Scrub_Upgrade.png", (uint8_t)OoTRegions::Lost_Woods, false, GameLayout::oot},
	{ "Generic Grotto", "./Resources/OoT/Lost_Woods/Generic.png", (uint8_t)OoTRegions::Lost_Woods, false, GameLayout::oot},
	{ "Deku's Theater Grotto", "./Resources/OoT/Lost_Woods/Theater.png", (uint8_t)OoTRegions::Lost_Woods, false, GameLayout::oot},
	{ "Wolfos Grotto", "./Resources/OoT/Sacred_Forest_Meadow/Wolfos.png", (uint8_t)OoTRegions::Sacred_Forest_Meadow, false, GameLayout::oot},
	{ "Song of Storms Grotto", "./Resources/OoT/Sacred_Forest_Meadow/Storms.png", (uint8_t)OoTRegions::Sacred_Forest_Meadow, false, GameLayout::oot},
	{ "Redead Grotto", "./Resources/OoT/Kakariko/Redead.png", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot},
	{ "Open Grotto", "./Resources/OoT/Kakariko/Open.png", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot},
	{ "Song of Storms Grotto", "./Resources/OoT/Death_Mountain_Trail/Storms.png", (uint8_t)OoTRegions::Death_Mountain_Trail, false, GameLayout::oot},
	{ "Cow Grotto", "./Resources/OoT/Death_Mountain_Trail/Cow.png", (uint8_t)OoTRegions::Death_Mountain_Trail, false, GameLayout::oot},
	{ "Deku Scrubs Grotto", "./Resources/OoT/Goron_City/Scrubs.png", (uint8_t)OoTRegions::Goron_City, false, GameLayout::oot},
	{ "Generic Grotto", "./Resources/OoT/Death_Mountain_Crater/Generic.png", (uint8_t)OoTRegions::Death_Mountain_Crater, false, GameLayout::oot},
	{ "Deku Scrubs Grotto", "./Resources/OoT/Death_Mountain_Crater/Scrubs.png", (uint8_t)OoTRegions::Death_Mountain_Crater, false, GameLayout::oot},
	{ "Song of Storms Grotto", "./Resources/OoT/Zora_River/Storms.png", (uint8_t)OoTRegions::Zora_River, false, GameLayout::oot},
	{ "Generic Grotto", "./Resources/OoT/Zora_River/Generic.png", (uint8_t)OoTRegions::Zora_River, false, GameLayout::oot},
	{ "Deku Scrubs Grotto", "./Resources/OoT/Lake_Hylia/Scrubs.png", (uint8_t)OoTRegions::Lake_Hylia, false, GameLayout::oot},
	{ "Deku Scrubs Grotto", "./Resources/OoT/Ranch/Scrubs.png", (uint8_t)OoTRegions::Lon_Lon_Ranch, false, GameLayout::oot},
	{ "Deku Scrubs Grotto", "./Resources/OoT/Hyrule/Field_Scrubs.png", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot},
	{ "Southeast Grotto", "./Resources/OoT/Hyrule/Field_SE.png", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot},
	{ "Open Grotto", "./Resources/OoT/Hyrule/Field_Open.png", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot},
	{ "Market Side Grotto", "./Resources/OoT/Hyrule/Field_Market.png", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot},
	{ "Tektite Grotto", "./Resources/OoT/Hyrule/Field_Tektite.png", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot},
	{ "Kakariko Side Grotto", "./Resources/OoT/Hyrule/Field_Kakariko.png", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot},
	{ "Gerudo Side Grotto", "./Resources/OoT/Hyrule/Field_Gerudo.png", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot},
	{ "Song of Storms Grotto", "./Resources/OoT/Hyrule/Castle_Storms.png", (uint8_t)OoTRegions::Castle, false, GameLayout::oot},
	{ "Song of Storms Grotto", "./Resources/OoT/Gerudo_Valley/Storms.png", (uint8_t)OoTRegions::Gerudo_Valley, false, GameLayout::oot},
	{ "Octorok Grotto", "./Resources/OoT/Gerudo_Valley/Octorok.png", (uint8_t)OoTRegions::Gerudo_Valley, false, GameLayout::oot},
	{ "Deku Scrubs Grotto", "./Resources/OoT/Desert_Colossus/Scrubs.png", (uint8_t)OoTRegions::Desert_Colossus, false, GameLayout::oot},
	{ "Fairy Fountain", "./Resources/OoT/Fairy/Fountain.png", (uint8_t)OoTRegions::Sacred_Forest_Meadow, false, GameLayout::oot},
	{ "Fairy Fountain", "./Resources/OoT/Fairy/Fountain.png", (uint8_t)OoTRegions::Zora_River, false, GameLayout::oot},
	{ "Fairy Fountain", "./Resources/OoT/Fairy/Fountain.png", (uint8_t)OoTRegions::Zora_Domain, false, GameLayout::oot},
	{ "Fairy Fountain", "./Resources/OoT/Fairy/Fountain.png", (uint8_t)OoTRegions::Hyrule, false, GameLayout::oot},
	{ "Fairy Fountain", "./Resources/OoT/Fairy/Fountain.png", (uint8_t)OoTRegions::Gerudo_Fortress, false, GameLayout::oot},
	{ "Great Fairy Fountain", "./Resources/OoT/Fairy/Great_Fairy.png", (uint8_t)OoTRegions::Castle, true, GameLayout::oot},
	{ "Great Fairy Fountain", "./Resources/OoT/Fairy/Great_Fairy.png", (uint8_t)OoTRegions::Zora_Fountain, false, GameLayout::oot},
	{ "Great Fairy Fountain", "./Resources/OoT/Fairy/Great_Fairy.png", (uint8_t)OoTRegions::Desert_Colossus, false, GameLayout::oot},
	{ "Great Fairy Fountain", "./Resources/OoT/Fairy/Great_Fairy.png", (uint8_t)OoTRegions::Death_Mountain_Trail, false, GameLayout::oot},
	{ "Great Fairy Fountain", "./Resources/OoT/Fairy/Great_Fairy.png", (uint8_t)OoTRegions::Death_Mountain_Crater, false, GameLayout::oot},
	{ "Bazaar", "./Resources/OoT/Kakariko/Bazaar.png", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot},
	{ "Bazaar", "./Resources/OoT/Market/Bazaar.png", (uint8_t)OoTRegions::Market, false, GameLayout::oot},
	{ "Shooting Gallery", "./Resources/OoT/Kakariko/Shooting.png", (uint8_t)OoTRegions::Kakariko, false, GameLayout::oot},
	{ "Shooting Gallery", "./Resources/OoT/Market/Shooting.png", (uint8_t)OoTRegions::Market, false, GameLayout::oot},
	{ "Silo", "./Resources/OoT/Ranch/Silo.png", (uint8_t)OoTRegions::Lon_Lon_Ranch, false, GameLayout::oot},
	{ "Windmill", "./Resources/OoT/Kakariko/Windmill.png", (uint8_t)OoTRegions::Kakariko, true, GameLayout::oot},
	{ "Inside Eggs", "", (uint8_t)OoTRegions::None, false, GameLayout::oot},
	{ "Market", "", (uint8_t)OoTRegions::None, false, GameLayout::oot}
};


SceneMetaInfo MMScenesMetaInfo[MM_NUM_SCENES] =
{
	{ "Southern Swamp - Clear", " ", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Fairy Fountain", "./Resources/MM/Snowhead/Fairy.png", (uint8_t)MMRegions::Snowhead, false, GameLayout::mm},
	{ "Fairy Fountain", "./Resources/MM/Zora_Cape/Fairy.png", (uint8_t)MMRegions::Zora_Cape, false, GameLayout::mm},
	{ "Fairy Fountain", "./Resources/MM/Woodfall/Fairy.png", (uint8_t)MMRegions::Woodfall, false, GameLayout::mm},
	{ "Fairy Fountain", "./Resources/MM/Clock_Town/Fairy.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Fairy Fountain", "./Resources/MM/Ikana_Canyon/Fairy.png", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm},
	{ "Path to Snowhead, Spring", "", (uint8_t)MMRegions::None, true, GameLayout::mm},
	{ "Grottos", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Cutscene Map", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Path to Mountain Village, Spring", "", (uint8_t)MMRegions::None, true, GameLayout::mm},
	{ "Potion Shop", "./Resources/MM/Southern_Swamp/Potion_Shop.png", (uint8_t)MMRegions::Southern_Swamp, false, GameLayout::mm},
	{ "Majora's Lair", "./Resources/MM/Moon/Majora.png", (uint8_t)MMRegions::Moon, false, GameLayout::mm},
	{ "Beneath the Graveyard", "./Resources/MM/Graveyard/Beneath_Graveyard.png", (uint8_t)MMRegions::Graveyard, false, GameLayout::mm},
	{ "Curiosity Shop", "./Resources/MM/Clock_Town/Curiosity_Shop.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Beneath the Graveyard, Night 1", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Beneath the Graveyard, Night 2", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Ranch Barn", "./Resources/MM/Ranch/Stable.png", (uint8_t)MMRegions::Ranch, false, GameLayout::mm},
	{ "Honey & Darling", "./Resources/MM/Clock_Town/Honey_Darling.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Town Hall", "./Resources/MM/Clock_Town/Town_Hall.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Ikana Canyon", "./Resources/MM/Ikana_Canyon/Ikana_Canyon.png", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm},
	{ "Pirate's Fortress, Exterior", "./Resources/MM/Fortress/Exterior.png", (uint8_t)MMRegions::Fortress, false, GameLayout::mm},
	{ "Milk Bar", "./Resources/MM/Clock_Town/Milk_Bar.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Stone Tower Temple", "", (uint8_t)MMRegions::Dungeons, false, GameLayout::mm},
	{ "Treasure Shop", "./Resources/MM/Clock_Town/Treasure.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Stone Tower Temple, Inverted", "", (uint8_t)MMRegions::Dungeons, false, GameLayout::mm},
	{ "Clock Tower Rooftop", "./Resources/MM/Clock_Town/Tower_Rooftop.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Opening", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Woodfall Temple", "", (uint8_t)MMRegions::Dungeons, false, GameLayout::mm},
	{ "Path to Mountain Village", "./Resources/MM/Path_to_Mountain_Village/Path.png", (uint8_t)MMRegions::Path_to_Mountain_Village, true, GameLayout::mm},
	{ "Ancient Ikana Castle", "", (uint8_t)MMRegions::Dungeons, false, GameLayout::mm},
	{ "Deku's Playground", "./Resources/MM/Clock_Town/Deku_Playground.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Odolwa's Lair", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Shooting Gallery", "./Resources/MM/Clock_Town/Shooting.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Snowhead Temple", "", (uint8_t)MMRegions::Dungeons, false, GameLayout::mm},
	{ "Milk Road", "./Resources/MM/Milk_Road/Milk_Road.png", (uint8_t)MMRegions::Milk_Road, false, GameLayout::mm},
	{ "Pirate's Fortress, Interior", "./Resources/MM/Fortress/Interior.png", (uint8_t)MMRegions::Fortress, false, GameLayout::mm},
	{ "Shooting Gallery", "./Resources/MM/Road_to_Southern_Swamp/Shooting.png", (uint8_t)MMRegions::Southern_Swamp_Road, false, GameLayout::mm},
	{ "Pinnacle Rock", "./Resources/MM/Great_Bay_Coast/Rock.png", (uint8_t)MMRegions::Great_Bay_Coast, false, GameLayout::mm},
	{ "Fairy Fountain", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Spider's House", "./Resources/MM/Southern_Swamp/Spider_House.png", (uint8_t)MMRegions::Southern_Swamp, false, GameLayout::mm},
	{ "Spider's House", "./Resources/MM/Great_Bay_Coast/Spider_House.png", (uint8_t)MMRegions::Great_Bay_Coast, false, GameLayout::mm},
	{ "Observatory", "./Resources/MM/Clock_Town/Observatory.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Deku's Trial", "./Resources/MM/Moon/Deku.png", (uint8_t)MMRegions::Moon, false, GameLayout::mm},
	{ "Deku's Palace", "./Resources/MM/Deku_Palace/Outside.png", (uint8_t)MMRegions::Deku_Palace, false, GameLayout::mm},
	{ "Blacksmith", "./Resources/MM/Mountain_Village/Blacksmith.png", (uint8_t)MMRegions::Mountain_Village, false, GameLayout::mm},
	{ "Termina Field", "./Resources/MM/Termina/Field.png", (uint8_t)MMRegions::Termina, false, GameLayout::mm},
	{ "Post Office", "./Resources/MM/Clock_Town/Post_Office.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Laboratory", "./Resources/MM/Great_Bay_Coast/Laboratory.png", (uint8_t)MMRegions::Great_Bay_Coast, false, GameLayout::mm},
	{ "Dampe's House", "./Resources/MM/Graveyard/Dampe.png", (uint8_t)MMRegions::Graveyard, false, GameLayout::mm},
	{ "Inside Ancient Ikana Castle", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Goron's Shrine", "./Resources/MM/Goron_Village/City.png", (uint8_t)MMRegions::Goron_Village, false, GameLayout::mm},
	{ "Zora's Theater", "./Resources/MM/Zora_Hall/Zora_Hall.png", (uint8_t)MMRegions::Zora_Hall, false, GameLayout::mm},
	{ "Trading Post", "./Resources/MM/Clock_Town/Trading_Post.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Romani's Ranch", "./Resources/MM/Ranch/Romani_Ranch.png", (uint8_t)MMRegions::Ranch, false, GameLayout::mm},
	{ "Twinmold Lair", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Great Bay Coast", "./Resources/MM/Great_Bay_Coast/Great_Bay.png", (uint8_t)MMRegions::Great_Bay_Coast, false, GameLayout::mm},
	{ "Zora Cape", "./Resources/MM/Zora_Cape/Cape.png", (uint8_t)MMRegions::Zora_Cape, false, GameLayout::mm},
	{ "Lottery", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Snowhead, Spring", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Pirate's Fortress, Entrance", "./Resources/MM/Fortress/Entrance.png", (uint8_t)MMRegions::Fortress, false, GameLayout::mm},
	{ "Fisherman's Hut", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Shop", "./Resources/MM/Goron_Village/Shop.png", (uint8_t)MMRegions::Goron_Village, false, GameLayout::mm},
	{ "Deku King's Chamber", "./Resources/MM/Deku_Palace/King.png", (uint8_t)MMRegions::Deku_Palace, false, GameLayout::mm},
	{ "Goron's Trial", "", (uint8_t)MMRegions::Moon, false, GameLayout::mm},
	{ "Road to Southern Swamp", "./Resources/MM/Road_to_Southern_Swamp/Road.png", (uint8_t)MMRegions::Southern_Swamp_Road, false, GameLayout::mm},
	{ "Dog's Racetrack", "./Resources/MM/Ranch/Dog_Race.png", (uint8_t)MMRegions::Ranch, false, GameLayout::mm},
	{ "Cucco's Shack", "./Resources/MM/Ranch/Shack.png", (uint8_t)MMRegions::Ranch, false, GameLayout::mm},
	{ "Ikana Graveyard", "./Resources/MM/Graveyard/Graveyard.png", (uint8_t)MMRegions::Graveyard, false, GameLayout::mm},
	{ "Goht's Lair", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Southern Swamp", "./Resources/MM/Southern_Swamp/Swamp.png", (uint8_t)MMRegions::Southern_Swamp, false, GameLayout::mm},
	{ "Woodfall", "./Resources/MM/Woodfall/Woodfall.png", (uint8_t)MMRegions::Woodfall, false, GameLayout::mm},
	{ "Zora's Trial", "./Resources/MM/Moon/Zora.png", (uint8_t)MMRegions::Moon, false, GameLayout::mm},
	{ "Goron's Village, Spring", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Great Bay Temple", "", (uint8_t)MMRegions::Dungeons, false, GameLayout::mm},
	{ "Waterfall Rapids", "./Resources/MM/Zora_Cape/Waterfall_Rapids.png", (uint8_t)MMRegions::Zora_Cape, false, GameLayout::mm},
	{ "Beneath the Well", "./Resources/MM/Ikana_Canyon/Beneath_Well.png", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm},
	{ "Lulu's Room", "./Resources/MM/Zora_Hall/Lulu_Room.png", (uint8_t)MMRegions::Zora_Hall, false, GameLayout::mm},
	{ "Goron's Village", "./Resources/MM/Goron_Village/Village.png", (uint8_t)MMRegions::Goron_Village, false, GameLayout::mm},
	{ "Goron's Graveyard", "./Resources/MM/Mountain_Village/Graveyard.png", (uint8_t)MMRegions::Mountain_Village, false, GameLayout::mm},
	{ "Sakon's Hideout", "./Resources/MM/Ikana_Canyon/Sakon_Hideout.png", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm},
	{ "Mountain Village, Winter", "", (uint8_t)MMRegions::None, true, GameLayout::mm},
	{ "Ghost Hut", "./Resources/MM/Ikana_Canyon/Ghost_Hut.png", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm},
	{ "Deku's Shrine", "./Resources/MM/Deku_Palace/Shrine.png", (uint8_t)MMRegions::Deku_Palace, false, GameLayout::mm},
	{ "Road to Ikana", "./Resources/MM/Road_to_Ikana/Road.png", (uint8_t)MMRegions::Road_to_Ikana, false, GameLayout::mm},
	{ "Swordsman's School", "./Resources/MM/Clock_Town/Swordsman_School.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Music Box House", "./Resources/MM/Ikana_Canyon/Music_Box_House.png", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm},
	{ "Ikana's Lair", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Tourist Information", "./Resources/MM/Southern_Swamp/Tourist_Information.png", (uint8_t)MMRegions::Southern_Swamp, false, GameLayout::mm},
	{ "Stone Tower", "./Resources/MM/Stone_Tower/Tower.png", (uint8_t)MMRegions::Stone_Tower, false, GameLayout::mm},
	{ "Stone Tower, Inverted", "./Resources/MM/Stone_Tower/Tower_Inverted.png", (uint8_t)MMRegions::Stone_Tower, false, GameLayout::mm},
	{ "Mountain Village, Spring", "", (uint8_t)MMRegions::None, true, GameLayout::mm},
	{ "Path to Snowhead", "./Resources/MM/Path_to_Snowhead/Path.png", (uint8_t)MMRegions::Path_to_Snowhead, true, GameLayout::mm},
	{ "Snowhead", "./Resources/MM/Snowhead/Snowhead.png", (uint8_t)MMRegions::Snowhead, true, GameLayout::mm},
	{ "Twin Islands, Winter", "", (uint8_t)MMRegions::None, true, GameLayout::mm},
	{ "Twin Islands, Spring", "", (uint8_t)MMRegions::None, true, GameLayout::mm},
	{ "Gyorg's Lair", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Secret's Shrine", "./Resources/MM/Ikana_Canyon/Shrine.png", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm},
	{ "Stock Pot Inn", "./Resources/MM/Clock_Town/Stock_Pot_Inn.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Great Bay Cutscene", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Clock Tower, Interior", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Woods of Mystery", "./Resources/MM/Woods_of_Mystery/Woods_Mystery.png", (uint8_t)MMRegions::Woods_of_Mystery, false, GameLayout::mm},
	{ "Lost Woods", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Link's Trial", "./Resources/MM/Moon/Link.png", (uint8_t)MMRegions::Moon, false, GameLayout::mm},
	{ "Moon", "./Resources/MM/Moon/Moon.png", (uint8_t)MMRegions::Moon, false, GameLayout::mm},
	{ "Bomb Shop", "./Resources/MM/Clock_Town/Bomb_Shop.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Giant Chamber", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Gorman's Track", "./Resources/MM/Milk_Road/Gorman_Track.png", (uint8_t)MMRegions::Milk_Road, false, GameLayout::mm},
	{ "Goron's Racetrack", "./Resources/MM/Twin_Islands/Goron_Racetrack.png", (uint8_t)MMRegions::Twin_Islands, false, GameLayout::mm},
	{ "Clock Town, East", "./Resources/MM/Clock_Town/East.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Clock Town, West", "./Resources/MM/Clock_Town/West.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Clock Town, North", "./Resources/MM/Clock_Town/North.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Clock Town, South", "./Resources/MM/Clock_Town/South.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Laundry Pool", "./Resources/MM/Clock_Town/Pool.png", (uint8_t)MMRegions::Clock_Town, false, GameLayout::mm},
	{ "Extra", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Lone Peak Shrine", "./Resources/MM/Goron_Village/Lone_Peak_Shrine.png", (uint8_t)MMRegions::Goron_Village, false, GameLayout::mm},
	{ "Shop", "./Resources/MM/Zora_Hall/Zora_Shop.png", (uint8_t)MMRegions::Zora_Hall, false, GameLayout::mm},
	{ "Evan's Room", "./Resources/MM/Zora_Hall/Evan_Room.png", (uint8_t)MMRegions::Zora_Hall, false, GameLayout::mm},
	{ "Stable", "", (uint8_t)MMRegions::None, false, GameLayout::mm},
	{ "Dodongo Grotto", "./Resources/MM/Termina/Dodongo.png", (uint8_t)MMRegions::Termina, false, GameLayout::mm},
	{ "Gossip Stones Grotto, Ocean", "./Resources/MM/Termina/Ocean_Gossip.png", (uint8_t)MMRegions::Termina, false, GameLayout::mm},
	{ "Gossip Stones Grotto, Canyon", "./Resources/MM/Termina/Canyon_Gossip.png", (uint8_t)MMRegions::Termina, false, GameLayout::mm},
	{ "Bio Baba Grotto", "./Resources/MM/Termina/Bio_Baba.png", (uint8_t)MMRegions::Termina, false, GameLayout::mm},
	{ "Peehat Grotto", "./Resources/MM/Termina/Peehat.png", (uint8_t)MMRegions::Termina, false, GameLayout::mm},
	{ "Deku Scrub Grotto", "./Resources/MM/Termina/Scrub.png", (uint8_t)MMRegions::Termina, false, GameLayout::mm},
	{ "Tall Grass Grotto", "./Resources/MM/Grottos/Generic.png", (uint8_t)MMRegions::Termina, false, GameLayout::mm},
	{ "Cow Grotto", "./Resources/MM/Grottos/Cow.png", (uint8_t)MMRegions::Termina, false, GameLayout::mm},
	{ "Pillar Grotto", "./Resources/MM/Grottos/Generic.png", (uint8_t)MMRegions::Termina, false, GameLayout::mm},
	{ "Open Grotto", "./Resources/MM/Grottos/Generic.png", (uint8_t)MMRegions::Great_Bay_Coast, false, GameLayout::mm},
	{ "Cow Grotto", "./Resources/MM/Grottos/Cow.png", (uint8_t)MMRegions::Great_Bay_Coast, false, GameLayout::mm},
	{ "Generic Grotto", "./Resources/MM/Grottos/Generic.png", (uint8_t)MMRegions::Zora_Cape, false, GameLayout::mm},
	{ "Generic Grotto", "./Resources/MM/Grottos/Generic.png", (uint8_t)MMRegions::Graveyard, false, GameLayout::mm},
	{ "Open Grotto", "./Resources/MM/Grottos/Generic.png", (uint8_t)MMRegions::Ikana_Canyon, false, GameLayout::mm},
	{ "Generic Grotto", "./Resources/MM/Grottos/Generic.png", (uint8_t)MMRegions::Road_to_Ikana, false, GameLayout::mm},
	{ "Frozen Grotto", "./Resources/MM/Twin_Islands/Frozen.png", (uint8_t)MMRegions::Twin_Islands, false, GameLayout::mm},
	{ "Ramp Grotto", "./Resources/MM/Grottos/Generic.png", (uint8_t)MMRegions::Twin_Islands, false, GameLayout::mm},
	{ "Generic Grotto", "./Resources/MM/Grottos/Generic.png", (uint8_t)MMRegions::Path_to_Snowhead, false, GameLayout::mm},
	{ "Generic Grotto", "./Resources/MM/Grottos/Generic.png", (uint8_t)MMRegions::Mountain_Village, false, GameLayout::mm},
	{ "Open Grotto", "./Resources/MM/Grottos/Generic.png", (uint8_t)MMRegions::Southern_Swamp_Road, false, GameLayout::mm},
	{ "Open Grotto", "./Resources/MM/Grottos/Generic.png", (uint8_t)MMRegions::Southern_Swamp, false, GameLayout::mm},
	{ "Open Grotto", "./Resources/MM/Grottos/Generic.png", (uint8_t)MMRegions::Woods_of_Mystery, false, GameLayout::mm},
	{ "Beans Grotto", "./Resources/MM/Deku_Palace/Beans.png", (uint8_t)MMRegions::Deku_Palace, false, GameLayout::mm},
	{ "Generic Grotto", "./Resources/MM/Deku_Palace/Generic.png", (uint8_t)MMRegions::Deku_Palace, false, GameLayout::mm},
	{ "Mountain Village", "./Resources/MM/Mountain_Village/Village.png", (uint8_t)MMRegions::Mountain_Village, true, GameLayout::mm},
	{ "Twin Islands", "./Resources/MM/Twin_Islands/Twins.png", (uint8_t)MMRegions::Twin_Islands, true, GameLayout::mm}
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
			case FROGS_ZL:
			case FROGS_EPONA:
			case FROGS_SARIA:
			case FROGS_SUNS:
			case FROGS_SOT:
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
			case TALON_MILK:
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

			// Haunted Wasteland
			case CARPET_MERCHANT:
				return HAUNTED_WASTELAND;

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