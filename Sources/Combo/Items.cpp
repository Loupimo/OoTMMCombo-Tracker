#include "Combo/Items.h"

#pragma region Items

#define NUM_ITEM 0x30B

const ItemInfo ItemList[NUM_ITEM] =
{ 
	{ 0x01, "5 Bombs (OoT)" },
	{ 0x02, "5 Deku Nuts (OoT)" },
	{ 0x03, "10 Bombchu (OoT)" },
	{ 0x04, "Fairy Bow (OoT)" },
	{ 0x05, "Fairy Slingshot (OoT)" },
	{ 0x06, "Boomerang (OoT)" },
	{ 0x07, "Deku Stick (OoT)" },
	{ 0x08, "Hookshot (OoT)" },
	{ 0x09, "Longshot (OoT)" },
	{ 0x0A, "Lens of Truth (OoT)" },
	{ 0x0B, "Zelda's Letter" },
	{ 0x0C, "Ocarina of Time (OoT)" },
	{ 0x0D, "Megaton Hammer (OoT)" },
	{ 0x0E, "Cojiro" },
	{ 0x0F, "Empty Bottle (OoT)" },
	{ 0x10, "Red Potion (OoT)" },
	{ 0x11, "Green Potion (OoT)" },
	{ 0x12, "Blue Potion (OoT)" },
	{ 0x13, "Fairy (OoT)" },
	{ 0x14, "Bottle of Lon Lon Milk (OoT)" },
	{ 0x15, "Ruto's Letter" },
	{ 0x16, "Magic Bean (OoT)" },
	{ 0x17, "Skull Mask (OoT)" },
	{ 0x18, "Spooky Mask (OoT)" },
	{ 0x19, "Chicken" },
	{ 0x1A, "Keaton Mask (OoT)" },
	{ 0x1B, "Bunny Hood (OoT)" },
	{ 0x1C, "Mask of Truth (OoT)" },
	{ 0x1D, "Pocket Egg" },
	{ 0x1E, "Pocket Cucco" },
	{ 0x1F, "Odd Mushroom" },
	{ 0x20, "Odd Potion" },
	{ 0x21, "Poacher's Saw" },
	{ 0x22, "Broken Goron's Sword" },
	{ 0x23, "Prescription" },
	{ 0x24, "Eyeball Frog" },
	{ 0x25, "Eye Drops" },
	{ 0x26, "Claim Check" },
	{ 0x27, "Kokiri Sword (OoT)" },
	{ 0x28, "Giant's Knife (OoT)" },
	{ 0x29, "Deku Shield (OoT)" },
	{ 0x2A, "Hylian Shield (OoT)" },
	{ 0x2B, "Mirror Shield (OoT)" },
	{ 0x2C, "Goron Tunic (OoT)" },
	{ 0x2D, "Zora Tunic (OoT)" },
	{ 0x2E, "Iron Boots (OoT)" },
	{ 0x2F, "Hover Boots (OoT)" },
	{ 0x30, "Big Quiver (OoT)" },
	{ 0x31, "Biggest Quiver (OoT)" },
	{ 0x32, "Bomb Bag (OoT)" },
	{ 0x33, "Big Bomb Bag (OoT)" },
	{ 0x34, "Biggest Bomb Bag (OoT)" },
	{ 0x35, "Silver Gauntlets (OoT)" },
	{ 0x36, "Golden Gauntlets (OoT)" },
	{ 0x37, "Silver Scale (OoT)" },
	{ 0x38, "Golden Scale (OoT)" },
	{ 0x39, "Stone of Agony (OoT)" },
	{ 0x3A, "Gerudo's Card (OoT)" },
	{ 0x3B, "Fairy Ocarina (OoT)" },
	{ 0x3C, "5 Deku Seeds (OoT)" },
	{ 0x3D, "Heart Container (OoT)" },
	{ 0x3E, "Piece of Heart (OoT)" },
	{ 0x3F, "Boss Key (OoT)" },
	{ 0x40, "Compass (OoT)" },
	{ 0x41, "Dungeon Map (OoT)" },
	{ 0x42, "Small Key (OoT)" },
	{ 0x43, "Small Magic Jar (OoT)" },
	{ 0x44, "Large Magic Jar (OoT)" },
	{ 0x45, "Adult's Wallet (OoT)" },
	{ 0x46, "Giant's Wallet (OoT)" },
	{ 0x47, "Weird Egg" },
	{ 0x48, "Recovery Heart (OoT)" },
	{ 0x49, "5 Arrows (OoT)" },
	{ 0x4A, "10 Arrows (OoT)" },
	{ 0x4B, "30 Arrows (OoT)" },
	{ 0x4C, "Green Rupee (OoT)" },
	{ 0x4D, "Blue Rupee (OoT)" },
	{ 0x4E, "Red Rupee (OoT)" },
	{ 0x4F, "Heart Container (OoT)" },
	{ 0x50, "Lon Lon Milk (OoT)" },
	{ 0x51, "Goron Mask (OoT)" },
	{ 0x52, "Zora Mask (OoT)" },
	{ 0x53, "Gerudo Mask (OoT)" },
	{ 0x54, "Goron's Bracelet (OoT)" },
	{ 0x55, "Purple Rupee (OoT)" },
	{ 0x56, "Huge Rupee (OoT)" },
	{ 0x57, "Biggoron's Sword (OoT)" },
	{ 0x58, "Fire Arrow (OoT)" },
	{ 0x59, "Ice Arrow (OoT)" },
	{ 0x5A, "Light Arrow (OoT)" },
	{ 0x5B, "Gold Skulltula Token" },
	{ 0x5C, "Din's Fire (OoT)" },
	{ 0x5D, "Farore's Wind (OoT)" },
	{ 0x5E, "Nayru's Love (OoT)" },
	{ 0x5F, "Bullet Bag (OoT)" },
	{ 0x60, "Large Bullet Bag (OoT)" },
	{ 0x61, "5 Deku Sticks (OoT)" },
	{ 0x62, "10 Deku Sticks (OoT)" },
	{ 0x63, "5 Deku Nuts (OoT)" },
	{ 0x64, "10 Deku Nuts (OoT)" },
	{ 0x65, "Bomb (OoT)" },
	{ 0x66, "10 Bombs (OoT)" },
	{ 0x67, "20 Bombs (OoT)" },
	{ 0x68, "30 Bombs (OoT)" },
	{ 0x69, "30 Deku Seeds (OoT)" },
	{ 0x6A, "5 Bombchu (OoT)" },
	{ 0x6B, "20 Bombchu (OoT)" },
	{ 0x6C, "Fish (OoT)" },
	{ 0x6D, "Bug (OoT)" },
	{ 0x6E, "Blue Fire (OoT)" },
	{ 0x6F, "Poe (OoT)" },
	{ 0x70, "Big Poe (OoT)" },
	{ 0x71, "Small Key (OoT)" },
	{ 0x72, "Green Rupee (OoT)" },
	{ 0x73, "Blue Rupee (OoT)" },
	{ 0x74, "Red Rupee (OoT)" },
	{ 0x75, "Purple Rupee (OoT)" },
	{ 0x76, "Piece of Heart (OoT)" },
	{ 0x77, "Deku Stick Upgrade (OoT)" },
	{ 0x78, "Second Deku Stick Upgrade (OoT)" },
	{ 0x79, "Deku Nut Upgrade (OoT)" },
	{ 0x7A, "Second Deku Nut Upgrade (OoT)" },
	{ 0x7B, "Largest Bullet Bag (OoT)" },
	{ 0x7C, "Ice Trap (OoT)" },
	{ 0x7D, "Unknown" },
	{ 0x7E, "Unknown" },
	{ 0x7F, "Unknown" },
	{ 0x80, "Rainbow Rupee" },
	{ 0x81, "Minuet of Forest" },
	{ 0x82, "Bolero of Fire" },
	{ 0x83, "Serenade of Water" },
	{ 0x84, "Requiem of Spirit" },
	{ 0x85, "Nocturne of Shadow" },
	{ 0x86, "Prelude of Light" },
	{ 0x87, "Zelda's Lullaby" },
	{ 0x88, "Epona's Song (OoT)" },
	{ 0x89, "Saria's Song" },
	{ 0x8A, "Sun's Song (OoT)" },
	{ 0x8B, "Song of Time (OoT)" },
	{ 0x8C, "Song of Storms (OoT)" },
	{ 0x8D, "Master Sword (OoT)" },
	{ 0x8E, "Kokiri's Emerald" },
	{ 0x8F, "Goron's Ruby" },
	{ 0x90, "Zora's Sapphire" },
	{ 0x91, "Forest Medallion" },
	{ 0x92, "Fire Medallion" },
	{ 0x93, "Water Medallion" },
	{ 0x94, "Spirit Medallion" },
	{ 0x95, "Shadow Medallion" },
	{ 0x96, "Light Medallion" },
	{ 0x97, "Magic Upgrade (OoT)" },
	{ 0x98, "Larger Magic Upgrade (OoT)" },
	{ 0x99, "Defense Upgrade (OoT)" },
	{ 0x9A, "Progressive Deku Shield (OoT)" },
	{ 0x9B, "Progressive Hylian Shield (OoT)" },
	{ 0x9C, "Small Key (Forest)" },
	{ 0x9D, "Small Key (Fire)" },
	{ 0x9E, "Small Key (Water)" },
	{ 0x9F, "Small Key (Spirit)" },
	{ 0xA0, "Small Key (Shadow)" },
	{ 0xA1, "Small Key (Ganon)" },
	{ 0xA2, "Small Key (Well)" },
	{ 0xA3, "Small Key (Hideout)" },
	{ 0xA4, "Small Key (GTG)" },
	{ 0xA5, "Boss Key (Forest)" },
	{ 0xA6, "Boss Key (Fire)" },
	{ 0xA7, "Boss Key (Water)" },
	{ 0xA8, "Boss Key (Spirit)" },
	{ 0xA9, "Boss Key (Shadow)" },
	{ 0xAA, "Boss Key (Ganon)" },
	{ 0xAB, "Dungeon Map (Deku Tree)" },
	{ 0xAC, "Dungeon Map (Dodongo's Cavern)" },
	{ 0xAD, "Dungeon Map (Jabu-Jabu)" },
	{ 0xAE, "Dungeon Map (Forest)" },
	{ 0xAF, "Dungeon Map (Fire)" },
	{ 0xB0, "Dungeon Map (Water)" },
	{ 0xB1, "Dungeon Map (Spirit)" },
	{ 0xB2, "Dungeon Map (Shadow)" },
	{ 0xB3, "Dungeon Map (Bottom of the Well)" },
	{ 0xB4, "Dungeon Map (Ice Cavern)" },
	{ 0xB5, "Compass (Deku Tree)" },
	{ 0xB6, "Compass (Dodongo's Cavern)" },
	{ 0xB7, "Compass (Jabu-Jabu)" },
	{ 0xB8, "Compass (Forest)" },
	{ 0xB9, "Compass (Fire)" },
	{ 0xBA, "Compass (Water)" },
	{ 0xBB, "Compass (Spirit)" },
	{ 0xBC, "Compass (Shadow)" },
	{ 0xBD, "Compass (Bottom of the Well)" },
	{ 0xBE, "Compass (Ice Cavern)" },
	{ 0xBF, "Child's Wallet (OoT)" },
	{ 0xC0, "Colossal Wallet (OoT)" },
	{ 0xC1, "Triforce Piece" },
	{ 0xC2, "Bottomless Wallet (OoT)" },
	{ 0xC3, "Silver Rupee (Dodongo's Cavern)" },
	{ 0xC4, "Silver Rupee (Bottom of the Well)" },
	{ 0xC5, "Silver Rupee (Spirit Child)" },
	{ 0xC6, "Silver Rupee (Spirit Sun)" },
	{ 0xC7, "Silver Rupee (Spirit Boulders)" },
	{ 0xC8, "Silver Rupee (Spirit Lobby)" },
	{ 0xC9, "Silver Rupee (Spirit Adult)" },
	{ 0xCA, "Silver Rupee (Shadow Scythe)" },
	{ 0xCB, "Silver Rupee (Shadow Pit)" },
	{ 0xCC, "Silver Rupee (Shadow Spikes)" },
	{ 0xCD, "Silver Rupee (Shadow Blades)" },
	{ 0xCE, "Silver Rupee (Ice Scythe)" },
	{ 0xCF, "Silver Rupee (Ice Block)" },
	{ 0xD0, "Silver Rupee (GTG Slopes)" },
	{ 0xD1, "Silver Rupee (GTG Lava)" },
	{ 0xD2, "Silver Rupee (GTG Water)" },
	{ 0xD3, "Silver Rupee (Ganon Spirit)" },
	{ 0xD4, "Silver Rupee (Ganon Light)" },
	{ 0xD5, "Silver Rupee (Ganon Fire)" },
	{ 0xD6, "Silver Rupee (Ganon Forest)" },
	{ 0xD7, "Silver Rupee (Ganon Shadow)" },
	{ 0xD8, "Silver Rupee (Ganon Water)" },
	{ 0xD9, "Soul of Stalfos" },
	{ 0xDA, "Soul of Octoroks (OoT)" },
	{ 0xDB, "Soul of Wallmasters (OoT)" },
	{ 0xDC, "Soul of Dodongos (OoT)" },
	{ 0xDD, "Soul of Keeses (OoT)" },
	{ 0xDE, "Soul of Tektites (OoT)" },
	{ 0xDF, "Soul of Leevers (OoT)" },
	{ 0xE0, "Soul of Peahats (OoT)" },
	{ 0xE1, "Soul of Lizalfos / Dinalfos (OoT)" },
	{ 0xE2, "Soul of Gohma Larvae" },
	{ 0xE3, "Soul of Shaboms" },
	{ 0xE4, "Soul of Baby Dodongos" },
	{ 0xE5, "Soul of Dark Link" },
	{ 0xE6, "Soul of Biris / Baris" },
	{ 0xE7, "Soul of Tailpasarans" },
	{ 0xE8, "Soul of Jabu-Jabu's Parasites" },
	{ 0xE9, "Soul of Skulltulas (OoT)" },
	{ 0xEA, "Soul of Torch Slugs" },
	{ 0xEB, "Soul of Moblins" },
	{ 0xEC, "Soul of Armos (OoT)" },
	{ 0xED, "Soul of Deku Babas (OoT)" },
	{ 0xEE, "Soul of Deku Scrubs (OoT)" },
	{ 0xEF, "Soul of Bubbles (OoT)" },
	{ 0xF0, "Soul of Beamos (OoT)" },
	{ 0xF1, "Soul of Floormasters (OoT)" },
	{ 0xF2, "Soul of ReDeads / Gibdos (OoT)" },
	{ 0xF3, "Soul of Skullwalltulas (OoT)" },
	{ 0xF4, "Soul of Flare Dancers" },
	{ 0xF5, "Soul of Dead Hands" },
	{ 0xF6, "Soul of Shell Blades (OoT)" },
	{ 0xF7, "Soul of Like-Likes (OoT)" },
	{ 0xF8, "Soul of Spikes" },
	{ 0xF9, "Soul of Anubis" },
	{ 0xFA, "Soul of Iron Knuckles (OoT)" },
	{ 0xFB, "Soul of Skull Kids" },
	{ 0xFC, "Soul of Flying Pots (OoT)" },
	{ 0xFD, "Soul of Freezards (OoT)" },
	{ 0xFE, "Soul of Stingers" },
	{ 0xFF, "Soul of Wolfos (OoT)" },
	{ 0x100, "Soul of Stalchildren (OoT)" },
	{ 0x101, "Soul of Guays (OoT)" },
	{ 0x102, "Soul of the Fighting Gerudos" },
	{ 0x103, "Soul of Queen Gohma" },
	{ 0x104, "Soul of King Dodongo" },
	{ 0x105, "Soul of Barinade" },
	{ 0x106, "Soul of Phantom Ganon" },
	{ 0x107, "Soul of Volvagia" },
	{ 0x108, "Soul of Morpha" },
	{ 0x109, "Soul of Bongo-Bongo" },
	{ 0x10A, "Soul of Twinrova" },
	{ 0x10B, "Soul of Gold Skulltulas (OoT)" },
	{ 0x10C, "Soul of Business Scrubs (OoT)" },
	{ 0x10D, "Key Ring (OoT)" },
	{ 0x10E, "Key Ring (Forest)" },
	{ 0x10F, "Key Ring (Fire)" },
	{ 0x110, "Key Ring (Water)" },
	{ 0x111, "Key Ring (Spirit)" },
	{ 0x112, "Key Ring (Shadow)" },
	{ 0x113, "Key Ring (Ganon)" },
	{ 0x114, "Key Ring (Bottom of the Well)" },
	{ 0x115, "Key Ring (Hideout)" },
	{ 0x116, "Key Ring (GTG)" },
	{ 0x117, "Skeleton Key (OoT)" },
	{ 0x118, "Red Coin (OoT)" },
	{ 0x119, "Green Coin (OoT)" },
	{ 0x11A, "Blue Coin (OoT)" },
	{ 0x11B, "Yellow Coin (OoT)" },
	{ 0x11C, "Triforce of Power" },
	{ 0x11D, "Triforce of Courage" },
	{ 0x11E, "Triforce of Wisdom" },
	{ 0x11F, "Silver Pouch (Dodongo's Cavern)" },
	{ 0x120, "Silver Pouch (Bottom of the Well)" },
	{ 0x121, "Silver Pouch (Spirit Child)" },
	{ 0x122, "Silver Pouch (Spirit Sun)" },
	{ 0x123, "Silver Pouch (Spirit Boulders)" },
	{ 0x124, "Silver Pouch (Spirit Lobby)" },
	{ 0x125, "Silver Pouch (Spirit Adult)" },
	{ 0x126, "Silver Pouch (Shadow Scythe)" },
	{ 0x127, "Silver Pouch (Shadow Pit)" },
	{ 0x128, "Silver Pouch (Shadow Spikes)" },
	{ 0x129, "Silver Pouch (Shadow Blades)" },
	{ 0x12A, "Silver Pouch (Ice Scythe)" },
	{ 0x12B, "Silver Pouch (Ice Block)" },
	{ 0x12C, "Silver Pouch (GTG Slopes)" },
	{ 0x12D, "Silver Pouch (GTG Lava)" },
	{ 0x12E, "Silver Pouch (GTG Water)" },
	{ 0x12F, "Silver Pouch (Ganon Spirit)" },
	{ 0x130, "Silver Pouch (Ganon Light)" },
	{ 0x131, "Silver Pouch (Ganon Fire)" },
	{ 0x132, "Silver Pouch (Ganon Forest)" },
	{ 0x133, "Silver Pouch (Ganon Shadow)" },
	{ 0x134, "Silver Pouch (Ganon Water)" },
	{ 0x135, "Magical Rupee" },
	{ 0x136, "A Button (OoT)" },
	{ 0x137, "C-Right Button (OoT)" },
	{ 0x138, "C-Left Button (OoT)" },
	{ 0x139, "C-Up Button (OoT)" },
	{ 0x13A, "C-Down Button (OoT)" },
	{ 0x13B, "Small Key (Chest Game)" },
	{ 0x13C, "Key Ring (Chest Game)" },
	{ 0x13D, "Soul of Saria" },
	{ 0x13E, "Soul of Darunia" },
	{ 0x13F, "Soul of Ruto" },
	{ 0x140, "Soul of King Zora" },
	{ 0x141, "Soul of Zelda" },
	{ 0x142, "Soul of Sheik" },
	{ 0x143, "Soul of Cucco Lady" },
	{ 0x144, "Soul of Carpenters (OoT)" },
	{ 0x145, "Soul of Guru-Guru (OoT)" },
	{ 0x146, "Soul of Mido" },
	{ 0x147, "Soul of Kokiri" },
	{ 0x148, "Soul of Kokiri Shopkeeper" },
	{ 0x149, "Soul of Hylian Guard" },
	{ 0x14A, "Soul of Citizens (OoT)" },
	{ 0x14B, "Soul of Malon" },
	{ 0x14C, "Soul of Talon" },
	{ 0x14D, "Soul of Fishing Pond Owner" },
	{ 0x14E, "Soul of Goron (OoT)" },
	{ 0x14F, "Soul of Medigoron" },
	{ 0x150, "Soul of Biggoron (OoT)" },
	{ 0x151, "Soul of Child Goron" },
	{ 0x152, "Soul of Goron Shopkeeper (OoT)" },
	{ 0x153, "Soul of Zora (OoT)" },
	{ 0x154, "Soul of Zora Shopkeeper (OoT)" },
	{ 0x155, "Soul of Bazaar Shopkeeper" },
	{ 0x156, "Soul of Bombchu Shopkeeper" },
	{ 0x157, "Soul of Potion Shopkeeper" },
	{ 0x158, "Soul of Bowling Lady" },
	{ 0x159, "Soul of Chest Game Owner" },
	{ 0x15A, "Soul of Shooting Gallery Owner" },
	{ 0x15B, "Soul of Dampe (OoT)" },
	{ 0x15C, "Soul of Graveyard Kid" },
	{ 0x15D, "Soul of Poe Collector" },
	{ 0x15E, "Soul of Composer Bros. (OoT)" },
	{ 0x15F, "Soul of Honey & Darling (OoT)" },
	{ 0x160, "Soul of the Astronomer (OoT)" },
	{ 0x161, "Soul of the Rooftop Man" },
	{ 0x162, "Soul of the Bean Salesman (OoT)" },
	{ 0x163, "Soul of the Scientist (OoT)" },
	{ 0x164, "Soul of Ingo" },
	{ 0x165, "Soul of the Punk Boy" },
	{ 0x166, "Soul of the Dog Lady (OoT)" },
	{ 0x167, "Soul of the Carpet Man" },
	{ 0x168, "Soul of the Old Hag" },
	{ 0x169, "Soul of the Beggar" },
	{ 0x16A, "Soul of the Patrolling Gerudos" },
	{ 0x16B, "Bottle of Red Potion (OoT)" },
	{ 0x16C, "Bottle of Green Potion (OoT)" },
	{ 0x16D, "Bottle of Blue Potion (OoT)" },
	{ 0x16E, "Bottled Fairy (OoT)" },
	{ 0x16F, "Bottled Poe (OoT)" },
	{ 0x170, "Bottled Big Poe (OoT)" },
	{ 0x171, "Bottle of Blue Fire" },
	{ 0x172, "Child Fish (2 pounds)" },
	{ 0x173, "Child Fish (3 pounds)" },
	{ 0x174, "Child Fish (4 pounds)" },
	{ 0x175, "Child Fish (5 pounds)" },
	{ 0x176, "Child Fish (6 pounds)" },
	{ 0x177, "Child Fish (7 pounds)" },
	{ 0x178, "Child Fish (8 pounds)" },
	{ 0x179, "Child Fish (9 pounds)" },
	{ 0x17A, "Child Fish (10 pounds)" },
	{ 0x17B, "Child Fish (11 pounds)" },
	{ 0x17C, "Child Fish (12 pounds)" },
	{ 0x17D, "Child Fish (13 pounds)" },
	{ 0x17E, "Child Fish (14 pounds)" },
	{ 0x17F, "Adult Fish (4 pounds)" },
	{ 0x180, "Adult Fish (5 pounds)" },
	{ 0x181, "Adult Fish (6 pounds)" },
	{ 0x182, "Adult Fish (7 pounds)" },
	{ 0x183, "Adult Fish (8 pounds)" },
	{ 0x184, "Adult Fish (9 pounds)" },
	{ 0x185, "Adult Fish (10 pounds)" },
	{ 0x186, "Adult Fish (11 pounds)" },
	{ 0x187, "Adult Fish (12 pounds)" },
	{ 0x188, "Adult Fish (13 pounds)" },
	{ 0x189, "Adult Fish (14 pounds)" },
	{ 0x18A, "Adult Fish (15 pounds)" },
	{ 0x18B, "Adult Fish (16 pounds)" },
	{ 0x18C, "Adult Fish (17 pounds)" },
	{ 0x18D, "Adult Fish (18 pounds)" },
	{ 0x18E, "Adult Fish (19 pounds)" },
	{ 0x18F, "Adult Fish (20 pounds)" },
	{ 0x190, "Adult Fish (21 pounds)" },
	{ 0x191, "Adult Fish (22 pounds)" },
	{ 0x192, "Adult Fish (23 pounds)" },
	{ 0x193, "Adult Fish (24 pounds)" },
	{ 0x194, "Adult Fish (25 pounds)" },
	{ 0x195, "Child Loach (14 pounds)" },
	{ 0x196, "Child Loach (15 pounds)" },
	{ 0x197, "Child Loach (16 pounds)" },
	{ 0x198, "Child Loach (17 pounds)" },
	{ 0x199, "Child Loach (18 pounds)" },
	{ 0x19A, "Child Loach (19 pounds)" },
	{ 0x19B, "Adult Loach (29 pounds)" },
	{ 0x19C, "Adult Loach (30 pounds)" },
	{ 0x19D, "Adult Loach (31 pounds)" },
	{ 0x19E, "Adult Loach (32 pounds)" },
	{ 0x19F, "Adult Loach (33 pounds)" },
	{ 0x1A0, "Adult Loach (34 pounds)" },
	{ 0x1A1, "Adult Loach (35 pounds)" },
	{ 0x1A2, "Adult Loach (36 pounds)" },
	{ 0x1A3, "Bombchu Bag (OoT)" },
	{ 0x1A4, "Big Bombchu Bag (OoT)" },
	{ 0x1A5, "Biggest Bombchu Bag (OoT)" },
	{ 0x1A6, "Bombchu Bag (OoT)" },
	{ 0x1A7, "Bombchu Bag (OoT)" },
	{ 0x1A8, "Bombchu Bag (OoT)" },
	{ 0x1A9, "Razor Sword (OoT)" },
	{ 0x1AA, "Gilded Sword (OoT)" },
	{ 0x1AB, "Spin Attack Upgrade (OoT)" },
	{ 0x1AC, "Triforce (OoT)" },
	{ 0x1AD, "Ice Trap (OoT)" },
	{ 0x1AE, "Big Fairy (OoT)" },
	{ 0x1AF, "Blast Mask (OoT)" },
	{ 0x1B0, "Stone Mask (OoT)" },
	{ 0x1B1, "Rupoor (OoT)" },
	{ 0x1B2, "Elegy of Emptiness (OoT)" },
	{ 0x1B3, "Bronze Scale (OoT)" },
	{ 0x1B4, "Green Rupee (MM)" },
	{ 0x1B5, "Blue Rupee (MM)" },
	{ 0x1B6, "Red Rupee (MM)" },
	{ 0x1B7, "Red Rupee (MM)" },
	{ 0x1B8, "Purple Rupee (MM)" },
	{ 0x1B9, "Silver Rupee (MM)" },
	{ 0x1BA, "Gold Rupee (MM)" },
	{ 0x1BB, "Adult Wallet (MM)" },
	{ 0x1BC, "Giant Wallet (MM)" },
	{ 0x1BD, "Recovery Heart (MM)" },
	{ 0x1BE, "Piece of Heart (MM)" },
	{ 0x1BF, "Heart Container (MM)" },
	{ 0x1C0, "Small Magic Jar (MM)" },
	{ 0x1C1, "Large Magic Jar (MM)" },
	{ 0x1C2, "Stray Fairy (MM)" },
	{ 0x1C3, "Bomb (MM)" },
	{ 0x1C4, "5 Bombs (MM)" },
	{ 0x1C5, "10 Bombs (MM)" },
	{ 0x1C6, "20 Bombs (MM)" },
	{ 0x1C7, "30 Bombs (MM)" },
	{ 0x1C8, "Deku Stick (MM)" },
	{ 0x1C9, "10 Bombchu (MM)" },
	{ 0x1CA, "Bomb Bag (MM)" },
	{ 0x1CB, "Big Bomb Bag (MM)" },
	{ 0x1CC, "Biggest Bomb Bag (MM)" },
	{ 0x1CD, "10 Arrows (MM)" },
	{ 0x1CE, "30 Arrows (MM)" },
	{ 0x1CF, "40 Arrows (MM)" },
	{ 0x1D0, "40 Arrows (MM)" },
	{ 0x1D1, "Hero's Bow (MM)" },
	{ 0x1D2, "Big Quiver (MM)" },
	{ 0x1D3, "Biggest Quiver (MM)" },
	{ 0x1D4, "Fire Arrow (MM)" },
	{ 0x1D5, "Ice Arrow (MM)" },
	{ 0x1D6, "Light Arrow (MM)" },
	{ 0x1D7, "Deku Nut (MM)" },
	{ 0x1D8, "5 Deku Nuts (MM)" },
	{ 0x1D9, "10 Deku Nuts (MM)" },
	{ 0x1DA, "30 Deku Nuts (MM)" },
	{ 0x1DB, "30 Deku Nuts (MM)" },
	{ 0x1DC, "40 Deku Nuts (MM)" },
	{ 0x1DD, "20 Bombchu (MM)" },
	{ 0x1DE, "20 Deku Sticks (MM)" },
	{ 0x1DF, "20 Deku Sticks (MM)" },
	{ 0x1E0, "30 Deku Sticks (MM)" },
	{ 0x1E1, "Deku Shield (MM)" },
	{ 0x1E2, "Hero's Shield (MM)" },
	{ 0x1E3, "Mirror Shield (MM)" },
	{ 0x1E4, "Progressive Deku Shield (MM)" },
	{ 0x1E5, "Progressive Hero's Shield (MM)" },
	{ 0x1E6, "Powder Keg" },
	{ 0x1E7, "Magic Bean (MM)" },
	{ 0x1E8, "Bombchu (MM)" },
	{ 0x1E9, "Kokiri Sword (MM)" },
	{ 0x1EA, "Razor Sword (MM)" },
	{ 0x1EB, "Gilded Sword (MM)" },
	{ 0x1EC, "5 Bombchu (MM)" },
	{ 0x1ED, "Great Fairy's Sword (MM)" },
	{ 0x1EE, "Small Key (MM)" },
	{ 0x1EF, "Boss Key (MM)" },
	{ 0x1F0, "Dungeon Map (MM)" },
	{ 0x1F1, "Compass (MM)" },
	{ 0x1F2, "Hookshot (MM)" },
	{ 0x1F3, "Lens of Truth (MM)" },
	{ 0x1F4, "Pictograph Box" },
	{ 0x1F5, "Ocarina of Time (MM)" },
	{ 0x1F6, "Bomber's Notebook (MM)" },
	{ 0x1F7, "Bottle of Red Potion (MM)" },
	{ 0x1F8, "Empty Bottle (MM)" },
	{ 0x1F9, "Red Potion (MM)" },
	{ 0x1FA, "Green Potion (MM)" },
	{ 0x1FB, "Blue Potion (MM)" },
	{ 0x1FC, "Fairy (MM)" },
	{ 0x1FD, "Bottle of Milk (MM)" },
	{ 0x1FE, "Fish (MM)" },
	{ 0x1FF, "Bug (MM)" },
	{ 0x200, "Big Poe (MM)" },
	{ 0x201, "Bottle of Gold Dust" },
	{ 0x202, "Magic Mushroom" },
	{ 0x203, "Bottle of Chateau Romani" },
	{ 0x204, "Deku Mask" },
	{ 0x205, "Goron Mask (MM)" },
	{ 0x206, "Zora Mask (MM)" },
	{ 0x207, "Fierce Deity's Mask" },
	{ 0x208, "Captain's Hat" },
	{ 0x209, "Giant's Mask" },
	{ 0x20A, "All Night Mask" },
	{ 0x20B, "Bunny Hood (MM)" },
	{ 0x20C, "Keaton Mask (MM)" },
	{ 0x20D, "Garo's Mask" },
	{ 0x20E, "Romani's Mask" },
	{ 0x20F, "Circus Leader's Mask" },
	{ 0x210, "Postman's Hat" },
	{ 0x211, "Couple's Mask" },
	{ 0x212, "Great Fairy Mask" },
	{ 0x213, "Gibdo Mask" },
	{ 0x214, "Don Gero's Mask" },
	{ 0x215, "Kamaro's Mask" },
	{ 0x216, "Mask of Truth (MM)" },
	{ 0x217, "Stone Mask (MM)" },
	{ 0x218, "Bremen Mask" },
	{ 0x219, "Blast Mask (MM)" },
	{ 0x21A, "Mask of Scents" },
	{ 0x21B, "Kafei's Mask" },
	{ 0x21C, "Chateau Romani" },
	{ 0x21D, "Milk (MM)" },
	{ 0x21E, "Gold Dust" },
	{ 0x21F, "Seahorse" },
	{ 0x220, "Moon's Tear" },
	{ 0x221, "Land Title Deed" },
	{ 0x222, "Swamp Title Deed" },
	{ 0x223, "Mountain Title Deed" },
	{ 0x224, "Ocean Title Deed" },
	{ 0x225, "Room Key" },
	{ 0x226, "Letter to Mama" },
	{ 0x227, "Letter to Kafei" },
	{ 0x228, "Pendant of Memories" },
	{ 0x229, "World Map (Clock Town)" },
	{ 0x22A, "World Map (Woodfall)" },
	{ 0x22B, "World Map (Snowhead)" },
	{ 0x22C, "World Map (Romani Ranch)" },
	{ 0x22D, "World Map (Great Bay)" },
	{ 0x22E, "World Map (Stone Tower)" },
	{ 0x22F, "Magic Upgrade (MM)" },
	{ 0x230, "Larger Magic Upgrade (MM)" },
	{ 0x231, "Defense Upgrade (MM)" },
	{ 0x232, "Spin Attack Upgrade (MM)" },
	{ 0x233, "Sonata of Awakening" },
	{ 0x234, "Goron Lullaby" },
	{ 0x235, "New Wave Bossa Nova" },
	{ 0x236, "Elegy of Emptiness (MM)" },
	{ 0x237, "Oath to Order" },
	{ 0x238, "Song of Time (MM)" },
	{ 0x239, "Song of Healing" },
	{ 0x23A, "Epona's Song (MM)" },
	{ 0x23B, "Song of Soaring" },
	{ 0x23C, "Song of Storms (MM)" },
	{ 0x23D, "Lullaby Intro" },
	{ 0x23E, "Odolwa's Remains" },
	{ 0x23F, "Goht's Remains" },
	{ 0x240, "Gyorg's Remains" },
	{ 0x241, "Twinmold's Remains" },
	{ 0x242, "Swamp Skulltula Token" },
	{ 0x243, "Ocean Skulltula Token" },
	{ 0x244, "Small Key (Woodfall)" },
	{ 0x245, "Small Key (Snowhead)" },
	{ 0x246, "Small Key (Great Bay)" },
	{ 0x247, "Small Key (Stone Tower)" },
	{ 0x248, "Boss Key (Woodfall)" },
	{ 0x249, "Boss Key (Snowhead)" },
	{ 0x24A, "Boss Key (Great Bay)" },
	{ 0x24B, "Boss Key (Stone Tower)" },
	{ 0x24C, "Stray Fairy (Woodfall)" },
	{ 0x24D, "Stray Fairy (Snowhead)" },
	{ 0x24E, "Stray Fairy (Great Bay)" },
	{ 0x24F, "Stray Fairy (Stone Tower)" },
	{ 0x250, "Stray Fairy (Town)" },
	{ 0x251, "Dungeon Map (Woodfall)" },
	{ 0x252, "Dungeon Map (Snowhead)" },
	{ 0x253, "Dungeon Map (Great Bay)" },
	{ 0x254, "Dungeon Map (Stone Tower)" },
	{ 0x255, "Compass (Woodfall)" },
	{ 0x256, "Compass (Snowhead)" },
	{ 0x257, "Compass (Great Bay)" },
	{ 0x258, "Compass (Stone Tower)" },
	{ 0x259, "Fairy Ocarina (MM)" },
	{ 0x25A, "Short Hookshot (MM)" },
	{ 0x25B, "Sun's Song (MM)" },
	{ 0x25C, "Child Wallet (MM)" },
	{ 0x25D, "Colossal Wallet (MM)" },
	{ 0x25E, "Owl Statue (Great Bay)" },
	{ 0x25F, "Owl Statue (Zora Cape)" },
	{ 0x260, "Owl Statue (Snowhead)" },
	{ 0x261, "Owl Statue (Mountain Village)" },
	{ 0x262, "Owl Statue (Clock Town)" },
	{ 0x263, "Owl Statue (Milk Road)" },
	{ 0x264, "Owl Statue (Woodfall)" },
	{ 0x265, "Owl Statue (Southern Swamp)" },
	{ 0x266, "Owl Statue (Ikana Canyon)" },
	{ 0x267, "Owl Statue (Stone Tower)" },
	{ 0x268, "Owl Statue (Hidden)" },
	{ 0x269, "Bottomless Wallet (MM)" },
	{ 0x26A, "Soul of Octoroks (MM)" },
	{ 0x26B, "Soul of Wallmasters (MM)" },
	{ 0x26C, "Soul of Dodongos (MM)" },
	{ 0x26D, "Soul of Keeses (MM)" },
	{ 0x26E, "Soul of Tektites (MM)" },
	{ 0x26F, "Soul of Peahats (MM)" },
	{ 0x270, "Soul of Lizalfos / Dinalfos (MM)" },
	{ 0x271, "Soul of Skulltulas (MM)" },
	{ 0x272, "Soul of Armos (MM)" },
	{ 0x273, "Soul of Deku Babas (MM)" },
	{ 0x274, "Soul of Deku Scrubs (MM)" },
	{ 0x275, "Soul of Bubbles (MM)" },
	{ 0x276, "Soul of Beamos (MM)" },
	{ 0x277, "Soul of ReDeads / Gibdos (MM)" },
	{ 0x278, "Soul of Skullwalltulas (MM)" },
	{ 0x279, "Soul of Shell Blades (MM)" },
	{ 0x27A, "Soul of Like-Likes (MM)" },
	{ 0x27B, "Soul of Iron Knuckles (MM)" },
	{ 0x27C, "Soul of Freezards (MM)" },
	{ 0x27D, "Soul of Wolfos (MM)" },
	{ 0x27E, "Soul of Guays (MM)" },
	{ 0x27F, "Soul of Flying Pots (MM)" },
	{ 0x280, "Soul of Floormasters (MM)" },
	{ 0x281, "Soul of Chuchus" },
	{ 0x282, "Soul of Deep Pythons" },
	{ 0x283, "Soul of Stalchildren (MM)" },
	{ 0x284, "Soul of Leevers (MM)" },
	{ 0x285, "Soul of Skullfish" },
	{ 0x286, "Soul of Dexihands" },
	{ 0x287, "Soul of Dragonflies" },
	{ 0x288, "Soul of Eenos" },
	{ 0x289, "Soul of Eyegores" },
	{ 0x28A, "Soul of Hiploops" },
	{ 0x28B, "Soul of Real Bombchu" },
	{ 0x28C, "Soul of Takkuri" },
	{ 0x28D, "Soul of Boes" },
	{ 0x28E, "Soul of Nejirons" },
	{ 0x28F, "Soul of Bio Babas" },
	{ 0x290, "Soul of Garo" },
	{ 0x291, "Soul of Wizzrobes" },
	{ 0x292, "Soul of Gomess" },
	{ 0x293, "Soul of Gekkos" },
	{ 0x294, "Soul of Bad Bats" },
	{ 0x295, "Soul of Snappers" },
	{ 0x296, "Soul of Warts" },
	{ 0x297, "Soul of Captain Keeta" },
	{ 0x298, "Soul of the Fighting Pirates" },
	{ 0x299, "Soul of Odolwa" },
	{ 0x29A, "Soul of Goht" },
	{ 0x29B, "Soul of Gyorg" },
	{ 0x29C, "Soul of Twinmold" },
	{ 0x29D, "Soul of Igos" },
	{ 0x29E, "Soul of Madame Aroma" },
	{ 0x29F, "Soul of Town Archery Owner" },
	{ 0x2A0, "Soul of Swamp Archery Owner" },
	{ 0x2A1, "Soul of Gorons (MM)" },
	{ 0x2A2, "Soul of Baby Goron" },
	{ 0x2A3, "Soul of Bombs Shopkeeper" },
	{ 0x2A4, "Soul of Bombers' Kids" },
	{ 0x2A5, "Soul of Deku Butler" },
	{ 0x2A6, "Soul of Citizens (MM)" },
	{ 0x2A7, "Soul of Composer Bros (MM)" },
	{ 0x2A8, "Soul of Dampe (MM)" },
	{ 0x2A9, "Soul of Deku King" },
	{ 0x2AA, "Soul of Deku Princess" },
	{ 0x2AB, "Soul of Playground Scrubs" },
	{ 0x2AC, "Soul of Goron Elder" },
	{ 0x2AD, "Soul of Fisherman" },
	{ 0x2AE, "Soul of Goron Shopkeeper (MM)" },
	{ 0x2AF, "Soul of Honey & Darling (MM)" },
	{ 0x2B0, "Soul of Toto" },
	{ 0x2B1, "Soul of Lulu" },
	{ 0x2B2, "Soul of Zora Musicians" },
	{ 0x2B3, "Soul of Kafei" },
	{ 0x2B4, "Soul of Keaton" },
	{ 0x2B5, "Soul of Koume & Kotake" },
	{ 0x2B6, "Soul of Mayor Dotour" },
	{ 0x2B7, "Soul of Keg Trial Goron" },
	{ 0x2B8, "Soul of Biggoron (MM)" },
	{ 0x2B9, "Soul of Mr. Barten" },
	{ 0x2BA, "Soul of Astronomer (MM)" },
	{ 0x2BB, "Soul of Poe Hut Owner" },
	{ 0x2BC, "Soul of Blacksmiths" },
	{ 0x2BD, "Soul of Chest Game Lady" },
	{ 0x2BE, "Soul of Tingle" },
	{ 0x2BF, "Soul of Toilet Hand" },
	{ 0x2C0, "Soul of Trading Post Owner" },
	{ 0x2C1, "Soul of the Part-Timer" },
	{ 0x2C2, "Soul of Zoras (MM)" },
	{ 0x2C3, "Soul of Zora Shopkeeper (MM)" },
	{ 0x2C4, "Soul of Romani / Cremia" },
	{ 0x2C5, "Soul of the Bean Salesman (MM)" },
	{ 0x2C6, "Soul of Carpenters (MM)" },
	{ 0x2C7, "Soul of Anju" },
	{ 0x2C8, "Soul of Guru-Guru (MM)" },
	{ 0x2C9, "Soul of the Scientist (MM)" },
	{ 0x2CA, "Soul of Gorman & Bros." },
	{ 0x2CB, "Soul of Grog" },
	{ 0x2CC, "Soul of the Dog Lady (MM)" },
	{ 0x2CD, "Soul of Swordsman" },
	{ 0x2CE, "Soul of Anju's Grandmother" },
	{ 0x2CF, "Soul of Boat Cruise Man" },
	{ 0x2D0, "Soul of the Banker" },
	{ 0x2D1, "Soul of Moon Children" },
	{ 0x2D2, "Soul of Patrolling Pirates and their Chief" },
	{ 0x2D3, "Soul of Gold Skulltulas (MM)" },
	{ 0x2D4, "Soul of Business Scrubs (MM)" },
	{ 0x2D5, "Key Ring (MM)" },
	{ 0x2D6, "Key Ring (Woodfall)" },
	{ 0x2D7, "Key Ring (Snowhead)" },
	{ 0x2D8, "Key Ring (Great Bay)" },
	{ 0x2D9, "Key Ring (Stone Tower)" },
	{ 0x2DA, "Skeleton Key (MM)" },
	{ 0x2DB, "A Button (MM)" },
	{ 0x2DC, "C-Right Button (MM)" },
	{ 0x2DD, "C-Left Button (MM)" },
	{ 0x2DE, "C-Up Button (MM)" },
	{ 0x2DF, "C-Down Button (MM)" },
	{ 0x2E0, "Bottle of Green Potion (MM)" },
	{ 0x2E1, "Bottle of Blue Potion (MM)" },
	{ 0x2E2, "Bottled Fairy (MM)" },
	{ 0x2E3, "Poe (MM)" },
	{ 0x2E4, "Bottled Poe (MM)" },
	{ 0x2E5, "Bottled Big Poe (MM)" },
	{ 0x2E6, "Bombchu Bag (MM)" },
	{ 0x2E7, "Big Bombchu Bag (MM)" },
	{ 0x2E8, "Biggest Bombchu Bag (MM)" },
	{ 0x2E9, "Bombchu Bag (MM)" },
	{ 0x2EA, "Bombchu Bag (MM)" },
	{ 0x2EB, "Bombchu Bag (MM)" },
	{ 0x2EC, "Bombchu Bag (MM)" },
	{ 0x2ED, "Big Fairy (MM)" },
	{ 0x2EE, "Din's Fire (MM)" },
	{ 0x2EF, "Farore's Wind (MM)" },
	{ 0x2F0, "Nayru's Love (MM)" },
	{ 0x2F1, "Iron Boots (MM)" },
	{ 0x2F2, "Hover Boots (MM)" },
	{ 0x2F3, "Goron Tunic (MM)" },
	{ 0x2F4, "Zora Tunic (MM)" },
	{ 0x2F5, "Bronze Scale (MM)" },
	{ 0x2F6, "Silver Scale (MM)" },
	{ 0x2F7, "Golden Scale (MM)" },
	{ 0x2F8, "Goron's Bracelet (MM)" },
	{ 0x2F9, "Silver Gauntlets (MM)" },
	{ 0x2FA, "Golden Gauntlets (MM)" },
	{ 0x2FB, "Clock (Day 1)" },
	{ 0x2FC, "Clock (Night 1)" },
	{ 0x2FD, "Clock (Day 2)" },
	{ 0x2FE, "Clock (Night 2)" },
	{ 0x2FF, "Clock (Day 3)" },
	{ 0x300, "Clock (Night 3)" },
	{ 0x301, "Rupoor (MM)" },
	{ 0x302, "Megaton Hammer (MM)" },
	{ 0x303, "Deku Stick Upgrade (MM)" },
	{ 0x304, "Second Deku Stick Upgrade (MM)" },
	{ 0x305, "Deku Nut Upgrade (MM)" },
	{ 0x306, "Second Deku Nut Upgrade (MM)" },
	{ 0x307, "Stone of Agony (MM)" },
	{ 0x308, "Majora's Mask (MM)" },
	{ 0x309, "Sold Out" },
	{ 0x30A, "Nothing" },
	{ 0x30B, "MM Scoop" },



	/*
	{ 0x01, "5 Bombs (OoT)" },
	{ 0x02, "5 Deku Nuts (OoT)" },
	{ 0x03, "10 Bombchu (OoT)" },
	{ 0x04, "Fairy Bow (OoT)" },
	{ 0x05, "Fairy Slingshot" },
	{ 0x06, "Boomerang" },
	{ 0x07, "Deku Stick (OoT)" },
	{ 0x08, "Hookshot (Oot)" },
	{ 0x09, "Longshot (OoT)" },
	{ 0x0A, "Lens of Truth (OoT)" },
	{ 0x0B, "Zelda's Letter" },
	{ 0x0C, "Ocarina of Time (OoT)" },
	{ 0x0D, "Megaton Hammer (OoT)" },
	{ 0x0E, "Cojiro" },
	{ 0x0F, "Empty Bottle (OoT)" },
	{ 0x10, "Red Potion (OoT)" },
	{ 0x11, "Green Potion (OoT)" },
	{ 0x12, "Blue Potion (OoT)" },
	{ 0x13, "Fairy (OoT)" },
	{ 0x14, "Lon Lon Milk (OoT)" },
	{ 0x15, "Ruto's Letter" },
	{ 0x16, "Magic Bean (OoT)" },
	{ 0x17, "Skull Mask" },
	{ 0x18, "Spooky Mask" },
	{ 0x19, "Chicken" },
	{ 0x1A, "Keaton Mask (OoT)" },
	{ 0x1B, "Bunny Hood (OoT)" },
	{ 0x1C, "Mask of Truth (OoT)" },
	{ 0x1D, "Pocket Egg" },
	{ 0x1E, "Pocket Cucco" },
	{ 0x1F, "Odd Mushroom" },
	{ 0x20, "Odd Potion" },
	{ 0x21, "Poacher's Saw" },
	{ 0x22, "Broken Goron's Sword" },
	{ 0x23, "Prescription" },
	{ 0x24, "Eyeball Frog" },
	{ 0x25, "Eye Drops" },
	{ 0x26, "Claim Check" },
	{ 0x27, "Kokiri's Sword (OoT)" },
	{ 0x28, "Giant's Knife (OoT)" },
	{ 0x29, "Deku Shield (OoT)" },
	{ 0x2A, "Hylian Shield (OoT)" },
	{ 0x2B, "Mirror Shield (OoT)" },
	{ 0x2C, "Goron Tunic (OoT)" },
	{ 0x2D, "Zora Tunic (OoT)" },
	{ 0x2E, "Iron Boots (OoT)" },
	{ 0x2F, "Hover Boots (OoT)" },
	{ 0x30, "Big Quiver (OoT)" },
	{ 0x31, "Biggest Quiver (OoT)" },
	{ 0x32, "Bomb Bag (OoT)" },
	{ 0x33, "Big Bomb Bag (OoT)" },
	{ 0x34, "Biggest Bomb Bag (OoT)" },
	{ 0x35, "Silver Gauntlets (OoT)" },
	{ 0x36, "Golden Gauntlets (OoT)" },
	{ 0x37, "Silver Scale (OoT)" },
	{ 0x38, "Golden Scale (OoT)" },
	{ 0x39, "Stone of Agony (OoT)" },
	{ 0x3A, "Gerudo's Card" },
	{ 0x3B, "Fairy Ocarina (OoT)" },
	{ 0x3C, "5 Deku Seeds (OoT)" },
	{ 0x3D, "Heart Container (OoT)" },
	{ 0x3E, "Piece of Heart (OoT)" },
	{ 0x3F, "Boss Key (OoT)" },
	{ 0x40, "Compass (OoT)" },
	{ 0x41, "Dungeon Map (OoT)" },
	{ 0x42, "Small Key (OoT)" },
	{ 0x43, "Small Magic Jar (OoT)" },
	{ 0x44, "Large Magic Jar (OoT)" },
	{ 0x45, "Adult's Wallet (OoT)" },
	{ 0x46, "Giant's Wallet (OoT)" },
	{ 0x47, "Weird Egg" },
	{ 0x48, "Recovery Heart (OoT)" },
	{ 0x49, "5 Arrows (OoT)" },
	{ 0x4A, "10 Arrows (OoT)" },
	{ 0x4B, "30 Arrows (OoT)" },
	{ 0x4C, "Green Rupee (OoT)" },
	{ 0x4D, "Blue Rupee (OoT)" },
	{ 0x4E, "Red Rupee (OoT)" },
	{ 0x4F, "Heart Container (OoT)" },
	{ 0x50, "Lon Lon Milk (OoT)" },
	{ 0x51, "Goron Mask (OoT)" },
	{ 0x52, "Zora Mask (OoT)" },
	{ 0x53, "Gerudo Mask" },
	{ 0x54, "Goron's Bracelet (OoT)" },
	{ 0x55, "Purple Rupee (OoT)" },
	{ 0x56, "Gold Rupee (OoT)" },
	{ 0x57, "Biggoron’s Sword" },
	{ 0x58, "Fire Arrow (OoT)" },
	{ 0x59, "Ice Arrow (OoT)" },
	{ 0x5A, "Light Arrow (OoT)" },
	{ 0x5B, "Gold Skulltula Token" },
	{ 0x5C, "Din's Fire (OoT)" },
	{ 0x5D, "Farore's Wind (OoT)" },
	{ 0x5E, "Nayru's Love (OoT)" },
	{ 0x5F, "Deku Seeds Bullet Bag (40)" },
	{ 0x60, "Large Bullet Bag" },
	{ 0x61, "5 Deku Sticks (OoT)" },
	{ 0x62, "10 Deku Sticks (OoT)" },
	{ 0x63, "5 Deku Nuts (OoT)" },
	{ 0x64, "10 Deku Nuts (OoT)" },
	{ 0x65, "1 Bomb (OoT)" },
	{ 0x66, "10 Bomb (OoT)" },
	{ 0x67, "20 Bombs (OoT)" },
	{ 0x68, "30 Bombs (OoT)" },
	{ 0x69, "30 Deku Seeds (OoT)" },
	{ 0x6A, "5 Bombchu (OoT)" },
	{ 0x6B, "20 Bombchu (OoT)" },
	{ 0x6C, "Fish (OoT)" },
	{ 0x6D, "Bugs (OoT)" },
	{ 0x6E, "Blue Fire" },
	{ 0x6F, "Bottled Poe" },
	{ 0x70, "Bottled Big Poe" },
	{ 0x71, "Door Key (OoT Treasure Box Shop)" },
	{ 0x72, "Loser Green Rupee (OoT Treasure Box Shop)" },
	{ 0x73, "Loser Blue Rupee (OoT Treasure Box Shop)" },
	{ 0x74, "Loser Red Rupee (OoT Treasure Box Shop)" },
	{ 0x75, "Winner Purple Rupee (OoT Treasure Box Shop)" },
	{ 0x76, "Winner Piece of Heart (OoT Treasure Box Shop)" },
	{ 0x77, "First Deku Stick Upgrade (OoT)" },
	{ 0x78, "Second Deku Stick Upgrade (OoT)" },
	{ 0x79, "First Deku Nut Upgrade (OoT)" },
	{ 0x7A, "Second Deku Nut Upgrade (OoT)" },
	{ 0x7B, "Deku Seeds Bullet Bag (50)" },
	{ 0x7C, "Ice Trap (OoT)" },
	{ 0x7D, "Unknown" },
	{ 0x7E, "Unknown" },
	{ 0x7F, "Unknown" },
	{ 0x7D, "Rainbow Rupee" }
	{ 0x7E, "Minuet of Forest" }
	{ 0x7F, "Bolero of Fire" }
	{ 0x80, "Serenade of Water" }
	{ 0x81, "Requiem of Spirit" }
	{ 0x82, "Nocturne of Shadow" }
	{ 0x83, "Prelude of Light" }
	{ 0x84, "Zelda's Song" }
	{ 0x85, "Epona's Song (OoT)" }
	{ 0x86, "Saria's Song" }
	{ 0x87, "Sun's Song (OoT)" }
	{ 0x88, "Song of Time (OoT)" }
	{ 0x89, "Song of Storms (OoT)" }
	{ 0x8A, "Master Sword (OoT)" }
	{ 0x8B, "Kokiri's Emerald" }
	{ 0x8C, "Goron's Ruby" }
	{ 0x8D, "Zora's Sapphire" }
	{ 0x8E, "Forest Medallion" }
	{ 0x8F, "Fire Medallion" }
	{ 0x90, "Water Medallion" }
	{ 0x91, "Spirit Medallion" }
	{ 0x92, "Shadow Medallion" }
	{ 0x93, "Light Medallion" }
	{ 0x94, "Magic Upgrade (OoT)" }
	{ 0x95, "Larger Magic Updgrade (OoT)" }
	{ 0x96, "Defense Upgrade (OoT)" }
	{ 0x97, "Progressive Deku Shield (OoT)" }
	{ 0x98, "Progressive Hylian Shield (OoT)" }
	{ 0x99, "Small Key (Forest Temple)" }
	{ 0x9A, "Small Key (Fire Temple)" }
	{ 0x9B, "Small Key (Water Temple)" }
	{ 0x9C, "Small Key (Spirit Temple)" }
	{ 0x9D, "Small Key (Shadow Temple)" }
	{ 0x9E, "Small Key (Ganon's Castle)" }
	{ 0x9F, "Small Key (Bottom of the Well)" }
	{ 0xA0, "Small Key (Gerudo's Fortress)" }
	{ 0xA1, "Small Key (Gerudo's Training Ground)" }
	{ 0xA2, "Boss Key (Forest Temple)" }
	{ 0xA3, "Boss Key (Fire Temple)" }
	{ 0xA4, "Boss Key (Water Temple)" }
	{ 0xA5, "Boss Key (Spirit Temple)" }
	{ 0xA6, "Boss Key (Shadow Temple)" }
	{ 0xA7, "Boss Key (Ganon's Castle)" }
	{ 0xA8, "Dungeon Map (Deku Tree)" }
	{ 0xA9, "Dungeon Map (Dodongo's Cavern)" }
	{ 0xAA, "Dungeon Map (Jabu-Jabu's Belly)" }
	{ 0xAB, "Dungeon Map (Forest Temple)" }
	{ 0xAC, "Dungeon Map (Fire Temple)" }
	{ 0xAD, "Dungeon Map (Water Temple)" }
	{ 0xAE, "Dungeon Map (Spirit Temple)" }
	{ 0xAF, "Dungeon Map (Shadow Temple)" }
	{ 0xB0, "Dungeon Map (Bottom of the Well)" }
	{ 0xB1, "Dungeon Map (Ice Cavern)" }
	{ 0xB2, "Compass (Deku Tree)" }
	{ 0xB3, "Compass (Dodongo's Cavern)" }
	{ 0xB4, "Compass (Jabu-Jabu's Belly)" }
	{ 0xB5, "Compass (Forest Temple)" }
	{ 0xB6, "Compass (Fire Temple)" }
	{ 0xB7, "Compass (Water Temple)" }
	{ 0xB8, "Compass (Spirit Temple)" }
	{ 0xB9, "Compass (Shadow Temple)" }
	{ 0xBA, "Compass (Bottom of the Well)" }
	{ 0xBB, "Compass (Ice Cavern)" }
	{ 0xBC, "Child's Wallet (OoT)" }
	{ 0xBD, "Colossal Wallet (OoT)" }
	{ 0xBE, "Triforce Piece" }
	{ 0xBF, "Bottomless Wallet (OoT)" }
	{ 0xC0, "Silver Rupee (Dodongo's Cavern)" }
	{ 0xC1, "Silver Rupee (Bottom of the Well)" }
	{ 0xC2, "Silver Rupee (Spirit Child)" }
	{ 0xC3, "Silver Rupee (Spirit Sun)" }
	{ 0xC4, "Silver Rupee (Spirit Boulders)" }
	{ 0xC5, "Silver Rupee (Spirit Lobby)" }
	{ 0xC6, "Silver Rupee (Spirit Adult)" }
	{ 0xC7, "Silver Rupee (Shadow Scythe)" }
	{ 0xC8, "Silver Rupee (Shadow Pit)" }
	{ 0xC9, "Silver Rupee (Shadow Spikes)" }
	{ 0xCA, "Silver Rupee (Shadow Blades)" }
	{ 0xCB, "Silver Rupee (Ice Scythe)" }
	{ 0xCC, "Silver Rupee (Ice Block)" }
	{ 0xCD, "Silver Rupee (GTG Slopes)" }
	{ 0xCE, "Silver Rupee (GTG Lava)" }
	{ 0xCF, "Silver Rupee (GTG Water)" }
	{ 0xD0, "Silver Rupee (Ganon Spirit)" }
	{ 0xD1, "Silver Rupee (Ganon Light)" }
	{ 0xD2, "Silver Rupee (Ganon Fire)" }
	{ 0xD3, "Silver Rupee (Ganon Forest)" }
	{ 0xD4, "Silver Rupee (Ganon Shadow)" }
	{ 0xD5, "Silver Rupee (Ganon Water)" }
	{ 0xD6, "Soul of Stalfos (OoT)" }
	{ 0xD7, "Soul of Octoroks (OoT)" }
	{ 0xD8, "Soul of Wallmasters (OoT)" }
	{ 0xD9, "Soul of Dodongos (OoT)" }
	{ 0xDA, "Soul of Keeses (OoT)" }
	{ 0xDB, "Soul of Tektites (OoT)" }
	{ 0xDC, "Soul of Leevers (OoT)" }
	{ 0xDD, "Soul of Peahats (OoT)" }
	{ 0xDE, "Soul of Lizalfos / Dinolfos (OoT)" }
	{ 0xDF, "Soul of Gohma Larvae" }
	{ 0xE0, "Soul of Shaboms" }
	{ 0xE1, "Soul of Baby Dodongos" }
	{ 0xE2, "Soul of Dark Link" }
	{ 0xE3, "Soul of Biris / Baris" }
	{ 0xE4, "Soul of Tailpasarans" }
	{ 0xE5, "Soul of Jabu-Jabu's Parasites" }
	{ 0xE6, "Soul of Skulltulas (OoT)" }
	{ 0xE7, "Soul of Torch Slugs" }
	{ 0xE8, "Soul of Moblins" }
	{ 0xE9, "Soul of Armos (OoT)" }
	{ 0xEA, "Soul of Deku Babas (OoT)" }
	{ 0xEB, "Soul of Deku Scrubs (OoT)" }
	{ 0xEC, "Soul of Bubbles (OoT)" }
	{ 0xED, "Soul of Beamos (OoT)" }
	{ 0xEE, "Soul of Floormasters (OoT)" }
	{ 0xEF, "Soul of ReDeads / Gibdos (OoT)" }
	{ 0xF0, "Soul of Skullwalltulas (OoT)" }
	{ 0xF1, "Soul of Flare Dancers" }
	{ 0xF2, "Soul of Dead Hands" }
	{ 0xF3, "Soul of Shell Blades (OoT)" }
	{ 0xF4, "Soul of Like Likes (OoT)" }
	{ 0xF5, "Soul of Spikes" }
	{ 0xF6, "Soul of Anubis" }
	{ 0xF7, "Soul of Iron Knuckles (OoT)" }
	{ 0xF8, "Soul of Skull Kids (OoT)" }
	{ 0xF9, "Soul of Flying Pots (OoT)" }
	{ 0xFA, "Soul of Freezards (OoT)" }
	{ 0xFB, "Soul of Stingers" }
	{ 0xFC, "Soul of Wolfoses (OoT)" }
	{ 0xFD, "Soul of Stalchildren (OoT)" }
	{ 0xFE, "Soul of Guays (OoT)" }
	{ 0xFF, "Soul of the Fighting Gerudo" }
	{ 0x100, "Soul of Queen Gohma" }
	{ 0x101, "Soul of King Dodongo" }
	{ 0x102, "Soul of Barinade" }
	{ 0x103, "Soul of Phantom Ganon" }
	{ 0x104, "Soul of Volvagia" }
	{ 0x105, "Soul of Morpha" }
	{ 0x106, "Soul of Bongo-Bongo" }
	{ 0x107, "Soul of Twinrova" }
	{ 0x108, "Soul of Gold Skulltulas (OoT)" }
	{ 0x109, "Soul of Business Scrubs (OoT)" }
	{ 0x10A, "Key Ring (OoT)" }
	{ 0x10B, "Key Ring (Forest Temple)" }
	{ 0x10C, "Key Ring (Fire Temple)" }
	{ 0x10D, "Key Ring (Water Temple)" }
	{ 0x10E, "Key Ring (Shadow Temple)" }
	{ 0x10F, "Key Ring (Spirit Temple)" }
	{ 0x110, "Key Ring (Ganon)" }
	{ 0x111, "Key Ring (Bottom of the Well)" }
	{ 0x112, "Key Ring (Thieves Hideout)" }
	{ 0x113, "Key Ring (Gerudo Training Ground)" }
	{ 0x114, "Skeleton Key (OoT)" }
	{ 0x115, "Red Coin (OoT)" }
	{ 0x116, "Green Coin (OoT)" }
	{ 0x117, "Blue Coin (OoT)" }
	{ 0x118, "Yellow Coin (OoT)" }
	{ 0x119, "Triforce of Power" }
	{ 0x11A, "Triforce of Courage" }
	{ 0x11B, "Triforce of Wisdom" }
	{ 0x11C, "Silver Pouch (Dodongo's Cavern)" }
	{ 0x11D, "Silver Pouch (Bottom of the Well)" }
	{ 0x11E, "Silver Pouch (Spirit Child)" }
	{ 0x11F, "Silver Pouch (Spirit Sun)" }
	{ 0x120, "Silver Pouch (Spirit Boulders)" }
	{ 0x121, "Silver Pouch (Spirit Lobby)" }
	{ 0x122, "Silver Pouch (Spirit Adult)" }
	{ 0x123, "Silver Pouch (Shadow Scythe)" }
	{ 0x124, "Silver Pouch (Shadow Pit)" }
	{ 0x125, "Silver Pouch (Shadow Spikes)" }
	{ 0x126, "Silver Pouch (Shadow Blades)" }
	{ 0x127, "Silver Pouch (Ice Scythe)" }
	{ 0x128, "Silver Pouch (Ice Block)" }
	{ 0x129, "Silver Pouch (GTG Slopes)" }
	{ 0x12A, "Silver Pouch (GTG Lava)" }
	{ 0x12B, "Silver Pouch (GTG Water)" }
	{ 0x12C, "Silver Pouch (Ganon Spirit)" }
	{ 0x12D, "Silver Pouch (Ganon Light)" }
	{ 0x12E, "Silver Pouch (Ganon Fire)" }
	{ 0x12F, "Silver Pouch (Ganon Forest)" }
	{ 0x130, "Silver Pouch (Ganon Shadow)" }
	{ 0x131, "Silver Pouch (Ganon Water)" }
	{ 0x132, "Magical Rupee" }
	{ 0x133, "A Button (OoT)" }
	{ 0x134, "C-Right Button (OoT)" }
	{ 0x135, "C-Left Button (OoT)" }
	{ 0x136, "C-Up Button (OoT)" }
	{ 0x137, "C-Down Button (OoT)" }
	{ 0x138, "Samll Key (Chest Game)" }
	{ 0x139, "Key Ring (Chest Game)" }
	{ 0x13A, "Soul of Saria" }
	{ 0x13B, "Soul of Darunia" }
	{ 0x13C, "Soul of Ruto (OoT)" }
	{ 0x13D, "Soul of King Zora" }
	{ 0x13E, "Soul of Zelda" }
	{ 0x13F, "Soul of Sheik" }
	{ 0x140, "Soul of Cucco Lady (OoT)" }
	{ 0x141, "Soul of Carpenters (OoT)" }
	{ 0x142, "Soul of Guru-Guru (OoT)" }
	{ 0x143, "Soul of Mido" }
	{ 0x144, "Soul of Kokiri" }
	{ 0x145, "Soul of Kokiri Shopkeeper" }
	{ 0x146, "Soul of Hylian Guard" }
	{ 0x147, "Soul of Citizens (OoT)" }
	{ 0x148, "Soul of Malon (OoT)" }
	{ 0x149, "Soul of Talon (OoT)" }
	{ 0x14A, "Soul of Fishing Pond Owner (OoT)" }
	{ 0x14B, "Soul of Goron (OoT)" }
	{ 0x14C, "Soul of Medigoron (OoT)" }
	{ 0x14D, "Soul of Biggoron (OoT)" }
	{ 0x14E, "Soul of Child Goron (OoT)" }
	{ 0x14F, "Soul of Goron Shopkeeper (OoT)" }
	{ 0x150, "Soul of Zora (OoT)" }
	{ 0x151, "Soul of Zora Shopkeeper (OoT)" }
	{ 0x152, "Soul of Bazaar Shopkeeper (OoT)" }
	{ 0x153, "Soul of Bombchu Shopkeeper (OoT)" }
	{ 0x154, "Soul of Potion Shopkeeper" }
	{ 0x155, "Soul of Bombchu Bowling Lady (OoT)" }
	{ 0x156, "Soul of Chest Game Owner (OoT)" }
	{ 0x157, "Soul of Shooting Gallery Owner (OoT)" }
	{ 0x158, "Soul of Dampe (OoT)" }
	{ 0x159, "Soul of Graveyard Kid (OoT)" }
	{ 0x15A, "Soul of Poe Collector (OoT)" }
	{ 0x15B, "Soul of the Composer Bros. (OoT)}," }
	{ 0x15C, "Soul of Honey & Darling (OoT)" }
	{ 0x15D, "Soul of the Astronomer (OoT)" }
	{ 0x15E, "Soul of the Rooftop Man (OoT)" }
	{ 0x15F, "Soul of the Bean Salesman (OoT)" }
	{ 0x160, "Soul of the Scientist (OoT)" }
	{ 0x161, "Soul of Ingo (OoT)" }
	{ 0x162, "Soul of the Punk Boy (OoT)" }
	{ 0x163, "Soul of the Dog Lady (OoT)" }
	{ 0x164, "Soul of the Carpet Man (OoT)" }
	{ 0x165, "Soul of the Old Hag (OoT)" }
	{ 0x166, "Soul of the Beggar (OoT)" }
	{ 0x167, "Soul of the Patrolling Gerudos" }
	{ 0x168, "Bottled Red Potion (OoT)" }
	{ 0x169, "Bottled Green Potion (OoT)" }
	{ 0x16A, "Bottled Blue Potion (OoT)" }
	{ 0x16B, "Bottled Fairy (OoT)" }
	{ 0x16C, "Bottled Poe (OoT)" }
	{ 0x16D, "Bottled Big Poe (OoT)" }
	{ 0x16E, "Bottled Blue Fire (OoT)" }
	{ 0x16F, "Child Fish (2 pounds)" }
	{ 0x170, "Child Fish (3 pounds)" }
	{ 0x171, "Child Fish (4 pounds)" }
	{ 0x172, "Child Fish (5 pounds)" }
	{ 0x173, "Child Fish (6 pounds)" }
	{ 0x174, "Child Fish (7 pounds)" }
	{ 0x175, "Child Fish (8 pounds)" }
	{ 0x176, "Child Fish (9 pounds)" }
	{ 0x177, "Child Fish (10 pounds)" }
	{ 0x178, "Child Fish (11 pounds)" }
	{ 0x179, "Child Fish (12 pounds)" }
	{ 0x17A, "Child Fish (13 pounds)" }
	{ 0x17B, "Child Fish (14 pounds)" }
	{ 0x17C, "Adult Fish (4 pounds)" }
	{ 0x17D, "Adult Fish (5 pounds)" }
	{ 0x17E, "Adult Fish (6 pounds)" }
	{ 0x17F, "Adult Fish (7 pounds)" }
	{ 0x180, "Adult Fish (8 pounds)" }
	{ 0x181, "Adult Fish (9 pounds)" }
	{ 0x182, "Adult Fish (10 pounds)" }
	{ 0x183, "Adult Fish (11 pounds)" }
	{ 0x184, "Adult Fish (12 pounds)" }
	{ 0x185, "Adult Fish (13 pounds)" }
	{ 0x186, "Adult Fish (14 pounds)" }
	{ 0x187, "Adult Fish (15 pounds)" }
	{ 0x188, "Adult Fish (16 pounds)" }
	{ 0x189, "Adult Fish (17 pounds)" }
	{ 0x18A, "Adult Fish (18 pounds)" }
	{ 0x18B, "Adult Fish (19 pounds)" }
	{ 0x18C, "Adult Fish (20 pounds)" }
	{ 0x18D, "Adult Fish (21 pounds)" }
	{ 0x18E, "Adult Fish (22 pounds)" }
	{ 0x18F, "Adult Fish (23 pounds)" }
	{ 0x190, "Adult Fish (24 pounds)" }
	{ 0x191, "Adult Fish (25 pounds)" }
	{ 0x192, "Child Loach (14 Pounds)" }
	{ 0x193, "Child Loach (15 Pounds)" }
	{ 0x194, "Child Loach (16 Pounds)" }
	{ 0x195, "Child Loach (17 Pounds)" }
	{ 0x196, "Child Loach (18 Pounds)" }
	{ 0x197, "Child Loach (19 Pounds)" }
	{ 0x198, "Adult Loach (29 pounds)" }
	{ 0x199, "Adult Loach (30 pounds)" }
	{ 0x19A, "Adult Loach (31 pounds)" }
	{ 0x19B, "Adult Loach (32 pounds)" }
	{ 0x19C, "Adult Loach (33 pounds)" }
	{ 0x19D, "Adult Loach (34 pounds)" }
	{ 0x19E, "Adult Loach (35 pounds)" }
	{ 0x19F, "Adult Loach (36 pounds)" }
	{ 0x1A0, "Bombchu Bag (OoT)" }
	{ 0x1A1, "Big Bombchu Bag (OoT)" }
	{ 0x1A2, "Biggest Bombchu Bag (OoT)" }
	{ 0x1A3, "Razor Sword (OoT)" }
	{ 0x1A4, "Gilded Sword (OoT)" }
	{ 0x1A5, "Spin Attack Upgrade (OoT)" }
	{ 0x1A6, "Triforce" }
	{ 0x1A7, "Big Fairy (OoT)" }
	{ 0x1A8, "Blast Mask (OoT)" }
	{ 0x1A9, "Stone Mask (OoT)" }
	{ 0x1AA, "Rupoor (OoT)" }
	{ 0x1AB, "Elegy of Emptiness (OoT)" }
	{ 0x1AC, "Green Rupee (MM)" }
	{ 0x1AD, "Blue Rupee (MM)" }
	{ 0x1AE, "Red Rupee (MM)" }
	{ 0x1AF, "Red Rupee (MM)" }
	{ 0x1B0, "Purple Rupee (MM)" }
	{ 0x1B1, "Silver Rupee (MM)" }
	{ 0x1B2, "Gold Rupee (MM)" }
	{ 0x1B3, "Adult's Wallet (MM)" }
	{ 0x1B4, "Giant's Wallet (MM)" }
	{ 0x1B5, "Recovery Heart (MM)" }
	{ 0x1B6, "Piece of Heart (MM)" }
	{ 0x1B7, "Heart Container (MM)" }
	{ 0x1B8, "Small Magic Jar (MM)" }
	{ 0x1B9, "Large Magic Jar (MM)" }
	{ 0x1BA, "Stray Fairy" }
	{ 0x1BB, "1 Bomb (MM)" }
	{ 0x1BC, "5 Bombs (MM)" }
	{ 0x1BD, "10 Bombs (MM)" }
	{ 0x1BE, "20 Bombs (MM)" }
	{ 0x1BF, "30 Bombs (MM)" }
	{ 0x1C0, "Deku Stick (MM)" }
	{ 0x1C1, "10 Bombchu (MM)" }
	{ 0x1C2, "Bomb Bag (MM)" }
	{ 0x1C3, "Big Bomb Bag (MM)" }
	{ 0x1C4, "Biggest Bomb Bag (MM)" }
	{ 0x1C5, "10 Arrows (MM)" }
	{ 0x1C6, "30 Arrows (MM)" }
	{ 0x1C7, "40 Arrows (MM)" }
	{ 0x1C8, "40 Arrows (MM)" }
	{ 0x1C9, "Hero's Bow (MM)" }
	{ 0x1CA, "Fairy's Bow (MM)" }
	{ 0x1CB, "Biggest Quiver (MM)" }
	{ 0x1CC, "Fire Arrow (MM)" }
	{ 0x1CD, "Ice Arrow (MM)" }
	{ 0x1CE, "Light Arrow (MM)" }
	{ 0x1CF, "Deku Nut (MM)" }
	{ 0x1D0, "5 Deku Nuts (MM)" }
	{ 0x1D1, "10 Deku Nuts (MM)" }
	{ 0x1D2, "30 Deku Nuts (MM)" }
	{ 0x1D3, "40 Deku Nuts (MM)" }
	{ 0x1D4, "20 Bombchu (MM)" }
	{ 0x1D5, "20 Deku Sticks" }
	{ 0x1D6, "20 Deku Sticks" }
	{ 0x1D7, "30 Deku Sticks" }
	{ 0x1D8, "Deku Shield (MM)" }
	{ 0x1D9, "Hero's Shield (MM)" }
	{ 0x1DA, "Mirror Shield (MM)" }
	{ 0x1DB, "Powder Keg" }
	{ 0x1DC, "Magic Bean (MM)" }
	{ 0x1DD, "1 Bombchu (MM)" }
	{ 0x1DE, "Kokiri Sword (MM)" }
	{ 0x1DF, "Razor Sword (MM)" }
	{ 0x1E0, "Gilded Sword (MM)" }
	{ 0x1E1, "5 Bombchu (MM)" }
	{ 0x1E2, "Great Fairy's Sword" }
	{ 0x1E3, "Small Key" }
	{ 0x1E4, "Boss Key" }
	{ 0x1E5, "Dungeon Map" }
	{ 0x1E6, "Compass" }
	{ 0x1E7, "Hookshot (MM)" }
	{ 0x1E8, "Lens of Truth (MM)" }
	{ 0x1E9, "Pictograph Box" }
	{ 0x1EA, "Ocarina of Time (MM)" }
	{ 0x1EB, "Bomber's Notebook" }
	{ 0x1EC, "Bottled Red Potion (MM)" }
	{ 0x1ED, "Empty Bottle (MM)" }
	{ 0x1EE, "Red Potion (MM)" }
	{ 0x1EF, "Green Potion (MM)" }
	{ 0x1F0, "Blue Potion (MM)" }
	{ 0x1F1, "Fairy (MM)" }
	{ 0x1F2, "Bottled Milk (MM)" }
	{ 0x1F3, "Fish (MM)" }
	{ 0x1F4, "Bug (MM)" }
	{ 0x1F5, "Big Poe (MM)" }
	{ 0x1F6, "Bottled Gold Dust" }
	{ 0x1F7, "Magic Mushroom" }
	{ 0x1F8, "Bottled Chateau Romani" }
	{ 0x1F9, "Deku Mask (MM)" }
	{ 0x1FA, "Goron Mask (MM)" }
	{ 0x1FB, "Zora Mask (MM)" }
	{ 0x1FC, "Fierce Deity's Mask" }
	{ 0x1FD, "Captain's Hat" }
	{ 0x1FE, "Giant's Mask" }
	{ 0x1FF, "All Night Mask" }
	{ 0x200, "Bunny Hood (MM)" }
	{ 0x201, "Keaton Mask (MM)" }
	{ 0x202, "Garo's Mask" }
	{ 0x203, "Romani's Mask" }
	{ 0x204, "Circus Leader's Mask" }
	{ 0x205, "Postman's Hat" }
	{ 0x206, "Couple's Mask" }
	{ 0x207, "Great Fairy's Mask" }
	{ 0x208, "Gibdo Mask" }
	{ 0x209, "Don Gero's Mask" }
	{ 0x20A, "Kamaro's Mask" }
	{ 0x20B, "Mask of Truth (MM)" }
	{ 0x20C, "Stone Mask (MM)" }
	{ 0x20D, "Bremen Mask" }
	{ 0x20E, "Blast Mask (MM)" }
	{ 0x20F, "Mask of Scents" }
	{ 0x210, "Kafei's Mask" }
	{ 0x211, "Chateau Romani (MM)" }
	{ 0x212, "Milk (MM)" }
	{ 0x213, "Gold Dust" }
	{ 0x214, "Seahorse" }
	{ 0x215, "Moon's Tear" }
	{ 0x216, "Land Title Deed" }
	{ 0x217, "Swamp Title Deed" }
	{ 0x218, "Mountain Title Deed" }
	{ 0x219, "Ocean Title Deed" }
	{ 0x21A, "Room Key" }
	{ 0x21B, "Letter to Mama" }
	{ 0x21C, "Letter to Kafei" }
	{ 0x21D, "Pendant of Memories" }
	{ 0x21E, "World Map (Clock Town)" }
	{ 0x21F, "World Map (Woodfall)" }
	{ 0x220, "World Map (Snowhead)" }
	{ 0x221, "World Map (Romani Ranch)" }
	{ 0x222, "World Map (Great Bay)" }
	{ 0x223, "World Map (Stone Tower)" }
	{ 0x224, "Magic Upgrade (MM)" }
	{ 0x225, "Larger Magic Upgrade (MM)" }
	{ 0x226, "Defense Upgrade (MM)" }
	{ 0x227, "Spin Attack Upgrade (MM)" }
	{ 0x228, "Sonata of Awakening" }
	{ 0x229, "Goron Lullaby" }
	{ 0x22A, "New Wave Bossa Nova" }
	{ 0x22B, "Elegy of Emptiness (MM)" }
	{ 0x22C, "Oath to Order" }
	{ 0x22D, "Song of Time (MM)" }
	{ 0x22E, "Song of Healing" }
	{ 0x22F, "Epona's Song (MM)" }
	{ 0x230, "Song of Soaring" }
	{ 0x231, "Song of Storms (MM)" }
	{ 0x232, "Goron Lullaby Intro" }
	{ 0x233, "Odolwa's Remains" }
	{ 0x234, "Goht's Remains" }
	{ 0x235, "Gyorg's Remains" }
	{ 0x236, "Twinmold's Remains" }
	{ 0x237, "Hero's Shield (MM)" }
	{ 0x238, "Swamp Skulltula Token" }
	{ 0x239, "Ocean Skulltula Token" }
	{ 0x23A, "Small Key (Woodfall Temple)" }
	{ 0x23B, "Small Key (Snowhead Temple)" }
	{ 0x23C, "Small Key (Great Bay Temple)" }
	{ 0x23D, "Small Key (Stone Tower Temple)" }
	{ 0x23E, "Boss Key (Woodfall Temple)" }
	{ 0x23F, "Boss Key (Snowhead Temple)" }
	{ 0x240, "Boss Key (Great Bay Temple)" }
	{ 0x241, "Boss Key (Stone Tower Temple)" }
	{ 0x242, "Stray Fairy (Woodfall Temple)" }
	{ 0x243, "Stray Fairy (Snowhead Temple)" }
	{ 0x244, "Stray Fairy (Great Bay Temple)" }
	{ 0x245, "Stray Fairy (Stone Tower Temple)" }
	{ 0x246, "Stray Fairy (Town)" }
	{ 0x247, "Dungeon Map (Woodfall Temple)" }
	{ 0x248, "Dungeon Map (Snowhead)" }
	{ 0x249, "Dungeon Map (Great Bay Temple)" }
	{ 0x24A, "Dungeon Map (Stone Tower Temple)" }
	{ 0x24B, "Compass (Woodfall Temple)" }
	{ 0x24C, "Compass (Snowhead Temple)" }
	{ 0x24D, "Compass (Great Bay Temple)" }
	{ 0x24E, "Compass (Stone Tower Temple)" }
	{ 0x24F, "Fairy Ocarina (MM)" }
	{ 0x250, "Short Hookshot (MM)" }
	{ 0x251, "Sun's Song (MM)" }
	{ 0x252, "Child's Wallet (MM)" }
	{ 0x253, "Colossal Wallet (MM)" }
	{ 0x254, "Owl Statue (Great Bay)" }
	{ 0x255, "Owl Statue (Zora Cape)" }
	{ 0x256, "Owl Statue (Snowhead)" }
	{ 0x257, "Owl Statue (Mountain Village)" }
	{ 0x258, "Owl Statue (Clock Town)" }
	{ 0x259, "Owl Statue (Milk Road)" }
	{ 0x25A, "Owl Statue (Woodfall)" }
	{ 0x25B, "Owl Statue (Southern Swamp)" }
	{ 0x25C, "Owl Statue (Ikana Canyon)" }
	{ 0x25D, "Owl Statue (Stone Tower)" }
	{ 0x25E, "Owl Statue (Hidden)" }
	{ 0x25F, "Bottomless Wallet" }
	{ 0x260, "Soul of Octoroks (MM)" }
	{ 0x261, "Soul of Wallmasters (MM)" }
	{ 0x262, "Soul of Dodongos (MM)" }
	{ 0x263, "Soul of Keeses (MM)" }
	{ 0x264, "Soul of Tektites (MM)" }
	{ 0x265, "Soul of Peahats (MM)" }
	{ 0x266, "Soul of Lizalfos / Dinalfos (MM)" }
	{ 0x267, "Soul of Skulltulas (MM)" }
	{ 0x268, "Soul of Armos (MM)" }
	{ 0x269, "Soul of Deku Babas (MM)" }
	{ 0x26A, "Soul of Deku Scrubs (MM)" }
	{ 0x26B, "Soul of Bubbles (MM)" }
	{ 0x26C, "Soul of Beamos (MM)" }
	{ 0x26D, "Soul of Redead / Gibdos (MM)" }
	{ 0x26E, "Soul of Skullwalltulas (MM)" }
	{ 0x26F, "Soul of Shell Blades (MM)" }
	{ 0x270, "Soul of Like-Likes (MM)" }
	{ 0x271, "Soul of Iron Knuckles (MM)" }
	{ 0x272, "Soul of Freezards (MM)" }
	{ 0x273, "Soul of Wolfos (MM)" }
	{ 0x274, "Soul of Guays (MM)" }
	{ 0x275, "Soul of Flying Pots (MM)" }
	{ 0x276, "Soul of Floormasters (MM)" }
	{ 0x277, "Soul of Chuchus" }
	{ 0x278, "Soul of Deep Pythons" }
	{ 0x279, "Soul of Stalchildren (MM)" }
	{ 0x27A, "Soul of Leevers (MM)" }
	{ 0x27B, "Soul of Skullfish (MM)" }
	{ 0x27C, "Soul of Dexihands" }
	{ 0x27D, "Soul of Dragonflies" }
	{ 0x27E, "Soul of Eenos" }
	{ 0x27F, "Soul of Eyegores" }
	{ 0x280, "Soul of Hiploops" }
	{ 0x281, "Soul of Real Bombchu" }
	{ 0x282, "Soul of Takkuri" }
	{ 0x283, "Soul of Boes" }
	{ 0x284, "Soul of Neijirons" }
	{ 0x285, "Soul of Bio Babas" }
	{ 0x286, "Soul of Garo" }
	{ 0x287, "Soul of Wizzrobes" }
	{ 0x288, "Soul of Gomess" }
	{ 0x289, "Soul of Gekkos" }
	{ 0x28A, "Soul of Bad Bats" }
	{ 0x28B, "Soul of Snappers" }
	{ 0x28C, "Soul of Warts" }
	{ 0x28D, "Soul of Captain Keeta" }
	{ 0x28E, "Soul of the Fighting Pirates" }
	{ 0x28F, "Soul of Odolwa" }
	{ 0x290, "Soul of Goht" }
	{ 0x291, "Soul of Gyorg" }
	{ 0x292, "Soul of Twinmold" }
	{ 0x293, "Soul of Igos" }
	{ 0x294, "Soul of Madame Aroma" }
	{ 0x295, "Soul of Town Archery Owner (MM)" }
	{ 0x296, "Soul of Swamp Archery Owner (MM)" }
	{ 0x297, "Soul of Gorons (MM)" }
	{ 0x298, "Soul of the Goron Baby (MM)" }
	{ 0x299, "Soul of the Bomb Shop Owner (MM)" }
	{ 0x29A, "Soul of Bombers' Kids" }
	{ 0x29B, "Soul of the Deku Butler" }
	{ 0x29C, "Soul of Citizens (MM)" }
	{ 0x29D, "Soul of Composer Bros. (MM)" }
	{ 0x29E, "Soul of Dampe (MM)" }
	{ 0x29F, "Soul of Deku King" }
	{ 0x2A0, "Soul of the Deku Princess" }
	{ 0x2A1, "Soul of Playground Scrubs" }
	{ 0x2A2, "Soul of Goron Elder" }
	{ 0x2A3, "Soul of Fisherman (MM)" }
	{ 0x2A4, "Soul of Goron Shopkeeper (MM)" }
	{ 0x2A5, "Soul of Honey & Darling (MM)" }
	{ 0x2A6, "Soul of Toto (MM)" }
	{ 0x2A7, "Soul of Lulu" }
	{ 0x2A8, "Soul of Zora Musicians" }
	{ 0x2A9, "Soul of Kafei" }
	{ 0x2AA, "Soul of Keaton" }
	{ 0x2AB, "Soul of Koume & Kotake" }
	{ 0x2AC, "Soul of Mayor Dotour" }
	{ 0x2AD, "Soul of Keg Trial Goron (MM)" }
	{ 0x2AE, "Soul of Biggoron (MM)" }
	{ 0x2AF, "Soul of Mr. Barten (MM)" }
	{ 0x2B0, "Soul of Astronomer (MM)" }
	{ 0x2B1, "Soul of Poe Hut Owner (MM)" }
	{ 0x2B2, "Soul of Blacksmiths" }
	{ 0x2B3, "Soul of Chest Game Lady (MM)" }
	{ 0x2B4, "Soul of Tingle" }
	{ 0x2B5, "Soul of Toilet Hand" }
	{ 0x2B6, "Soul of Trading Post Owner (MM)" }
	{ 0x2B7, "Soul of the Part-Timer (MM)" }
	{ 0x2B8, "Soul of Zoras (MM)" }
	{ 0x2B9, "Soul of the Zora Shopkeeper (MM)" }
	{ 0x2BA, "Soul of Romani / Cremia" }
	{ 0x2BB, "Soul of the Bean Salesman (MM)" }
	{ 0x2BC, "Soul of Carpenters (MM)" }
	{ 0x2BD, "Soul of Anju (MM)" }
	{ 0x2BE, "Soul of Guru-Guru (MM)" }
	{ 0x2BF, "Soul of Scientist (MM)" }
	{ 0x2C0, "Soul of Gorman & Bros. (MM)" }
	{ 0x2C1, "Soul of Grog (MM)" }
	{ 0x2C2, "Soul of the Dog Lady (MM)" }
	{ 0x2C3, "Soul of Swordsman (MM)" }
	{ 0x2C4, "Soul of Anju's Grandmother (MM)" }
	{ 0x2C5, "Soul of Tourist Center Owner" }
	{ 0x2C6, "Soul of the Banker (MM)" }
	{ 0x2C7, "Soul of Moon Children" }
	{ 0x2C8, "Soul of Patrolling Pirates and Their Chief" }
	{ 0x2C9, "Soul of Gold Skulltulas (MM)" }
	{ 0x2CA, "Soul of Business Scrubs (MM)" }
	{ 0x2CB, "Key Ring" }
	{ 0x2CC, "Key Ring (Woodfall Temple)" }
	{ 0x2CD, "Key Ring (Snowhead Temple)" }
	{ 0x2CE, "Key Ring (Great Bay)" }
	{ 0x2CF, "Key Ring (Stone Tower)" }
	{ 0x2D0, "Skeleton Key (MM)" }
	{ 0x2D1, "A Button (MM)" }
	{ 0x2D2, "C-Right Button (MM)" }
	{ 0x2D3, "C-Left Button (MM)" }
	{ 0x2D4, "C-Up Button (MM)" }
	{ 0x2D5, "C-Down Button (MM)" }
	{ 0x2D6, "Bottled Green Potion (MM)" }
	{ 0x2D7, "Bottled Blue Potion (MM)" }
	{ 0x2D8, "Bottled Fairy (MM)" }
	{ 0x2D9, "Poe (MM)" }
	{ 0x2DA, "Bottled Poe (MM)" }
	{ 0x2DB, "Bottled Big Poe (MM)" }
	{ 0x2DC, "Bombchu Bag (MM)" }
	{ 0x2DD, "Big Bombchu Bag (MM)" }
	{ 0x2DE, "Biggest Bombchu Bag (MM)" }
	{ 0x2DF, "Unknown" }
	{ 0x2E0, "Big Fairy (MM)" }
	{ 0x2E1, "Din's Fire (MM)" }
	{ 0x2E2, "Farore's Wind (MM)" }
	{ 0x2E3, "Nayru's Love (MM)" }
	{ 0x2E4, "Iron Boots (MM)" }
	{ 0x2E5, "Hover Boots (MM)" }
	{ 0x2E6, "Goron Tunic (MM)" }
	{ 0x2E7, "Zora Tunic (MM)" }
	{ 0x2E8, "Silver Scale (MM)" }
	{ 0x2E9, "Golden Scale (MM)" }
	{ 0x2EA, "Goron's Bracelet (MM)" }
	{ 0x2EB, "Silver Gauntlets (MM)" }
	{ 0x2EC, "Golden Gauntlets (MM)" }
	{ 0x2ED, "Clock (Day 1)" }
	{ 0x2EE, "Clock (Night 1)" }
	{ 0x2EF, "Clock (Day 2)" }
	{ 0x2F0, "Clock (Night 2)" }
	{ 0x2F1, "Clock (Day 3)" }
	{ 0x2F2, "Clock (Night 3)" }
	{ 0x2F3, "Rupoor (MM)" }
	{ 0x2F4, "Megaton Hammer (MM)" }
	{ 0x2F5, "First Deku Stick Upgrade (MM)" }
	{ 0x2F6, "Second Deku Stick Upgrade (MM)" }
	{ 0x2F7, "Deku Nut Upgrade (MM)" }
	{ 0x2F8, "Second Deku Nut Upgrade (MM)" }
	{ 0x2F9, "Stone of Agony (MM)" }
	{ 0x2FA, "Majora's Mask" }
	{ 0x2FB, "Sold Out" }
	{ 0x2FC, "Nothing" }*/
};

#pragma endregion 

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