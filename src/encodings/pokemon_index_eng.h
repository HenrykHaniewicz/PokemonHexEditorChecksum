#ifndef POKEMON_INDEX_ENG_H
#define POKEMON_INDEX_ENG_H

#include <cstdint>
#include <unordered_map>

namespace PokemonIndex {

// ============================================================================
// Type Codes (Gen 1 & Gen 2)
// ============================================================================

enum Gen1Type : uint8_t {
    TYPE_NORMAL   = 0x00,
    TYPE_FIGHTING = 0x01,
    TYPE_FLYING   = 0x02,
    TYPE_POISON   = 0x03,
    TYPE_GROUND   = 0x04,
    TYPE_ROCK     = 0x05,
    TYPE_BIRD     = 0x06, // Unused type seen on MissingNo.
    TYPE_BUG      = 0x07,
    TYPE_GHOST    = 0x08,
    TYPE_STEEL    = 0x09, // Gen 2
    TYPE_FIRE     = 0x14,
    TYPE_WATER    = 0x15,
    TYPE_GRASS    = 0x16,
    TYPE_ELECTRIC = 0x17,
    TYPE_PSYCHIC  = 0x18,
    TYPE_ICE      = 0x19,
    TYPE_DRAGON   = 0x1A,
    TYPE_DARK     = 0x1B, // Gen 2
    TYPE_CURSE    = 0x13, // ??? type (used by moves like Curse)
    TYPE_UNKNOWN  = 0xFF
};

// ============================================================================
// Pokémon Data Structure (Gen 1 & Gen 2)
// ============================================================================

struct PokemonInfo {
    const char* name;
    uint8_t type1;
    uint8_t type2;
};

// ============================================================================
// Generation 1 Pokémon Table
// ============================================================================

static const std::unordered_map<uint8_t, PokemonInfo> GEN1_POKEMON = {
    {0x01, {"Rhydon", TYPE_GROUND, TYPE_ROCK}},
    {0x02, {"Kangaskhan", TYPE_NORMAL, TYPE_NORMAL}},
    {0x03, {"Nidoran♂", TYPE_POISON, TYPE_POISON}},
    {0x04, {"Clefairy", TYPE_NORMAL, TYPE_NORMAL}},
    {0x05, {"Spearow", TYPE_NORMAL, TYPE_FLYING}},
    {0x06, {"Voltorb", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0x07, {"Nidoking", TYPE_POISON, TYPE_GROUND}},
    {0x08, {"Slowbro", TYPE_WATER, TYPE_PSYCHIC}},
    {0x09, {"Ivysaur", TYPE_GRASS, TYPE_POISON}},
    {0x0A, {"Exeggutor", TYPE_GRASS, TYPE_PSYCHIC}},
    {0x0B, {"Lickitung", TYPE_NORMAL, TYPE_NORMAL}},
    {0x0C, {"Exeggcute", TYPE_GRASS, TYPE_PSYCHIC}},
    {0x0D, {"Grimer", TYPE_POISON, TYPE_POISON}},
    {0x0E, {"Gengar", TYPE_GHOST, TYPE_POISON}},
    {0x0F, {"Nidoran♀", TYPE_POISON, TYPE_POISON}},
    {0x10, {"Nidoqueen", TYPE_POISON, TYPE_GROUND}},
    {0x11, {"Cubone", TYPE_GROUND, TYPE_GROUND}},
    {0x12, {"Rhyhorn", TYPE_GROUND, TYPE_ROCK}},
    {0x13, {"Lapras", TYPE_WATER, TYPE_ICE}},
    {0x14, {"Arcanine", TYPE_FIRE, TYPE_FIRE}},
    {0x15, {"Mew", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x16, {"Gyarados", TYPE_WATER, TYPE_FLYING}},
    {0x17, {"Shellder", TYPE_WATER, TYPE_WATER}},
    {0x18, {"Tentacool", TYPE_WATER, TYPE_POISON}},
    {0x19, {"Gastly", TYPE_GHOST, TYPE_POISON}},
    {0x1A, {"Scyther", TYPE_BUG, TYPE_FLYING}},
    {0x1B, {"Staryu", TYPE_WATER, TYPE_WATER}},
    {0x1C, {"Blastoise", TYPE_WATER, TYPE_WATER}},
    {0x1D, {"Pinsir", TYPE_BUG, TYPE_BUG}},
    {0x1E, {"Tangela", TYPE_GRASS, TYPE_GRASS}},
    {0x1F, {"MissingNo. (Scizor)", TYPE_BIRD, TYPE_NORMAL}},
    {0x20, {"MissingNo. (Shuckle)", TYPE_BIRD, TYPE_NORMAL}},
    {0x21, {"Growlithe", TYPE_FIRE, TYPE_FIRE}},
    {0x22, {"Onix", TYPE_ROCK, TYPE_GROUND}},
    {0x23, {"Fearow", TYPE_NORMAL, TYPE_FLYING}},
    {0x24, {"Pidgey", TYPE_NORMAL, TYPE_FLYING}},
    {0x25, {"Slowpoke", TYPE_WATER, TYPE_PSYCHIC}},
    {0x26, {"Kadabra", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x27, {"Graveler", TYPE_ROCK, TYPE_GROUND}},
    {0x28, {"Chansey", TYPE_NORMAL, TYPE_NORMAL}},
    {0x29, {"Machoke", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x2A, {"Mr. Mime", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x2B, {"Hitmonlee", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x2C, {"Hitmonchan", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x2D, {"Arbok", TYPE_POISON, TYPE_POISON}},
    {0x2E, {"Parasect", TYPE_BUG, TYPE_GRASS}},
    {0x2F, {"Psyduck", TYPE_WATER, TYPE_WATER}},
    {0x30, {"Drowzee", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x31, {"Golem", TYPE_ROCK, TYPE_GROUND}},
    {0x32, {"MissingNo. (Heracross)", TYPE_BIRD, TYPE_NORMAL}},
    {0x33, {"Magmar", TYPE_FIRE, TYPE_FIRE}},
    {0x34, {"MissingNo. (Ho-Oh)", TYPE_BIRD, TYPE_NORMAL}},
    {0x35, {"Electabuzz", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0x36, {"Magneton", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0x37, {"Koffing", TYPE_POISON, TYPE_POISON}},
    {0x38, {"MissingNo. (Sneasel)", TYPE_BIRD, TYPE_NORMAL}},
    {0x39, {"Mankey", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x3A, {"Seel", TYPE_WATER, TYPE_WATER}},
    {0x3B, {"Diglett", TYPE_GROUND, TYPE_GROUND}},
    {0x3C, {"Tauros", TYPE_NORMAL, TYPE_NORMAL}},
    {0x3D, {"MissingNo. (Teddiursa)", TYPE_BIRD, TYPE_NORMAL}},
    {0x3E, {"MissingNo. (Ursaring)", TYPE_BIRD, TYPE_NORMAL}},
    {0x3F, {"MissingNo. (Slugma)", TYPE_BIRD, TYPE_NORMAL}},
    {0x40, {"Farfetch'd", TYPE_NORMAL, TYPE_FLYING}},
    {0x41, {"Venonat", TYPE_BUG, TYPE_POISON}},
    {0x42, {"Dragonite", TYPE_DRAGON, TYPE_FLYING}},
    {0x43, {"MissingNo. (Magcargo)", TYPE_BIRD, TYPE_NORMAL}},
    {0x44, {"MissingNo. (Swinub)", TYPE_BIRD, TYPE_NORMAL}},
    {0x45, {"MissingNo. (Piloswine)", TYPE_BIRD, TYPE_NORMAL}},
    {0x46, {"Doduo", TYPE_NORMAL, TYPE_FLYING}},
    {0x47, {"Poliwag", TYPE_WATER, TYPE_WATER}},
    {0x48, {"Jynx", TYPE_ICE, TYPE_PSYCHIC}},
    {0x49, {"Moltres", TYPE_FIRE, TYPE_FLYING}},
    {0x4A, {"Articuno", TYPE_ICE, TYPE_FLYING}},
    {0x4B, {"Zapdos", TYPE_ELECTRIC, TYPE_FLYING}},
    {0x4C, {"Ditto", TYPE_NORMAL, TYPE_NORMAL}},
    {0x4D, {"Meowth", TYPE_NORMAL, TYPE_NORMAL}},
    {0x4E, {"Krabby", TYPE_WATER, TYPE_WATER}},
    {0x4F, {"MissingNo. (Corsola)", TYPE_BIRD, TYPE_NORMAL}},
    {0x50, {"MissingNo. (Remoraid)", TYPE_BIRD, TYPE_NORMAL}},
    {0x51, {"MissingNo. (Octillery)", TYPE_BIRD, TYPE_NORMAL}},
    {0x52, {"Vulpix", TYPE_FIRE, TYPE_FIRE}},
    {0x53, {"Ninetales", TYPE_FIRE, TYPE_FIRE}},
    {0x54, {"Pikachu", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0x55, {"Raichu", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0x56, {"MissingNo. (Delibird)", TYPE_BIRD, TYPE_NORMAL}},
    {0x57, {"MissingNo. (Mantine)", TYPE_BIRD, TYPE_NORMAL}},
    {0x58, {"Dratini", TYPE_DRAGON, TYPE_DRAGON}},
    {0x59, {"Dragonair", TYPE_DRAGON, TYPE_DRAGON}},
    {0x5A, {"Kabuto", TYPE_ROCK, TYPE_WATER}},
    {0x5B, {"Kabutops", TYPE_ROCK, TYPE_WATER}},
    {0x5C, {"Horsea", TYPE_WATER, TYPE_WATER}},
    {0x5D, {"Seadra", TYPE_WATER, TYPE_WATER}},
    {0x5E, {"MissingNo. (Skarmory)", TYPE_BIRD, TYPE_NORMAL}},
    {0x5F, {"MissingNo. (Houndour)", TYPE_BIRD, TYPE_NORMAL}},
    {0x60, {"Sandshrew", TYPE_GROUND, TYPE_GROUND}},
    {0x61, {"Sandslash", TYPE_GROUND, TYPE_GROUND}},
    {0x62, {"Omanyte", TYPE_ROCK, TYPE_WATER}},
    {0x63, {"Omastar", TYPE_ROCK, TYPE_WATER}},
    {0x64, {"Jigglypuff", TYPE_NORMAL, TYPE_NORMAL}},
    {0x65, {"Wigglytuff", TYPE_NORMAL, TYPE_NORMAL}},
    {0x66, {"Eevee", TYPE_NORMAL, TYPE_NORMAL}},
    {0x67, {"Flareon", TYPE_FIRE, TYPE_FIRE}},
    {0x68, {"Jolteon", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0x69, {"Vaporeon", TYPE_WATER, TYPE_WATER}},
    {0x6A, {"Machop", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x6B, {"Zubat", TYPE_POISON, TYPE_FLYING}},
    {0x6C, {"Ekans", TYPE_POISON, TYPE_POISON}},
    {0x6D, {"Paras", TYPE_BUG, TYPE_GRASS}},
    {0x6E, {"Poliwhirl", TYPE_WATER, TYPE_WATER}},
    {0x6F, {"Poliwrath", TYPE_WATER, TYPE_FIGHTING}},
    {0x70, {"Weedle", TYPE_BUG, TYPE_POISON}},
    {0x71, {"Kakuna", TYPE_BUG, TYPE_POISON}},
    {0x72, {"Beedrill", TYPE_BUG, TYPE_POISON}},
    {0x73, {"MissingNo. (Houndoom)", TYPE_BIRD, TYPE_NORMAL}},
    {0x74, {"Dodrio", TYPE_NORMAL, TYPE_FLYING}},
    {0x75, {"Primeape", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x76, {"Dugtrio", TYPE_GROUND, TYPE_GROUND}},
    {0x77, {"Venomoth", TYPE_BUG, TYPE_POISON}},
    {0x78, {"Dewgong", TYPE_WATER, TYPE_ICE}},
    {0x79, {"MissingNo. (Kingdra)", TYPE_BIRD, TYPE_NORMAL}},
    {0x7A, {"MissingNo. (Phanpy)", TYPE_BIRD, TYPE_NORMAL}},
    {0x7B, {"Caterpie", TYPE_BUG, TYPE_BUG}},
    {0x7C, {"Metapod", TYPE_BUG, TYPE_BUG}},
    {0x7D, {"Butterfree", TYPE_BUG, TYPE_FLYING}},
    {0x7E, {"Machamp", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x7F, {"MissingNo. (Donphan)", TYPE_BIRD, TYPE_NORMAL}},
    {0x80, {"Golduck", TYPE_WATER, TYPE_WATER}},
    {0x81, {"Hypno", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x82, {"Golbat", TYPE_POISON, TYPE_FLYING}},
    {0x83, {"Mewtwo", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x84, {"Snorlax", TYPE_NORMAL, TYPE_NORMAL}},
    {0x85, {"Magikarp", TYPE_WATER, TYPE_WATER}},
    {0x86, {"MissingNo. (Porygon2)", TYPE_BIRD, TYPE_NORMAL}},
    {0x87, {"MissingNo. (Stantler)", TYPE_BIRD, TYPE_NORMAL}},
    {0x88, {"Muk", TYPE_POISON, TYPE_POISON}},
    {0x89, {"MissingNo. (Smeargle)", TYPE_BIRD, TYPE_NORMAL}},
    {0x8A, {"Kingler", TYPE_WATER, TYPE_WATER}},
    {0x8B, {"Cloyster", TYPE_WATER, TYPE_ICE}},
    {0x8C, {"MissingNo. (Tyrogue)", TYPE_BIRD, TYPE_NORMAL}},
    {0x8D, {"Electrode", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0x8E, {"Clefable", TYPE_NORMAL, TYPE_NORMAL}},
    {0x8F, {"Weezing", TYPE_POISON, TYPE_POISON}},
    {0x90, {"Persian", TYPE_NORMAL, TYPE_NORMAL}},
    {0x91, {"Marowak", TYPE_GROUND, TYPE_GROUND}},
    {0x92, {"MissingNo. (Hitmontop)", TYPE_BIRD, TYPE_NORMAL}},
    {0x93, {"Haunter", TYPE_GHOST, TYPE_POISON}},
    {0x94, {"Abra", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x95, {"Alakazam", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x96, {"Pidgeotto", TYPE_NORMAL, TYPE_FLYING}},
    {0x97, {"Pidgeot", TYPE_NORMAL, TYPE_FLYING}},
    {0x98, {"Starmie", TYPE_WATER, TYPE_PSYCHIC}},
    {0x99, {"Bulbasaur", TYPE_GRASS, TYPE_POISON}},
    {0x9A, {"Venusaur", TYPE_GRASS, TYPE_POISON}},
    {0x9B, {"Tentacruel", TYPE_WATER, TYPE_POISON}},
    {0x9C, {"MissingNo. (Smoochum)", TYPE_BIRD, TYPE_NORMAL}},
    {0x9D, {"Goldeen", TYPE_WATER, TYPE_WATER}},
    {0x9E, {"Seaking", TYPE_WATER, TYPE_WATER}},
    {0x9F, {"MissingNo. (Elekid)", TYPE_BIRD, TYPE_NORMAL}},
    {0xA0, {"MissingNo. (Magby)", TYPE_BIRD, TYPE_NORMAL}},
    {0xA1, {"MissingNo. (Miltank)", TYPE_BIRD, TYPE_NORMAL}},
    {0xA2, {"MissingNo. (Blissey)", TYPE_BIRD, TYPE_NORMAL}},
    {0xA3, {"Ponyta", TYPE_FIRE, TYPE_FIRE}},
    {0xA4, {"Rapidash", TYPE_FIRE, TYPE_FIRE}},
    {0xA5, {"Rattata", TYPE_NORMAL, TYPE_NORMAL}},
    {0xA6, {"Raticate", TYPE_NORMAL, TYPE_NORMAL}},
    {0xA7, {"Nidorino", TYPE_POISON, TYPE_POISON}},
    {0xA8, {"Nidorina", TYPE_POISON, TYPE_POISON}},
    {0xA9, {"Geodude", TYPE_ROCK, TYPE_GROUND}},
    {0xAA, {"Porygon", TYPE_NORMAL, TYPE_NORMAL}},
    {0xAB, {"Aerodactyl", TYPE_ROCK, TYPE_FLYING}},
    {0xAC, {"MissingNo. (Raikou)", TYPE_BIRD, TYPE_NORMAL}},
    {0xAD, {"Magnemite", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0xAE, {"MissingNo. (Entei)", TYPE_BIRD, TYPE_NORMAL}},
    {0xAF, {"MissingNo. (Suicune)", TYPE_BIRD, TYPE_NORMAL}},
    {0xB0, {"Charmander", TYPE_FIRE, TYPE_FIRE}},
    {0xB1, {"Squirtle", TYPE_WATER, TYPE_WATER}},
    {0xB2, {"Charmeleon", TYPE_FIRE, TYPE_FIRE}},
    {0xB3, {"Wartortle", TYPE_WATER, TYPE_WATER}},
    {0xB4, {"Charizard", TYPE_FIRE, TYPE_FLYING}},
    {0xB5, {"MissingNo. (Larvitar)", TYPE_BIRD, TYPE_NORMAL}},
    {0xB6, {"MissingNo. Kabutops Fossil (Pupitar)", TYPE_BIRD, TYPE_NORMAL}},
    {0xB7, {"MissingNo. Aerodactyl Fossil (Tyranitar)", TYPE_BIRD, TYPE_NORMAL}},
    {0xB8, {"MissingNo. Ghost (Lugia)", TYPE_BIRD, TYPE_NORMAL}},
    {0xB9, {"Oddish", TYPE_GRASS, TYPE_POISON}},
    {0xBA, {"Gloom", TYPE_GRASS, TYPE_POISON}},
    {0xBB, {"Vileplume", TYPE_GRASS, TYPE_POISON}},
    {0xBC, {"Bellsprout", TYPE_GRASS, TYPE_POISON}},
    {0xBD, {"Weepinbell", TYPE_GRASS, TYPE_POISON}},
    {0xBE, {"Victreebel", TYPE_GRASS, TYPE_POISON}},
};

// ============================================================================
// Generation 2 Pokémon Table
// ============================================================================

static const std::unordered_map<uint8_t, PokemonInfo> GEN2_POKEMON = {
    {0x00, {"?????", TYPE_STEEL, TYPE_UNKNOWN}},
    {0x01, {"Bulbasaur", TYPE_GRASS, TYPE_POISON}},
    {0x02, {"Ivysaur", TYPE_GRASS, TYPE_POISON}},
    {0x03, {"Venusaur", TYPE_GRASS, TYPE_POISON}},
    {0x04, {"Charmander", TYPE_FIRE, TYPE_FIRE}},
    {0x05, {"Charmeleon", TYPE_FIRE, TYPE_FIRE}},
    {0x06, {"Charizard", TYPE_FIRE, TYPE_FLYING}},
    {0x07, {"Squirtle", TYPE_WATER, TYPE_WATER}},
    {0x08, {"Wartortle", TYPE_WATER, TYPE_WATER}},
    {0x09, {"Blastoise", TYPE_WATER, TYPE_WATER}},
    {0x0A, {"Caterpie", TYPE_BUG, TYPE_BUG}},
    {0x0B, {"Metapod", TYPE_BUG, TYPE_BUG}},
    {0x0C, {"Butterfree", TYPE_BUG, TYPE_FLYING}},
    {0x0D, {"Weedle", TYPE_BUG, TYPE_POISON}},
    {0x0E, {"Kakuna", TYPE_BUG, TYPE_POISON}},
    {0x0F, {"Beedrill", TYPE_BUG, TYPE_POISON}},
    {0x10, {"Pidgey", TYPE_NORMAL, TYPE_FLYING}},
    {0x11, {"Pidgeotto", TYPE_NORMAL, TYPE_FLYING}},
    {0x12, {"Pidgeot", TYPE_NORMAL, TYPE_FLYING}},
    {0x13, {"Rattata", TYPE_NORMAL, TYPE_NORMAL}},
    {0x14, {"Raticate", TYPE_NORMAL, TYPE_NORMAL}},
    {0x15, {"Spearow", TYPE_NORMAL, TYPE_FLYING}},
    {0x16, {"Fearow", TYPE_NORMAL, TYPE_FLYING}},
    {0x17, {"Ekans", TYPE_POISON, TYPE_POISON}},
    {0x18, {"Arbok", TYPE_POISON, TYPE_POISON}},
    {0x19, {"Pikachu", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0x1A, {"Raichu", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0x1B, {"Sandshrew", TYPE_GROUND, TYPE_GROUND}},
    {0x1C, {"Sandslash", TYPE_GROUND, TYPE_GROUND}},
    {0x1D, {"Nidoran♀", TYPE_POISON, TYPE_POISON}},
    {0x1E, {"Nidorina", TYPE_POISON, TYPE_POISON}},
    {0x1F, {"Nidoqueen", TYPE_POISON, TYPE_GROUND}},
    {0x20, {"Nidoran♂", TYPE_POISON, TYPE_POISON}},
    {0x21, {"Nidorino", TYPE_POISON, TYPE_POISON}},
    {0x22, {"Nidoking", TYPE_POISON, TYPE_GROUND}},
    {0x23, {"Clefairy", TYPE_NORMAL, TYPE_NORMAL}},
    {0x24, {"Clefable", TYPE_NORMAL, TYPE_NORMAL}},
    {0x25, {"Vulpix", TYPE_FIRE, TYPE_FIRE}},
    {0x26, {"Ninetales", TYPE_FIRE, TYPE_FIRE}},
    {0x27, {"Jigglypuff", TYPE_NORMAL, TYPE_NORMAL}},
    {0x28, {"Wigglytuff", TYPE_NORMAL, TYPE_NORMAL}},
    {0x29, {"Zubat", TYPE_POISON, TYPE_FLYING}},
    {0x2A, {"Golbat", TYPE_POISON, TYPE_FLYING}},
    {0x2B, {"Oddish", TYPE_GRASS, TYPE_POISON}},
    {0x2C, {"Gloom", TYPE_GRASS, TYPE_POISON}},
    {0x2D, {"Vileplume", TYPE_GRASS, TYPE_POISON}},
    {0x2E, {"Paras", TYPE_BUG, TYPE_GRASS}},
    {0x2F, {"Parasect", TYPE_BUG, TYPE_GRASS}},
    {0x30, {"Venonat", TYPE_BUG, TYPE_POISON}},
    {0x31, {"Venomoth", TYPE_BUG, TYPE_POISON}},
    {0x32, {"Diglett", TYPE_GROUND, TYPE_GROUND}},
    {0x33, {"Dugtrio", TYPE_GROUND, TYPE_GROUND}},
    {0x34, {"Meowth", TYPE_NORMAL, TYPE_NORMAL}},
    {0x35, {"Persian", TYPE_NORMAL, TYPE_NORMAL}},
    {0x36, {"Psyduck", TYPE_WATER, TYPE_WATER}},
    {0x37, {"Golduck", TYPE_WATER, TYPE_WATER}},
    {0x38, {"Mankey", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x39, {"Primeape", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x3A, {"Growlithe", TYPE_FIRE, TYPE_FIRE}},
    {0x3B, {"Arcanine", TYPE_FIRE, TYPE_FIRE}},
    {0x3C, {"Poliwag", TYPE_WATER, TYPE_WATER}},
    {0x3D, {"Poliwhirl", TYPE_WATER, TYPE_WATER}},
    {0x3E, {"Poliwrath", TYPE_WATER, TYPE_FIGHTING}},
    {0x3F, {"Abra", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x40, {"Kadabra", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x41, {"Alakazam", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x42, {"Machop", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x43, {"Machoke", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x44, {"Machamp", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x45, {"Bellsprout", TYPE_GRASS, TYPE_POISON}},
    {0x46, {"Weepinbell", TYPE_GRASS, TYPE_POISON}},
    {0x47, {"Victreebel", TYPE_GRASS, TYPE_POISON}},
    {0x48, {"Tentacool", TYPE_WATER, TYPE_POISON}},
    {0x49, {"Tentacruel", TYPE_WATER, TYPE_POISON}},
    {0x4A, {"Geodude", TYPE_ROCK, TYPE_GROUND}},
    {0x4B, {"Graveler", TYPE_ROCK, TYPE_GROUND}},
    {0x4C, {"Golem", TYPE_ROCK, TYPE_GROUND}},
    {0x4D, {"Ponyta", TYPE_FIRE, TYPE_FIRE}},
    {0x4E, {"Rapidash", TYPE_FIRE, TYPE_FIRE}},
    {0x4F, {"Slowpoke", TYPE_WATER, TYPE_PSYCHIC}},
    {0x50, {"Slowbro", TYPE_WATER, TYPE_PSYCHIC}},
    {0x51, {"Magnemite", TYPE_ELECTRIC, TYPE_STEEL}},
    {0x52, {"Magneton", TYPE_ELECTRIC, TYPE_STEEL}},
    {0x53, {"Farfetch'd", TYPE_NORMAL, TYPE_FLYING}},
    {0x54, {"Doduo", TYPE_NORMAL, TYPE_FLYING}},
    {0x55, {"Dodrio", TYPE_NORMAL, TYPE_FLYING}},
    {0x56, {"Seel", TYPE_WATER, TYPE_WATER}},
    {0x57, {"Dewgong", TYPE_WATER, TYPE_ICE}},
    {0x58, {"Grimer", TYPE_POISON, TYPE_POISON}},
    {0x59, {"Muk", TYPE_POISON, TYPE_POISON}},
    {0x5A, {"Shellder", TYPE_WATER, TYPE_WATER}},
    {0x5B, {"Cloyster", TYPE_WATER, TYPE_ICE}},
    {0x5C, {"Gastly", TYPE_GHOST, TYPE_POISON}},
    {0x5D, {"Haunter", TYPE_GHOST, TYPE_POISON}},
    {0x5E, {"Gengar", TYPE_GHOST, TYPE_POISON}},
    {0x5F, {"Onix", TYPE_ROCK, TYPE_GROUND}},
    {0x60, {"Drowzee", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x61, {"Hypno", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x62, {"Krabby", TYPE_WATER, TYPE_WATER}},
    {0x63, {"Kingler", TYPE_WATER, TYPE_WATER}},
    {0x64, {"Voltorb", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0x65, {"Electrode", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0x66, {"Exeggcute", TYPE_GRASS, TYPE_PSYCHIC}},
    {0x67, {"Exeggutor", TYPE_GRASS, TYPE_PSYCHIC}},
    {0x68, {"Cubone", TYPE_GROUND, TYPE_GROUND}},
    {0x69, {"Marowak", TYPE_GROUND, TYPE_GROUND}},
    {0x6A, {"Hitmonlee", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x6B, {"Hitmonchan", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0x6C, {"Lickitung", TYPE_NORMAL, TYPE_NORMAL}},
    {0x6D, {"Koffing", TYPE_POISON, TYPE_POISON}},
    {0x6E, {"Weezing", TYPE_POISON, TYPE_POISON}},
    {0x6F, {"Rhyhorn", TYPE_GROUND, TYPE_ROCK}},
    {0x70, {"Rhydon", TYPE_GROUND, TYPE_ROCK}},
    {0x71, {"Chansey", TYPE_NORMAL, TYPE_NORMAL}},
    {0x72, {"Tangela", TYPE_GRASS, TYPE_GRASS}},
    {0x73, {"Kangaskhan", TYPE_NORMAL, TYPE_NORMAL}},
    {0x74, {"Horsea", TYPE_WATER, TYPE_WATER}},
    {0x75, {"Seadra", TYPE_WATER, TYPE_WATER}},
    {0x76, {"Goldeen", TYPE_WATER, TYPE_WATER}},
    {0x77, {"Seaking", TYPE_WATER, TYPE_WATER}},
    {0x78, {"Staryu", TYPE_WATER, TYPE_WATER}},
    {0x79, {"Starmie", TYPE_WATER, TYPE_PSYCHIC}},
    {0x7A, {"Mr. Mime", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x7B, {"Scyther", TYPE_BUG, TYPE_FLYING}},
    {0x7C, {"Jynx", TYPE_ICE, TYPE_PSYCHIC}},
    {0x7D, {"Electabuzz", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0x7E, {"Magmar", TYPE_FIRE, TYPE_FIRE}},
    {0x7F, {"Pinsir", TYPE_BUG, TYPE_BUG}},
    {0x80, {"Tauros", TYPE_NORMAL, TYPE_NORMAL}},
    {0x81, {"Magikarp", TYPE_WATER, TYPE_WATER}},
    {0x82, {"Gyarados", TYPE_WATER, TYPE_FLYING}},
    {0x83, {"Lapras", TYPE_WATER, TYPE_ICE}},
    {0x84, {"Ditto", TYPE_NORMAL, TYPE_NORMAL}},
    {0x85, {"Eevee", TYPE_NORMAL, TYPE_NORMAL}},
    {0x86, {"Vaporeon", TYPE_WATER, TYPE_WATER}},
    {0x87, {"Jolteon", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0x88, {"Flareon", TYPE_FIRE, TYPE_FIRE}},
    {0x89, {"Porygon", TYPE_NORMAL, TYPE_NORMAL}},
    {0x8A, {"Omanyte", TYPE_ROCK, TYPE_WATER}},
    {0x8B, {"Omastar", TYPE_ROCK, TYPE_WATER}},
    {0x8C, {"Kabuto", TYPE_ROCK, TYPE_WATER}},
    {0x8D, {"Kabutops", TYPE_ROCK, TYPE_WATER}},
    {0x8E, {"Aerodactyl", TYPE_ROCK, TYPE_FLYING}},
    {0x8F, {"Snorlax", TYPE_NORMAL, TYPE_NORMAL}},
    {0x90, {"Articuno", TYPE_ICE, TYPE_FLYING}},
    {0x91, {"Zapdos", TYPE_ELECTRIC, TYPE_FLYING}},
    {0x92, {"Moltres", TYPE_FIRE, TYPE_FLYING}},
    {0x93, {"Dratini", TYPE_DRAGON, TYPE_DRAGON}},
    {0x94, {"Dragonair", TYPE_DRAGON, TYPE_DRAGON}},
    {0x95, {"Dragonite", TYPE_DRAGON, TYPE_FLYING}},
    {0x96, {"Mewtwo", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x97, {"Mew", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0x98, {"Chikorita", TYPE_GRASS, TYPE_GRASS}},
    {0x99, {"Bayleef", TYPE_GRASS, TYPE_GRASS}},
    {0x9A, {"Meganium", TYPE_GRASS, TYPE_GRASS}},
    {0x9B, {"Cyndaquil", TYPE_FIRE, TYPE_FIRE}},
    {0x9C, {"Quilava", TYPE_FIRE, TYPE_FIRE}},
    {0x9D, {"Typhlosion", TYPE_FIRE, TYPE_FIRE}},
    {0x9E, {"Totodile", TYPE_WATER, TYPE_WATER}},
    {0x9F, {"Croconaw", TYPE_WATER, TYPE_WATER}},
    {0xA0, {"Feraligatr", TYPE_WATER, TYPE_WATER}},
    {0xA1, {"Sentret", TYPE_NORMAL, TYPE_NORMAL}},
    {0xA2, {"Furret", TYPE_NORMAL, TYPE_NORMAL}},
    {0xA3, {"Hoothoot", TYPE_NORMAL, TYPE_FLYING}},
    {0xA4, {"Noctowl", TYPE_NORMAL, TYPE_FLYING}},
    {0xA5, {"Ledyba", TYPE_BUG, TYPE_FLYING}},
    {0xA6, {"Ledian", TYPE_BUG, TYPE_FLYING}},
    {0xA7, {"Spinarak", TYPE_BUG, TYPE_POISON}},
    {0xA8, {"Ariados", TYPE_BUG, TYPE_POISON}},
    {0xA9, {"Crobat", TYPE_POISON, TYPE_FLYING}},
    {0xAA, {"Chinchou", TYPE_WATER, TYPE_ELECTRIC}},
    {0xAB, {"Lanturn", TYPE_WATER, TYPE_ELECTRIC}},
    {0xAC, {"Pichu", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0xAD, {"Cleffa", TYPE_NORMAL, TYPE_NORMAL}},
    {0xAE, {"Igglybuff", TYPE_NORMAL, TYPE_NORMAL}},
    {0xAF, {"Togepi", TYPE_NORMAL, TYPE_NORMAL}},
    {0xB0, {"Togetic", TYPE_NORMAL, TYPE_FLYING}},
    {0xB1, {"Natu", TYPE_PSYCHIC, TYPE_FLYING}},
    {0xB2, {"Xatu", TYPE_PSYCHIC, TYPE_FLYING}},
    {0xB3, {"Mareep", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0xB4, {"Flaaffy", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0xB5, {"Ampharos", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0xB6, {"Bellossom", TYPE_GRASS, TYPE_GRASS}},
    {0xB7, {"Marill", TYPE_WATER, TYPE_WATER}},
    {0xB8, {"Azumarill", TYPE_WATER, TYPE_WATER}},
    {0xB9, {"Sudowoodo", TYPE_ROCK, TYPE_ROCK}},
    {0xBA, {"Politoed", TYPE_WATER, TYPE_WATER}},
    {0xBB, {"Hoppip", TYPE_GRASS, TYPE_FLYING}},
    {0xBC, {"Skiploom", TYPE_GRASS, TYPE_FLYING}},
    {0xBD, {"Jumpluff", TYPE_GRASS, TYPE_FLYING}},
    {0xBE, {"Aipom", TYPE_NORMAL, TYPE_NORMAL}},
    {0xBF, {"Sunkern", TYPE_GRASS, TYPE_GRASS}},
    {0xC0, {"Sunflora", TYPE_GRASS, TYPE_GRASS}},
    {0xC1, {"Yanma", TYPE_BUG, TYPE_FLYING}},
    {0xC2, {"Wooper", TYPE_WATER, TYPE_GROUND}},
    {0xC3, {"Quagsire", TYPE_WATER, TYPE_GROUND}},
    {0xC4, {"Espeon", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0xC5, {"Umbreon", TYPE_DARK, TYPE_DARK}},
    {0xC6, {"Murkrow", TYPE_DARK, TYPE_FLYING}},
    {0xC7, {"Slowking", TYPE_WATER, TYPE_PSYCHIC}},
    {0xC8, {"Misdreavus", TYPE_GHOST, TYPE_GHOST}},
    {0xC9, {"Unown", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0xCA, {"Wobbuffet", TYPE_PSYCHIC, TYPE_PSYCHIC}},
    {0xCB, {"Girafarig", TYPE_NORMAL, TYPE_PSYCHIC}},
    {0xCC, {"Pineco", TYPE_BUG, TYPE_BUG}},
    {0xCD, {"Forretress", TYPE_BUG, TYPE_STEEL}},
    {0xCE, {"Dunsparce", TYPE_NORMAL, TYPE_NORMAL}},
    {0xCF, {"Gligar", TYPE_GROUND, TYPE_FLYING}},
    {0xD0, {"Steelix", TYPE_STEEL, TYPE_GROUND}},
    {0xD1, {"Snubbull", TYPE_NORMAL, TYPE_NORMAL}},
    {0xD2, {"Granbull", TYPE_NORMAL, TYPE_NORMAL}},
    {0xD3, {"Qwilfish", TYPE_WATER, TYPE_POISON}},
    {0xD4, {"Scizor", TYPE_BUG, TYPE_STEEL}},
    {0xD5, {"Shuckle", TYPE_BUG, TYPE_ROCK}},
    {0xD6, {"Heracross", TYPE_BUG, TYPE_FIGHTING}},
    {0xD7, {"Sneasel", TYPE_DARK, TYPE_ICE}},
    {0xD8, {"Teddiursa", TYPE_NORMAL, TYPE_NORMAL}},
    {0xD9, {"Ursaring", TYPE_NORMAL, TYPE_NORMAL}},
    {0xDA, {"Slugma", TYPE_FIRE, TYPE_FIRE}},
    {0xDB, {"Magcargo", TYPE_FIRE, TYPE_ROCK}},
    {0xDC, {"Swinub", TYPE_ICE, TYPE_GROUND}},
    {0xDD, {"Piloswine", TYPE_ICE, TYPE_GROUND}},
    {0xDE, {"Corsola", TYPE_WATER, TYPE_ROCK}},
    {0xDF, {"Remoraid", TYPE_WATER, TYPE_WATER}},
    {0xE0, {"Octillery", TYPE_WATER, TYPE_WATER}},
    {0xE1, {"Delibird", TYPE_ICE, TYPE_FLYING}},
    {0xE2, {"Mantine", TYPE_WATER, TYPE_FLYING}},
    {0xE3, {"Skarmory", TYPE_STEEL, TYPE_FLYING}},
    {0xE4, {"Houndour", TYPE_DARK, TYPE_FIRE}},
    {0xE5, {"Houndoom", TYPE_DARK, TYPE_FIRE}},
    {0xE6, {"Kingdra", TYPE_WATER, TYPE_DRAGON}},
    {0xE7, {"Phanpy", TYPE_GROUND, TYPE_GROUND}},
    {0xE8, {"Donphan", TYPE_GROUND, TYPE_GROUND}},
    {0xE9, {"Porygon2", TYPE_NORMAL, TYPE_NORMAL}},
    {0xEA, {"Stantler", TYPE_NORMAL, TYPE_NORMAL}},
    {0xEB, {"Smeargle", TYPE_NORMAL, TYPE_NORMAL}},
    {0xEC, {"Tyrogue", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0xED, {"Hitmontop", TYPE_FIGHTING, TYPE_FIGHTING}},
    {0xEE, {"Smoochum", TYPE_ICE, TYPE_PSYCHIC}},
    {0xEF, {"Elekid", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0xF0, {"Magby", TYPE_FIRE, TYPE_FIRE}},
    {0xF1, {"Miltank", TYPE_NORMAL, TYPE_NORMAL}},
    {0xF2, {"Blissey", TYPE_NORMAL, TYPE_NORMAL}},
    {0xF3, {"Raikou", TYPE_ELECTRIC, TYPE_ELECTRIC}},
    {0xF4, {"Entei", TYPE_FIRE, TYPE_FIRE}},
    {0xF5, {"Suicune", TYPE_WATER, TYPE_WATER}},
    {0xF6, {"Larvitar", TYPE_ROCK, TYPE_GROUND}},
    {0xF7, {"Pupitar", TYPE_ROCK, TYPE_GROUND}},
    {0xF8, {"Tyranitar", TYPE_ROCK, TYPE_DARK}},
    {0xF9, {"Lugia", TYPE_PSYCHIC, TYPE_FLYING}},
    {0xFA, {"Ho-Oh", TYPE_FIRE, TYPE_FLYING}},
    {0xFB, {"Celebi", TYPE_PSYCHIC, TYPE_GRASS}},
    {0xFC, {"?????", TYPE_PSYCHIC, TYPE_UNKNOWN}},
    {0xFD, {"Glitch Egg", TYPE_CURSE, TYPE_CURSE}},
    {0xFE, {"?????", TYPE_NORMAL, TYPE_NORMAL}},
    {0xFF, {"?????", TYPE_NORMAL, TYPE_NORMAL}},
};

// ============================================================================
// Utility functions
// ============================================================================

// Get Pokémon info by index and generation (returns nullptr if not found)
inline const PokemonInfo* getPokemonInfo(uint8_t index, int generation) {
    const std::unordered_map<uint8_t, PokemonInfo>* pokemonMap = nullptr;
    
    switch (generation) {
        case 1:
            pokemonMap = &GEN1_POKEMON;
            break;
        case 2:
            pokemonMap = &GEN2_POKEMON;
            break;
        default:
            return nullptr;
    }
    
    auto it = pokemonMap->find(index);
    if (it != pokemonMap->end()) {
        return &it->second;
    }
    return nullptr;
}

// Get Pokémon name by index and generation (returns nullptr if not found)
inline const char* getPokemonName(uint8_t index, int generation) {
    const PokemonInfo* info = getPokemonInfo(index, generation);
    return info ? info->name : nullptr;
}

// Get primary type for Pokémon by index and generation (returns TYPE_UNKNOWN if not found)
inline uint8_t getPokemonType1(uint8_t index, int generation) {
    const PokemonInfo* info = getPokemonInfo(index, generation);
    return info ? info->type1 : TYPE_UNKNOWN;
}

// Get secondary type for Pokémon by index and generation (returns TYPE_UNKNOWN if not found)
inline uint8_t getPokemonType2(uint8_t index, int generation) {
    const PokemonInfo* info = getPokemonInfo(index, generation);
    return info ? info->type2 : TYPE_UNKNOWN;
}

// Get both types as a combined value (type1 in upper byte, type2 in lower byte, or 0xFFFF if not found)
inline uint16_t getPokemonTypes(uint8_t index, int generation) {
    uint8_t type1 = getPokemonType1(index, generation);
    uint8_t type2 = getPokemonType2(index, generation);
    
    if (type1 == TYPE_UNKNOWN && type2 == TYPE_UNKNOWN) {
        return 0xFFFF;
    }
    return (static_cast<uint16_t>(type1) << 8) | type2;
}

// ============================================================================
// Legacy Gen 1-specific functions (for backward compatibility)
// ============================================================================

inline const PokemonInfo* getGen1PokemonInfo(uint8_t index) {
    return getPokemonInfo(index, 1);
}

inline const char* getGen1PokemonName(uint8_t index) {
    return getPokemonName(index, 1);
}

inline uint8_t getGen1PokemonType1(uint8_t index) {
    return getPokemonType1(index, 1);
}

inline uint8_t getGen1PokemonType2(uint8_t index) {
    return getPokemonType2(index, 1);
}

// ============================================================================
// Legacy Gen 2-specific functions (for backward compatibility)
// ============================================================================

inline const PokemonInfo* getGen2PokemonInfo(uint8_t index) {
    return getPokemonInfo(index, 2);
}

inline const char* getGen2PokemonName(uint8_t index) {
    return getPokemonName(index, 2);
}

inline uint8_t getGen2PokemonType1(uint8_t index) {
    return getPokemonType1(index, 2);
}

inline uint8_t getGen2PokemonType2(uint8_t index) {
    return getPokemonType2(index, 2);
}

} // namespace PokemonIndex

#endif // POKEMON_INDEX_ENG_H