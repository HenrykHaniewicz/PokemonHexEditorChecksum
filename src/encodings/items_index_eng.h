#ifndef ITEMS_INDEX_ENG_H
#define ITEMS_INDEX_ENG_H

#include <cstdint>
#include <unordered_map>

namespace ItemsIndex {

// ============================================================================
// Generation 2 Pocket Indices
// ============================================================================

enum Gen2Pocket : uint8_t {
    POCKET_ITEMS = 0,
    POCKET_BALLS = 1,
    POCKET_KEY_ITEMS = 2,
    POCKET_TM_HM = 3,
    POCKET_UNKNOWN = 255
};

// ============================================================================
// Generation 2 Item Info Structure
// ============================================================================

struct Gen2ItemInfo {
    const char* nameGS;         // Gold/Silver name
    const char* nameCrystal;    // Crystal name (nullptr if same as GS)
    uint8_t pocket;
};

// ============================================================================
// Generation 3 Pocket Indices
// ============================================================================

enum Gen3Pocket : uint8_t {
    GEN3_POCKET_ITEMS = 0,
    GEN3_POCKET_BALLS = 1,
    GEN3_POCKET_KEY_ITEMS = 2,
    GEN3_POCKET_TM_HM = 3,
    GEN3_POCKET_BERRIES = 4,
    GEN3_POCKET_UNKNOWN = 255
};

// ============================================================================
// Generation 3 Game Availability Flags
// ============================================================================

enum Gen3GameFlags : uint8_t {
    GEN3_GAME_NONE = 0x00,
    GEN3_GAME_RS   = 0x01,    // Ruby/Sapphire
    GEN3_GAME_E    = 0x02,    // Emerald
    GEN3_GAME_FRLG = 0x04,    // FireRed/LeafGreen
    GEN3_GAME_RSE  = 0x03,    // Ruby/Sapphire/Emerald
    GEN3_GAME_ALL  = 0x07     // All Gen 3 games
};

// ============================================================================
// Generation 3 Item Info Structure
// ============================================================================

struct Gen3ItemInfo {
    const char* name;
    uint8_t pocket;
    uint8_t gameFlags;
};

// ============================================================================
// Generation 1 Items
// ============================================================================

// Sparse map - only valid items are included
// Invalid items (gaps in the index) are not represented
static const std::unordered_map<uint8_t, const char*> GEN1_ITEMS = {
    {0x01, "Master Ball"},
    {0x02, "Ultra Ball"},
    {0x03, "Great Ball"},
    {0x04, "Poke Ball"},
    {0x05, "Town Map"},
    {0x06, "Bicycle"},
    {0x07, "?????"},
    {0x08, "Safari Ball"},
    {0x09, "Pokedex"},
    {0x0A, "Moon Stone"},
    {0x0B, "Antidote"},
    {0x0C, "Burn Heal"},
    {0x0D, "Ice Heal"},
    {0x0E, "Awakening"},
    {0x0F, "Parlyz Heal"},
    {0x10, "Full Restore"},
    {0x11, "Max Potion"},
    {0x12, "Hyper Potion"},
    {0x13, "Super Potion"},
    {0x14, "Potion"},
    {0x15, "BoulderBadge"},
    {0x16, "CascadeBadge"},
    {0x17, "ThunderBadge"},
    {0x18, "RainbowBadge"},
    {0x19, "SoulBadge"},
    {0x1A, "MarshBadge"},
    {0x1B, "VolcanoBadge"},
    {0x1C, "EarthBadge"},
    {0x1D, "Escape Rope"},
    {0x1E, "Repel"},
    {0x1F, "Old Amber"},
    {0x20, "Fire Stone"},
    {0x21, "Thunderstone"},
    {0x22, "Water Stone"},
    {0x23, "HP Up"},
    {0x24, "Protein"},
    {0x25, "Iron"},
    {0x26, "Carbos"},
    {0x27, "Calcium"},
    {0x28, "Rare Candy"},
    {0x29, "Dome Fossil"},
    {0x2A, "Helix Fossil"},
    {0x2B, "Secret Key"},
    {0x2C, "?????"},
    {0x2D, "Bike Voucher"},
    {0x2E, "X Accuracy"},
    {0x2F, "Leaf Stone"},
    {0x30, "Card Key"},
    {0x31, "Nugget"},
    {0x32, "PP Up"},          // Note: Listed as "PP Up*" in source, duplicate of 0x4F
    {0x33, "Poke Doll"},
    {0x34, "Full Heal"},
    {0x35, "Revive"},
    {0x36, "Max Revive"},
    {0x37, "Guard Spec."},
    {0x38, "Super Repel"},
    {0x39, "Max Repel"},
    {0x3A, "Dire Hit"},
    {0x3B, "Coin"},
    {0x3C, "Fresh Water"},
    {0x3D, "Soda Pop"},
    {0x3E, "Lemonade"},
    {0x3F, "S.S. Ticket"},
    {0x40, "Gold Teeth"},
    {0x41, "X Attack"},
    {0x42, "X Defend"},
    {0x43, "X Speed"},
    {0x44, "X Special"},
    {0x45, "Coin Case"},
    {0x46, "Oak's Parcel"},
    {0x47, "Itemfinder"},
    {0x48, "Silph Scope"},
    {0x49, "Poke Flute"},
    {0x4A, "Lift Key"},
    {0x4B, "Exp. All"},
    {0x4C, "Old Rod"},
    {0x4D, "Good Rod"},
    {0x4E, "Super Rod"},
    {0x4F, "PP Up"},
    {0x50, "Ether"},
    {0x51, "Max Ether"},
    {0x52, "Elixer"},
    {0x53, "Max Elixer"},
    {0xC4, "HM01"},
    {0xC5, "HM02"},
    {0xC6, "HM03"},
    {0xC7, "HM04"},
    {0xC8, "HM05"},
    {0xC9, "TM01"},
    {0xCA, "TM02"},
    {0xCB, "TM03"},
    {0xCC, "TM04"},
    {0xCD, "TM05"},
    {0xCE, "TM06"},
    {0xCF, "TM07"},
    {0xD0, "TM08"},
    {0xD1, "TM09"},
    {0xD2, "TM10"},
    {0xD3, "TM11"},
    {0xD4, "TM12"},
    {0xD5, "TM13"},
    {0xD6, "TM14"},
    {0xD7, "TM15"},
    {0xD8, "TM16"},
    {0xD9, "TM17"},
    {0xDA, "TM18"},
    {0xDB, "TM19"},
    {0xDC, "TM20"},
    {0xDD, "TM21"},
    {0xDE, "TM22"},
    {0xDF, "TM23"},
    {0xE0, "TM24"},
    {0xE1, "TM25"},
    {0xE2, "TM26"},
    {0xE3, "TM27"},
    {0xE4, "TM28"},
    {0xE5, "TM29"},
    {0xE6, "TM30"},
    {0xE7, "TM31"},
    {0xE8, "TM32"},
    {0xE9, "TM33"},
    {0xEA, "TM34"},
    {0xEB, "TM35"},
    {0xEC, "TM36"},
    {0xED, "TM37"},
    {0xEE, "TM38"},
    {0xEF, "TM39"},
    {0xF0, "TM40"},
    {0xF1, "TM41"},
    {0xF2, "TM42"},
    {0xF3, "TM43"},
    {0xF4, "TM44"},
    {0xF5, "TM45"},
    {0xF6, "TM46"},
    {0xF7, "TM47"},
    {0xF8, "TM48"},
    {0xF9, "TM49"},
    {0xFA, "TM50"},
    {0xFB, "TM51"},
    {0xFC, "TM52"},
    {0xFD, "TM53"},
    {0xFE, "TM54"},
    {0xFF, "TM55"}
};

// ============================================================================
// Generation 2 Items
// ============================================================================

// Full item list for Generation 2
// Items with nameCrystal != nullptr are Crystal-exclusive (Teru-sama in GS)
static const std::unordered_map<uint8_t, Gen2ItemInfo> GEN2_ITEMS = {
    {0x00, {"None", nullptr, POCKET_ITEMS}},
    {0x01, {"Master Ball", nullptr, POCKET_BALLS}},
    {0x02, {"Ultra Ball", nullptr, POCKET_BALLS}},
    {0x03, {"BrightPowder", nullptr, POCKET_ITEMS}},
    {0x04, {"Great Ball", nullptr, POCKET_BALLS}},
    {0x05, {"Poke Ball", nullptr, POCKET_BALLS}},
    {0x06, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x07, {"Bicycle", nullptr, POCKET_KEY_ITEMS}},
    {0x08, {"Moon Stone", nullptr, POCKET_ITEMS}},
    {0x09, {"Antidote", nullptr, POCKET_ITEMS}},
    {0x0A, {"Burn Heal", nullptr, POCKET_ITEMS}},
    {0x0B, {"Ice Heal", nullptr, POCKET_ITEMS}},
    {0x0C, {"Awakening", nullptr, POCKET_ITEMS}},
    {0x0D, {"Parlyz Heal", nullptr, POCKET_ITEMS}},
    {0x0E, {"Full Restore", nullptr, POCKET_ITEMS}},
    {0x0F, {"Max Potion", nullptr, POCKET_ITEMS}},
    {0x10, {"Hyper Potion", nullptr, POCKET_ITEMS}},
    {0x11, {"Super Potion", nullptr, POCKET_ITEMS}},
    {0x12, {"Potion", nullptr, POCKET_ITEMS}},
    {0x13, {"Escape Rope", nullptr, POCKET_ITEMS}},
    {0x14, {"Repel", nullptr, POCKET_ITEMS}},
    {0x15, {"Max Elixer", nullptr, POCKET_ITEMS}},
    {0x16, {"Fire Stone", nullptr, POCKET_ITEMS}},
    {0x17, {"Thunderstone", nullptr, POCKET_ITEMS}},
    {0x18, {"Water Stone", nullptr, POCKET_ITEMS}},
    {0x19, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x1A, {"HP Up", nullptr, POCKET_ITEMS}},
    {0x1B, {"Protein", nullptr, POCKET_ITEMS}},
    {0x1C, {"Iron", nullptr, POCKET_ITEMS}},
    {0x1D, {"Carbos", nullptr, POCKET_ITEMS}},
    {0x1E, {"Lucky Punch", nullptr, POCKET_ITEMS}},
    {0x1F, {"Calcium", nullptr, POCKET_ITEMS}},
    {0x20, {"Rare Candy", nullptr, POCKET_ITEMS}},
    {0x21, {"X Accuracy", nullptr, POCKET_ITEMS}},
    {0x22, {"Leaf Stone", nullptr, POCKET_ITEMS}},
    {0x23, {"Metal Powder", nullptr, POCKET_ITEMS}},
    {0x24, {"Nugget", nullptr, POCKET_ITEMS}},
    {0x25, {"Poke Doll", nullptr, POCKET_ITEMS}},
    {0x26, {"Full Heal", nullptr, POCKET_ITEMS}},
    {0x27, {"Revive", nullptr, POCKET_ITEMS}},
    {0x28, {"Max Revive", nullptr, POCKET_ITEMS}},
    {0x29, {"Guard Spec.", nullptr, POCKET_ITEMS}},
    {0x2A, {"Super Repel", nullptr, POCKET_ITEMS}},
    {0x2B, {"Max Repel", nullptr, POCKET_ITEMS}},
    {0x2C, {"Dire Hit", nullptr, POCKET_ITEMS}},
    {0x2D, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x2E, {"Fresh Water", nullptr, POCKET_ITEMS}},
    {0x2F, {"Soda Pop", nullptr, POCKET_ITEMS}},
    {0x30, {"Lemonade", nullptr, POCKET_ITEMS}},
    {0x31, {"X Attack", nullptr, POCKET_ITEMS}},
    {0x32, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x33, {"X Defend", nullptr, POCKET_ITEMS}},
    {0x34, {"X Speed", nullptr, POCKET_ITEMS}},
    {0x35, {"X Special", nullptr, POCKET_ITEMS}},
    {0x36, {"Coin Case", nullptr, POCKET_KEY_ITEMS}},
    {0x37, {"Itemfinder", nullptr, POCKET_KEY_ITEMS}},
    {0x38, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x39, {"Exp.Share", nullptr, POCKET_ITEMS}},
    {0x3A, {"Old Rod", nullptr, POCKET_KEY_ITEMS}},
    {0x3B, {"Good Rod", nullptr, POCKET_KEY_ITEMS}},
    {0x3C, {"Silver Leaf", nullptr, POCKET_ITEMS}},
    {0x3D, {"Super Rod", nullptr, POCKET_KEY_ITEMS}},
    {0x3E, {"PP Up", nullptr, POCKET_ITEMS}},
    {0x3F, {"Ether", nullptr, POCKET_ITEMS}},
    {0x40, {"Max Ether", nullptr, POCKET_ITEMS}},
    {0x41, {"Elixer", nullptr, POCKET_ITEMS}},
    {0x42, {"Red Scale", nullptr, POCKET_KEY_ITEMS}},
    {0x43, {"SecretPotion", nullptr, POCKET_KEY_ITEMS}},
    {0x44, {"S.S. Ticket", nullptr, POCKET_KEY_ITEMS}},
    {0x45, {"Mystery Egg", nullptr, POCKET_KEY_ITEMS}},
    {0x46, {"Teru-sama", "Clear Bell", POCKET_KEY_ITEMS}},  // Crystal-exclusive
    {0x47, {"Silver Wing", nullptr, POCKET_KEY_ITEMS}},
    {0x48, {"Moomoo Milk", nullptr, POCKET_ITEMS}},
    {0x49, {"Quick Claw", nullptr, POCKET_ITEMS}},
    {0x4A, {"PSNCureBerry", nullptr, POCKET_ITEMS}},
    {0x4B, {"Gold Leaf", nullptr, POCKET_ITEMS}},
    {0x4C, {"Soft Sand", nullptr, POCKET_ITEMS}},
    {0x4D, {"Sharp Beak", nullptr, POCKET_ITEMS}},
    {0x4E, {"PRZCureBerry", nullptr, POCKET_ITEMS}},
    {0x4F, {"Burnt Berry", nullptr, POCKET_ITEMS}},
    {0x50, {"Ice Berry", nullptr, POCKET_ITEMS}},
    {0x51, {"Poison Barb", nullptr, POCKET_ITEMS}},
    {0x52, {"King's Rock", nullptr, POCKET_ITEMS}},
    {0x53, {"Bitter Berry", nullptr, POCKET_ITEMS}},
    {0x54, {"Mint Berry", nullptr, POCKET_ITEMS}},
    {0x55, {"Red Apricorn", nullptr, POCKET_ITEMS}},
    {0x56, {"TinyMushroom", nullptr, POCKET_ITEMS}},
    {0x57, {"Big Mushroom", nullptr, POCKET_ITEMS}},
    {0x58, {"SilverPowder", nullptr, POCKET_ITEMS}},
    {0x59, {"Blu Apricorn", nullptr, POCKET_ITEMS}},
    {0x5A, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x5B, {"Amulet Coin", nullptr, POCKET_ITEMS}},
    {0x5C, {"Ylw Apricorn", nullptr, POCKET_ITEMS}},
    {0x5D, {"Grn Apricorn", nullptr, POCKET_ITEMS}},
    {0x5E, {"Cleanse Tag", nullptr, POCKET_ITEMS}},
    {0x5F, {"Mystic Water", nullptr, POCKET_ITEMS}},
    {0x60, {"TwistedSpoon", nullptr, POCKET_ITEMS}},
    {0x61, {"Wht Apricorn", nullptr, POCKET_ITEMS}},
    {0x62, {"Blackbelt", nullptr, POCKET_ITEMS}},
    {0x63, {"Blk Apricorn", nullptr, POCKET_ITEMS}},
    {0x64, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x65, {"Pnk Apricorn", nullptr, POCKET_ITEMS}},
    {0x66, {"BlackGlasses", nullptr, POCKET_ITEMS}},
    {0x67, {"SlowpokeTail", nullptr, POCKET_ITEMS}},
    {0x68, {"Pink Bow", nullptr, POCKET_ITEMS}},
    {0x69, {"Stick", nullptr, POCKET_ITEMS}},
    {0x6A, {"Smoke Ball", nullptr, POCKET_ITEMS}},
    {0x6B, {"NeverMeltIce", nullptr, POCKET_ITEMS}},
    {0x6C, {"Magnet", nullptr, POCKET_ITEMS}},
    {0x6D, {"MiracleBerry", nullptr, POCKET_ITEMS}},
    {0x6E, {"Pearl", nullptr, POCKET_ITEMS}},
    {0x6F, {"Big Pearl", nullptr, POCKET_ITEMS}},
    {0x70, {"Everstone", nullptr, POCKET_ITEMS}},
    {0x71, {"Spell Tag", nullptr, POCKET_ITEMS}},
    {0x72, {"RageCandyBar", nullptr, POCKET_ITEMS}},
    {0x73, {"Teru-sama", "GS Ball", POCKET_KEY_ITEMS}},     // Crystal-exclusive
    {0x74, {"Teru-sama", "Blue Card", POCKET_KEY_ITEMS}},   // Crystal-exclusive
    {0x75, {"Miracle Seed", nullptr, POCKET_ITEMS}},
    {0x76, {"Thick Club", nullptr, POCKET_ITEMS}},
    {0x77, {"Focus Band", nullptr, POCKET_ITEMS}},
    {0x78, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x79, {"EnergyPowder", nullptr, POCKET_ITEMS}},
    {0x7A, {"Energy Root", nullptr, POCKET_ITEMS}},
    {0x7B, {"Heal Powder", nullptr, POCKET_ITEMS}},
    {0x7C, {"Revival Herb", nullptr, POCKET_ITEMS}},
    {0x7D, {"Hard Stone", nullptr, POCKET_ITEMS}},
    {0x7E, {"Lucky Egg", nullptr, POCKET_ITEMS}},
    {0x7F, {"Card Key", nullptr, POCKET_KEY_ITEMS}},
    {0x80, {"Machine Part", nullptr, POCKET_KEY_ITEMS}},
    {0x81, {"Teru-sama", "Egg Ticket", POCKET_KEY_ITEMS}},  // Crystal-exclusive
    {0x82, {"Lost Item", nullptr, POCKET_KEY_ITEMS}},
    {0x83, {"Stardust", nullptr, POCKET_ITEMS}},
    {0x84, {"Star Piece", nullptr, POCKET_ITEMS}},
    {0x85, {"Basement Key", nullptr, POCKET_KEY_ITEMS}},
    {0x86, {"Pass", nullptr, POCKET_KEY_ITEMS}},
    {0x87, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x88, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x89, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x8A, {"Charcoal", nullptr, POCKET_ITEMS}},
    {0x8B, {"Berry Juice", nullptr, POCKET_ITEMS}},
    {0x8C, {"Scope Lens", nullptr, POCKET_ITEMS}},
    {0x8D, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x8E, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x8F, {"Metal Coat", nullptr, POCKET_ITEMS}},
    {0x90, {"Dragon Fang", nullptr, POCKET_ITEMS}},
    {0x91, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x92, {"Leftovers", nullptr, POCKET_ITEMS}},
    {0x93, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x94, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x95, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x96, {"MysteryBerry", nullptr, POCKET_ITEMS}},
    {0x97, {"Dragon Scale", nullptr, POCKET_ITEMS}},
    {0x98, {"Berserk Gene", nullptr, POCKET_ITEMS}},
    {0x99, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x9A, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x9B, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0x9C, {"Sacred Ash", nullptr, POCKET_ITEMS}},
    {0x9D, {"Heavy Ball", nullptr, POCKET_BALLS}},
    {0x9E, {"Flower Mail", nullptr, POCKET_ITEMS}},
    {0x9F, {"Level Ball", nullptr, POCKET_BALLS}},
    {0xA0, {"Lure Ball", nullptr, POCKET_BALLS}},
    {0xA1, {"Fast Ball", nullptr, POCKET_BALLS}},
    {0xA2, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0xA3, {"Light Ball", nullptr, POCKET_ITEMS}},
    {0xA4, {"Friend Ball", nullptr, POCKET_BALLS}},
    {0xA5, {"Moon Ball", nullptr, POCKET_BALLS}},
    {0xA6, {"Love Ball", nullptr, POCKET_BALLS}},
    {0xA7, {"Normal Box", nullptr, POCKET_ITEMS}},
    {0xA8, {"Gorgeous Box", nullptr, POCKET_ITEMS}},
    {0xA9, {"Sun Stone", nullptr, POCKET_ITEMS}},
    {0xAA, {"Polkadot Bow", nullptr, POCKET_ITEMS}},
    {0xAB, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0xAC, {"Up-Grade", nullptr, POCKET_ITEMS}},
    {0xAD, {"Berry", nullptr, POCKET_ITEMS}},
    {0xAE, {"Gold Berry", nullptr, POCKET_ITEMS}},
    {0xAF, {"SquirtBottle", nullptr, POCKET_KEY_ITEMS}},
    {0xB0, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0xB1, {"Park Ball", nullptr, POCKET_BALLS}},
    {0xB2, {"Rainbow Wing", nullptr, POCKET_KEY_ITEMS}},
    {0xB3, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0xB4, {"Brick Piece", nullptr, POCKET_ITEMS}},
    {0xB5, {"Surf Mail", nullptr, POCKET_ITEMS}},
    {0xB6, {"Litebluemail", nullptr, POCKET_ITEMS}},
    {0xB7, {"Portraitmail", nullptr, POCKET_ITEMS}},
    {0xB8, {"Lovely Mail", nullptr, POCKET_ITEMS}},
    {0xB9, {"Eon Mail", nullptr, POCKET_ITEMS}},
    {0xBA, {"Morph Mail", nullptr, POCKET_ITEMS}},
    {0xBB, {"Bluesky Mail", nullptr, POCKET_ITEMS}},
    {0xBC, {"Music Mail", nullptr, POCKET_ITEMS}},
    {0xBD, {"Mirage Mail", nullptr, POCKET_ITEMS}},
    {0xBE, {"Teru-sama", nullptr, POCKET_ITEMS}},
    {0xBF, {"TM01", nullptr, POCKET_TM_HM}},
    {0xC0, {"TM02", nullptr, POCKET_TM_HM}},
    {0xC1, {"TM03", nullptr, POCKET_TM_HM}},
    {0xC2, {"TM04", nullptr, POCKET_TM_HM}},
    {0xC4, {"TM05", nullptr, POCKET_TM_HM}},
    {0xC5, {"TM06", nullptr, POCKET_TM_HM}},
    {0xC6, {"TM07", nullptr, POCKET_TM_HM}},
    {0xC7, {"TM08", nullptr, POCKET_TM_HM}},
    {0xC8, {"TM09", nullptr, POCKET_TM_HM}},
    {0xC9, {"TM10", nullptr, POCKET_TM_HM}},
    {0xCA, {"TM11", nullptr, POCKET_TM_HM}},
    {0xCB, {"TM12", nullptr, POCKET_TM_HM}},
    {0xCC, {"TM13", nullptr, POCKET_TM_HM}},
    {0xCD, {"TM14", nullptr, POCKET_TM_HM}},
    {0xCE, {"TM15", nullptr, POCKET_TM_HM}},
    {0xCF, {"TM16", nullptr, POCKET_TM_HM}},
    {0xD0, {"TM17", nullptr, POCKET_TM_HM}},
    {0xD1, {"TM18", nullptr, POCKET_TM_HM}},
    {0xD2, {"TM19", nullptr, POCKET_TM_HM}},
    {0xD3, {"TM20", nullptr, POCKET_TM_HM}},
    {0xD4, {"TM21", nullptr, POCKET_TM_HM}},
    {0xD5, {"TM22", nullptr, POCKET_TM_HM}},
    {0xD6, {"TM23", nullptr, POCKET_TM_HM}},
    {0xD7, {"TM24", nullptr, POCKET_TM_HM}},
    {0xD8, {"TM25", nullptr, POCKET_TM_HM}},
    {0xD9, {"TM26", nullptr, POCKET_TM_HM}},
    {0xDA, {"TM27", nullptr, POCKET_TM_HM}},
    {0xDB, {"TM28", nullptr, POCKET_TM_HM}},
    {0xDD, {"TM29", nullptr, POCKET_TM_HM}},
    {0xDE, {"TM30", nullptr, POCKET_TM_HM}},
    {0xDF, {"TM31", nullptr, POCKET_TM_HM}},
    {0xE0, {"TM32", nullptr, POCKET_TM_HM}},
    {0xE1, {"TM33", nullptr, POCKET_TM_HM}},
    {0xE2, {"TM34", nullptr, POCKET_TM_HM}},
    {0xE3, {"TM35", nullptr, POCKET_TM_HM}},
    {0xE4, {"TM36", nullptr, POCKET_TM_HM}},
    {0xE5, {"TM37", nullptr, POCKET_TM_HM}},
    {0xE6, {"TM38", nullptr, POCKET_TM_HM}},
    {0xE7, {"TM39", nullptr, POCKET_TM_HM}},
    {0xE8, {"TM40", nullptr, POCKET_TM_HM}},
    {0xE9, {"TM41", nullptr, POCKET_TM_HM}},
    {0xEA, {"TM42", nullptr, POCKET_TM_HM}},
    {0xEB, {"TM43", nullptr, POCKET_TM_HM}},
    {0xEC, {"TM44", nullptr, POCKET_TM_HM}},
    {0xED, {"TM45", nullptr, POCKET_TM_HM}},
    {0xEE, {"TM46", nullptr, POCKET_TM_HM}},
    {0xEF, {"TM47", nullptr, POCKET_TM_HM}},
    {0xF0, {"TM48", nullptr, POCKET_TM_HM}},
    {0xF1, {"TM49", nullptr, POCKET_TM_HM}},
    {0xF2, {"TM50", nullptr, POCKET_TM_HM}},
    {0xF3, {"HM01", nullptr, POCKET_TM_HM}},
    {0xF4, {"HM02", nullptr, POCKET_TM_HM}},
    {0xF5, {"HM03", nullptr, POCKET_TM_HM}},
    {0xF6, {"HM04", nullptr, POCKET_TM_HM}},
    {0xF7, {"HM05", nullptr, POCKET_TM_HM}},
    {0xF8, {"HM06", nullptr, POCKET_TM_HM}},
    {0xF9, {"HM07", nullptr, POCKET_TM_HM}},
    {0xFA, {"HM08", nullptr, POCKET_ITEMS}},       // Note: Invalid HM in Items pocket (glitch)
    {0xFB, {"HM09", nullptr, POCKET_ITEMS}},       // Note: Invalid HM in Items pocket (glitch)
    {0xFC, {"HM10", nullptr, POCKET_ITEMS}},       // Note: Invalid HM in Items pocket (glitch)
    {0xFD, {"HM11", nullptr, POCKET_ITEMS}},       // Note: Invalid HM in Items pocket (glitch)
    {0xFE, {"HM12", nullptr, POCKET_ITEMS}},       // Note: Invalid HM in Items pocket (glitch)
    {0xFF, {"Cancel", nullptr, POCKET_UNKNOWN}}
};

// ============================================================================
// Generation 3 Items
// ============================================================================

// Note: Gen 3 uses 16-bit item indices
// Items marked with gameFlags indicate which games they appear in
// Unknown/unused item slots are not included in this map
static const std::unordered_map<uint16_t, Gen3ItemInfo> GEN3_ITEMS = {
    // Nothing
    {0x0000, {"Nothing", GEN3_POCKET_UNKNOWN, GEN3_GAME_ALL}},
    
    // Poké Balls (0x0001 - 0x000C)
    {0x0001, {"Master Ball", GEN3_POCKET_BALLS, GEN3_GAME_ALL}},
    {0x0002, {"Ultra Ball", GEN3_POCKET_BALLS, GEN3_GAME_ALL}},
    {0x0003, {"Great Ball", GEN3_POCKET_BALLS, GEN3_GAME_ALL}},
    {0x0004, {"Poke Ball", GEN3_POCKET_BALLS, GEN3_GAME_ALL}},
    {0x0005, {"Safari Ball", GEN3_POCKET_BALLS, GEN3_GAME_ALL}},
    {0x0006, {"Net Ball", GEN3_POCKET_BALLS, GEN3_GAME_ALL}},
    {0x0007, {"Dive Ball", GEN3_POCKET_BALLS, GEN3_GAME_ALL}},
    {0x0008, {"Nest Ball", GEN3_POCKET_BALLS, GEN3_GAME_ALL}},
    {0x0009, {"Repeat Ball", GEN3_POCKET_BALLS, GEN3_GAME_ALL}},
    {0x000A, {"Timer Ball", GEN3_POCKET_BALLS, GEN3_GAME_ALL}},
    {0x000B, {"Luxury Ball", GEN3_POCKET_BALLS, GEN3_GAME_ALL}},
    {0x000C, {"Premier Ball", GEN3_POCKET_BALLS, GEN3_GAME_ALL}},
    
    // Regular Items (0x000D - 0x0033)
    {0x000D, {"Potion", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x000E, {"Antidote", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x000F, {"Burn Heal", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0010, {"Ice Heal", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0011, {"Awakening", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0012, {"Parlyz Heal", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0013, {"Full Restore", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0014, {"Max Potion", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0015, {"Hyper Potion", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0016, {"Super Potion", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0017, {"Full Heal", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0018, {"Revive", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0019, {"Max Revive", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x001A, {"Fresh Water", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x001B, {"Soda Pop", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x001C, {"Lemonade", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x001D, {"Moomoo Milk", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x001E, {"EnergyPowder", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x001F, {"Energy Root", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0020, {"Heal Powder", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0021, {"Revival Herb", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0022, {"Ether", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0023, {"Max Ether", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0024, {"Elixir", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0025, {"Max Elixir", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0026, {"Lava Cookie", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0027, {"Blue Flute", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0028, {"Yellow Flute", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0029, {"Red Flute", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x002A, {"Black Flute", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x002B, {"White Flute", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x002C, {"Berry Juice", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x002D, {"Sacred Ash", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x002E, {"Shoal Salt", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x002F, {"Shoal Shell", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0030, {"Red Shard", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0031, {"Blue Shard", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0032, {"Yellow Shard", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0033, {"Green Shard", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    
    // Regular Items (0x003F - 0x0051)
    {0x003F, {"HP Up", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0040, {"Protein", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0041, {"Iron", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0042, {"Carbos", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0043, {"Calcium", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0044, {"Rare Candy", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0045, {"PP Up", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0046, {"Zinc", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0047, {"PP Max", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0049, {"Guard Spec.", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x004A, {"Dire Hit", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x004B, {"X Attack", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x004C, {"X Defend", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x004D, {"X Speed", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x004E, {"X Accuracy", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x004F, {"X Special", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0050, {"Poke Doll", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0051, {"Fluffy Tail", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    
    // Regular Items (0x0053 - 0x0056)
    {0x0053, {"Super Repel", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0054, {"Max Repel", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0055, {"Escape Rope", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0056, {"Repel", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    
    // Evolution Stones (0x005D - 0x0062)
    {0x005D, {"Sun Stone", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x005E, {"Moon Stone", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x005F, {"Fire Stone", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0060, {"Thunderstone", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0061, {"Water Stone", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0062, {"Leaf Stone", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    
    // Sellable Items (0x0067 - 0x006F)
    {0x0067, {"TinyMushroom", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0068, {"Big Mushroom", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x006A, {"Pearl", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x006B, {"Big Pearl", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x006C, {"Stardust", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x006D, {"Star Piece", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x006E, {"Nugget", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x006F, {"Heart Scale", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    
    // Mail Items (0x0079 - 0x0084)
    {0x0079, {"Orange Mail", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x007A, {"Harbor Mail", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x007B, {"Glitter Mail", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x007C, {"Mech Mail", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x007D, {"Wood Mail", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x007E, {"Wave Mail", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x007F, {"Bead Mail", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0080, {"Shadow Mail", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0081, {"Tropic Mail", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0082, {"Dream Mail", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0083, {"Fab Mail", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0084, {"Retro Mail", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    
    // Berries (0x0085 - 0x00AF)
    {0x0085, {"Cheri Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0086, {"Chesto Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0087, {"Pecha Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0088, {"Rawst Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0089, {"Aspear Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x008A, {"Leppa Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x008B, {"Oran Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x008C, {"Persim Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x008D, {"Lum Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x008E, {"Sitrus Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x008F, {"Figy Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0090, {"Wiki Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0091, {"Mago Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0092, {"Aguav Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0093, {"Iapapa Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0094, {"Razz Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0095, {"Bluk Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0096, {"Nanab Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0097, {"Wepear Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0098, {"Pinap Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x0099, {"Pomeg Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x009A, {"Kelpsy Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x009B, {"Qualot Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x009C, {"Hondew Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x009D, {"Grepa Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x009E, {"Tamato Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x009F, {"Cornn Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00A0, {"Magost Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00A1, {"Rabuta Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00A2, {"Nomel Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00A3, {"Spelon Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00A4, {"Pamtre Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00A5, {"Watmel Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00A6, {"Durin Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00A7, {"Belue Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00A8, {"Liechi Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00A9, {"Ganlon Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00AA, {"Salac Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00AB, {"Petaya Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00AC, {"Apicot Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00AD, {"Lansat Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00AE, {"Starf Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    {0x00AF, {"Enigma Berry", GEN3_POCKET_BERRIES, GEN3_GAME_ALL}},
    
    // Hold Items (0x00B3 - 0x00E1)
    {0x00B3, {"BrightPowder", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00B4, {"White Herb", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00B5, {"Macho Brace", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00B6, {"Exp. Share", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00B7, {"Quick Claw", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00B8, {"Soothe Bell", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00B9, {"Mental Herb", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00BA, {"Choice Band", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00BB, {"King's Rock", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00BC, {"SilverPowder", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00BD, {"Amulet Coin", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00BE, {"Cleanse Tag", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00BF, {"Soul Dew", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00C0, {"DeepSeaTooth", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00C1, {"DeepSeaScale", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00C2, {"Smoke Ball", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00C3, {"Everstone", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00C4, {"Focus Band", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00C5, {"Lucky Egg", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00C6, {"Scope Lens", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00C7, {"Metal Coat", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00C8, {"Leftovers", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00C9, {"Dragon Scale", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00CA, {"Light Ball", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00CB, {"Soft Sand", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00CC, {"Hard Stone", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00CD, {"Miracle Seed", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00CE, {"BlackGlasses", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00CF, {"Black Belt", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00D0, {"Magnet", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00D1, {"Mystic Water", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00D2, {"Sharp Beak", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00D3, {"Poison Barb", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00D4, {"NeverMeltIce", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00D5, {"Spell Tag", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00D6, {"TwistedSpoon", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00D7, {"Charcoal", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00D8, {"Dragon Fang", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00D9, {"Silk Scarf", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00DA, {"Up-Grade", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00DB, {"Shell Bell", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00DC, {"Sea Incense", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00DD, {"Lax Incense", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00DE, {"Lucky Punch", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00DF, {"Metal Powder", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00E0, {"Thick Club", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00E1, {"Stick", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    
    // Contest Scarves (0x00FE - 0x0102)
    {0x00FE, {"Red Scarf", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x00FF, {"Blue Scarf", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0100, {"Pink Scarf", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0101, {"Green Scarf", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    {0x0102, {"Yellow Scarf", GEN3_POCKET_ITEMS, GEN3_GAME_ALL}},
    
    // Key Items - RSE (0x0103 - 0x0120)
    {0x0103, {"Mach Bike", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0104, {"Coin Case", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0105, {"Itemfinder", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0106, {"Old Rod", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0107, {"Good Rod", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0108, {"Super Rod", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0109, {"S.S. Ticket", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x010A, {"Contest Pass", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x010C, {"Wailmer Pail", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x010D, {"Devon Goods", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x010E, {"Soot Sack", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x010F, {"Basement Key", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0110, {"Acro Bike", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0111, {"Pokeblock Case", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0112, {"Letter", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0113, {"Eon Ticket", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0114, {"Red Orb", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0115, {"Blue Orb", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0116, {"Scanner", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0117, {"Go-Goggles", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0118, {"Meteorite", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0119, {"Rm. 1 Key", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x011A, {"Rm. 2 Key", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x011B, {"Rm. 4 Key", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x011C, {"Rm. 6 Key", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x011D, {"Storage Key", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x011E, {"Root Fossil", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x011F, {"Claw Fossil", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    {0x0120, {"Devon Scope", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_ALL}},
    
    // TMs (0x0121 - 0x0152)
    {0x0121, {"TM01", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0122, {"TM02", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0123, {"TM03", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0124, {"TM04", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0125, {"TM05", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0126, {"TM06", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0127, {"TM07", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0128, {"TM08", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0129, {"TM09", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x012A, {"TM10", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x012B, {"TM11", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x012C, {"TM12", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x012D, {"TM13", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x012E, {"TM14", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x012F, {"TM15", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0130, {"TM16", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0131, {"TM17", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0132, {"TM18", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0133, {"TM19", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0134, {"TM20", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0135, {"TM21", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0136, {"TM22", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0137, {"TM23", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0138, {"TM24", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0139, {"TM25", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x013A, {"TM26", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x013B, {"TM27", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x013C, {"TM28", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x013D, {"TM29", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x013E, {"TM30", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x013F, {"TM31", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0140, {"TM32", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0141, {"TM33", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0142, {"TM34", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0143, {"TM35", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0144, {"TM36", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0145, {"TM37", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0146, {"TM38", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0147, {"TM39", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0148, {"TM40", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0149, {"TM41", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x014A, {"TM42", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x014B, {"TM43", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x014C, {"TM44", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x014D, {"TM45", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x014E, {"TM46", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x014F, {"TM47", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0150, {"TM48", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0151, {"TM49", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0152, {"TM50", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    
    // HMs (0x0153 - 0x015A)
    {0x0153, {"HM01", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0154, {"HM02", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0155, {"HM03", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0156, {"HM04", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0157, {"HM05", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0158, {"HM06", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x0159, {"HM07", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    {0x015A, {"HM08", GEN3_POCKET_TM_HM, GEN3_GAME_ALL}},
    
    // Key Items - FRLG/Emerald only (0x015D - 0x0174)
    {0x015D, {"Oak's Parcel", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x015E, {"Poke Flute", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x015F, {"Secret Key", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0160, {"Bike Voucher", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0161, {"Gold Teeth", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0162, {"Old Amber", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0163, {"Card Key", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0164, {"Lift Key", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0165, {"Helix Fossil", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0166, {"Dome Fossil", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0167, {"Silph Scope", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0168, {"Bicycle", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0169, {"Town Map", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x016A, {"VS Seeker", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x016B, {"Fame Checker", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x016C, {"TM Case", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x016D, {"Berry Pouch", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x016E, {"Teachy TV", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x016F, {"Tri-Pass", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0170, {"Rainbow Pass", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0171, {"Tea", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0172, {"MysticTicket", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0173, {"AuroraTicket", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0174, {"Powder Jar", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    
    // Key Items - FRLG only (0x0175 - 0x0176)
    {0x0175, {"Ruby", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    {0x0176, {"Sapphire", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E | GEN3_GAME_FRLG}},
    
    // Key Items - Emerald only (0x0177 - 0x0178)
    {0x0177, {"Magma Emblem", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E}},
    {0x0178, {"Old Sea Map", GEN3_POCKET_KEY_ITEMS, GEN3_GAME_E}}
};

// ============================================================================
// Generation 1 and 2 Utility Functions
// ============================================================================

// Get Gen 1 item name by index (returns nullptr if not found)
inline const char* getGen1ItemName(uint8_t index) {
    auto it = GEN1_ITEMS.find(index);
    if (it != GEN1_ITEMS.end()) {
        return it->second;
    }
    return nullptr;
}

// Get Gen 2 item info by index (returns nullptr if not found)
inline const Gen2ItemInfo* getGen2ItemInfo(uint8_t index) {
    auto it = GEN2_ITEMS.find(index);
    if (it != GEN2_ITEMS.end()) {
        return &it->second;
    }
    return nullptr;
}

// Get Gen 2 item name by index for specific game version
// isCrystal: true for Crystal, false for Gold/Silver
inline const char* getGen2ItemName(uint8_t index, bool isCrystal) {
    const Gen2ItemInfo* info = getGen2ItemInfo(index);
    if (!info) return nullptr;
    
    if (isCrystal && info->nameCrystal != nullptr) {
        return info->nameCrystal;
    }
    return info->nameGS;
}

// Get pocket name for Gen 2
inline const char* getGen2PocketName(uint8_t pocket) {
    switch (pocket) {
        case POCKET_ITEMS:     return "Items";
        case POCKET_BALLS:     return "Poke Balls";
        case POCKET_KEY_ITEMS: return "Key Items";
        case POCKET_TM_HM:     return "TM/HM";
        default:               return "Unknown";
    }
}

// Get Gen 2 item pocket by index (returns POCKET_UNKNOWN if not found)
inline uint8_t getGen2ItemPocket(uint8_t index) {
    const Gen2ItemInfo* info = getGen2ItemInfo(index);
    if (!info) return POCKET_UNKNOWN;
    return info->pocket;
}

// Check if Gen 2 item exists in given index
inline bool gen2ItemExists(uint8_t index) {
    return GEN2_ITEMS.find(index) != GEN2_ITEMS.end();
}

// Check if Gen 1 item exists in given index
inline bool gen1ItemExists(uint8_t index) {
    return GEN1_ITEMS.find(index) != GEN1_ITEMS.end();
}

// ============================================================================
// Generation 3 Utility Functions
// ============================================================================

// Get Gen 3 item info by index (returns nullptr if not found)
inline const Gen3ItemInfo* getGen3ItemInfo(uint16_t index) {
    auto it = GEN3_ITEMS.find(index);
    if (it != GEN3_ITEMS.end()) {
        return &it->second;
    }
    return nullptr;
}

// Get Gen 3 item name by index (returns nullptr if not found)
inline const char* getGen3ItemName(uint16_t index) {
    const Gen3ItemInfo* info = getGen3ItemInfo(index);
    if (!info) return nullptr;
    return info->name;
}

// Get Gen 3 item pocket by index (returns GEN3_POCKET_UNKNOWN if not found)
inline uint8_t getGen3ItemPocket(uint16_t index) {
    const Gen3ItemInfo* info = getGen3ItemInfo(index);
    if (!info) return GEN3_POCKET_UNKNOWN;
    return info->pocket;
}

// Get Gen 3 item game flags by index (returns GEN3_GAME_NONE if not found)
inline uint8_t getGen3ItemGameFlags(uint16_t index) {
    const Gen3ItemInfo* info = getGen3ItemInfo(index);
    if (!info) return GEN3_GAME_NONE;
    return info->gameFlags;
}

// Check if Gen 3 item exists in given index
inline bool gen3ItemExists(uint16_t index) {
    return GEN3_ITEMS.find(index) != GEN3_ITEMS.end();
}

// Check if Gen 3 item is available in a specific game
inline bool gen3ItemAvailableIn(uint16_t index, uint8_t gameFlag) {
    const Gen3ItemInfo* info = getGen3ItemInfo(index);
    if (!info) return false;
    return (info->gameFlags & gameFlag) != 0;
}

// Check if item is available in Ruby/Sapphire
inline bool gen3ItemInRubySapphire(uint16_t index) {
    return gen3ItemAvailableIn(index, GEN3_GAME_RS);
}

// Check if item is available in Emerald
inline bool gen3ItemInEmerald(uint16_t index) {
    return gen3ItemAvailableIn(index, GEN3_GAME_E);
}

// Check if item is available in FireRed/LeafGreen
inline bool gen3ItemInFireRedLeafGreen(uint16_t index) {
    return gen3ItemAvailableIn(index, GEN3_GAME_FRLG);
}

// Get pocket name for Gen 3
inline const char* getGen3PocketName(uint8_t pocket) {
    switch (pocket) {
        case GEN3_POCKET_ITEMS:     return "Items";
        case GEN3_POCKET_BALLS:     return "Poke Balls";
        case GEN3_POCKET_KEY_ITEMS: return "Key Items";
        case GEN3_POCKET_TM_HM:     return "TM/HM";
        case GEN3_POCKET_BERRIES:   return "Berries";
        default:                    return "Unknown";
    }
}

// Check if item is a TM (returns TM number 1-50, or 0 if not a TM)
inline uint8_t getGen3TMNumber(uint16_t index) {
    if (index >= 0x0121 && index <= 0x0152) {
        return static_cast<uint8_t>(index - 0x0120);
    }
    return 0;
}

// Check if item is an HM (returns HM number 1-8, or 0 if not an HM)
inline uint8_t getGen3HMNumber(uint16_t index) {
    if (index >= 0x0153 && index <= 0x015A) {
        return static_cast<uint8_t>(index - 0x0152);
    }
    return 0;
}

// Check if item is a Berry (returns berry number 1-43, or 0 if not a berry)
inline uint8_t getGen3BerryNumber(uint16_t index) {
    if (index >= 0x0085 && index <= 0x00AF) {
        return static_cast<uint8_t>(index - 0x0084);
    }
    return 0;
}

// Check if item is a Poké Ball
inline bool isGen3PokeBall(uint16_t index) {
    return (index >= 0x0001 && index <= 0x000C);
}

// Check if item is a Berry
inline bool isGen3Berry(uint16_t index) {
    return (index >= 0x0085 && index <= 0x00AF);
}

// Check if item is a TM or HM
inline bool isGen3TMHM(uint16_t index) {
    return (index >= 0x0121 && index <= 0x015A);
}

// Check if item is a Key Item
inline bool isGen3KeyItem(uint16_t index) {
    const Gen3ItemInfo* info = getGen3ItemInfo(index);
    if (!info) return false;
    return info->pocket == GEN3_POCKET_KEY_ITEMS;
}

// Check if item is a Mail item
inline bool isGen3Mail(uint16_t index) {
    return (index >= 0x0079 && index <= 0x0084);
}

// Get TM/HM index from TM number (1-50) - returns item index or 0 if invalid
inline uint16_t getGen3TMIndex(uint8_t tmNumber) {
    if (tmNumber >= 1 && tmNumber <= 50) {
        return 0x0120 + tmNumber;
    }
    return 0;
}

// Get HM index from HM number (1-8) - returns item index or 0 if invalid
inline uint16_t getGen3HMIndex(uint8_t hmNumber) {
    if (hmNumber >= 1 && hmNumber <= 8) {
        return 0x0152 + hmNumber;
    }
    return 0;
}

// Get Berry index from berry number (1-43) - returns item index or 0 if invalid
inline uint16_t getGen3BerryIndex(uint8_t berryNumber) {
    if (berryNumber >= 1 && berryNumber <= 43) {
        return 0x0084 + berryNumber;
    }
    return 0;
}

} // namespace ItemsIndex

#endif // ITEMS_INDEX_ENG_H